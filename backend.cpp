#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <modbus/modbus.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

#include "backend.h"

typedef void * (*THREADFUNCPTR)(void *);

BackEnd::BackEnd(QObject *parent) : QObject(parent)
{
    int err = pthread_create(&hwdiag_thread, NULL, (THREADFUNCPTR) &BackEnd::runDiagLoop, this);
    if (err) {
        std::cout << "Thread creation failed : " << strerror(err);
    }

    err = pthread_create(&hwdiag_rs485_thread, NULL, (THREADFUNCPTR) &BackEnd::runRs485Loop, this);
    if (err) {
        std::cout << "RS485 Thread creation failed : " << strerror(err);
    }

    modbus_status_ok = 0;
}

void BackEnd::_setEthStatus(int status_ok)
{
    if (!status_ok && eth_status_ok) {
        eth_status_ok = 0;
        emit ethStatusChanged();
    } else if (status_ok && !eth_status_ok) {
        eth_status_ok = 1;
        emit ethStatusChanged();
    }
}

int BackEnd::_runEthTest(void)
{
    int fd, n;
    char buf[64];

    fd = open("/sys/class/net/eth0/operstate", O_RDONLY);
    if (fd < 0)
        _setEthStatus(0);

    n = read(fd, buf, sizeof(buf)-1);
    if (n < 0) {
        _setEthStatus(0);
        close(fd);
        return -1;
    }

    if (!strncmp(buf, "up", 2))
        _setEthStatus(1);
    else
        _setEthStatus(0);

    close(fd);
    return 0;
}

int BackEnd::_runSDCardTest(void)
{
    char buf[512];
    int fd, n;

    fd = open("/dev/disk/by-path/platform-2190000.usdhc-part1", O_RDONLY);
    if (fd < 0)
        goto sdcard_error;

    /* read 512 bytes from device */
    n = read(fd, buf, sizeof(buf));
    close(fd);

    if (n != sizeof(buf))
        goto sdcard_error;

    /* success */
    if (!sdcard_status_ok) {
        sdcard_status_ok = 1;
        emit SDCardStatusChanged();
    }

    return 0;

sdcard_error:
    if (sdcard_status_ok) {
        sdcard_status_ok = 0;
        emit SDCardStatusChanged();
    }

    return -1;
}

/* FIXME USB and USB OTG tests are basically the same. Do some code refactor. */
int BackEnd::_runUsbOtgTest(void)
{
    char buf[512];
    int fd, n;

    fd = open("/dev/disk/by-path/platform-ci_hdrc.0-usb-0:1:1.0-scsi-0:0:0:0-part1", O_RDONLY);
    if (fd < 0)
        goto usb_otg_error;


    /* read 512 bytes from device */
    n = read(fd, buf, sizeof(buf));
    close(fd);

    if (n != sizeof(buf))
        goto usb_otg_error;

    /* success */
    if (!usb_otg_status_ok) {
        usb_otg_status_ok = 1;
        emit usbOtgStatusChanged();
    }

    return 0;

usb_otg_error:
    if (usb_otg_status_ok) {
        usb_otg_status_ok = 0;
        emit usbOtgStatusChanged();
    }

    return -1;
}

int BackEnd::_runUsbTest(void)
{
    char buf[512];
    int fd, n;

    fd = open("/dev/disk/by-path/platform-ci_hdrc.1-usb-0:1:1.0-scsi-0:0:0:0-part1", O_RDONLY);
    if (fd < 0)
        goto usb_error;


    /* read 512 bytes from device */
    n = read(fd, buf, sizeof(buf));
    close(fd);

    if (n != sizeof(buf))
        goto usb_error;

    /* success */
    if (!usb_status_ok) {
        usb_status_ok = 1;
        emit usbStatusChanged();
    }

    return 0;

usb_error:
    if (usb_status_ok) {
        usb_status_ok = 0;
        emit usbStatusChanged();
    }

    return -1;
}

int BackEnd::_runModbusTest(void)
{
    modbus_t *ctx;
    int ret, i;

    /* Modbus UART uses ttymxc4 */
    ctx = modbus_new_rtu("/dev/ttymxc4", 19200, 'N', 8, 1);
    if (ctx == NULL) {
        qWarning("Unable to create the libmodbus context\n");
        if (modbus_status_ok == 1) {
            emit modbusStatusChanged();
            modbus_status_ok = 0;
        }
        return -1;
    }

    modbus_set_slave(ctx, 1);
    modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
    modbus_rtu_set_rts(ctx, MODBUS_RTU_RTS_UP);
    modbus_set_response_timeout(ctx, 0, 200000);

    if (modbus_connect(ctx) == -1) {
        qWarning("Unable to connect to modbus slave\n");
        if (modbus_status_ok == 1) {
            modbus_status_ok = 0;
            emit modbusStatusChanged();
        }
        goto error;
    }


    /* Try 10 times */
    for (i = 0; i < 10; i++) {
        ret = modbus_read_registers(ctx, 0, 16, regs);
        if (ret == 32)
            break;
    }

    if (ret == -1)
        qWarning("Read Mobbus registers returned %d : %s\n", ret, modbus_strerror(errno));

    modbus_flush(ctx);
    modbus_close(ctx);

    if (ret == -1 && modbus_status_ok == 1) {
        modbus_status_ok = 0;
        emit modbusStatusChanged();
    } else if (ret > 0 && modbus_status_ok == 0) {
        modbus_status_ok = 1;
        emit modbusStatusChanged();
    }

error:
    modbus_free(ctx);
    return 0;
}

void BackEnd::_eeprom_set_status(int eeprom_idx, int status)
{
    switch (eeprom_idx) {
    case 0:
        if (status && !eeprom1_status_ok) {
            eeprom1_status_ok = 1;
            eeprom1StatusChanged();
        } else if (!status && eeprom1_status_ok) {
            eeprom1_status_ok = 0;
            eeprom1StatusChanged();
        }
        break;
    case 1:
        if (status && !eeprom2_status_ok) {
            eeprom2_status_ok = 1;
            eeprom2StatusChanged();
        } else if (!status && eeprom2_status_ok) {
            eeprom2_status_ok = 0;
            eeprom2StatusChanged();
        }
    default:
        break;
    }

}

int BackEnd::_runEEPROMTest(void)
{
    int buf[64];
    int i, n;

    for (i = 0; i < 2; i++) {
        int fd = open(i == 0 ? "/sys/bus/i2c/devices/0-0050/eeprom" :
                               "/sys/bus/i2c/devices/0-0056/eeprom", O_RDWR);

        if (fd < 0) {
            _eeprom_set_status(i, 0);
            continue;
        }

        n = read(fd, buf, sizeof(buf));
        if (n == sizeof(buf))
            _eeprom_set_status(i, 1);
        else
            _eeprom_set_status(i, 0);

        close(fd);

    }
    return 0;
}

int BackEnd::_runRTCTest(void)
{
    int ret = system("hwclock -s");

    if (ret != 0 && rtc_status_ok == 1) {
        rtc_status_ok = 0;
        emit rtcStatusChanged();
    } else if (ret == 0 && rtc_status_ok == 0) {
        rtc_status_ok = 1;
        emit rtcStatusChanged();
    }

    return 0;
}

/* Thread for Modbus since it runs much slower than the rest */
void * BackEnd::runRs485Loop(void *)
{
    while(1) {
        _runModbusTest();
        usleep(1000000);
    }
}

void * BackEnd::runDiagLoop(void *)
{
    while(1) {
        _runUsbTest();
        _runUsbOtgTest();
        _runSDCardTest();
        _runEthTest();
        _runRTCTest();
        _runEEPROMTest();
        usleep(200000);
    }
}

QString BackEnd::modbusStatusGet()
{
    if (modbus_status_ok) {
        return "OK";
    } else {
        return "FAILURE";
    }
}

QString BackEnd::usbStatusGet()
{
    if (usb_status_ok) {
        return "OK";
    } else {
        return "FAILURE";
    }
}

QString BackEnd::usbOtgStatusGet()
{
    if (usb_otg_status_ok) {
        return "OK";
    } else {
        return "FAILURE";
    }
}

QString BackEnd::SDCardStatusGet()
{
    if (sdcard_status_ok) {
        return "OK";
    } else {
        return "FAILURE";
    }
}

QString BackEnd::ethStatusGet()
{
    if (eth_status_ok) {
        return "OK";
    } else {
        return "FAILURE";
    }
}

QString BackEnd::rtcStatusGet()
{
    if (rtc_status_ok) {
        return "OK";
    } else {
        return "FAILURE";
    }
}

QString BackEnd::eeprom1StatusGet()
{
    if (eeprom1_status_ok) {
        return "OK";
    } else {
        return "FAILURE";
    }
}

QString BackEnd::eeprom2StatusGet()
{
    if (eeprom2_status_ok) {
        return "OK";
    } else {
        return "FAILURE";
    }
}
