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
    system("/usr/sbin/config_digital_ios.sh");
    system("/usr/sbin/config_analog_inputs.sh");
    system("/usr/sbin/config_analog_outputs.sh");

    int err = pthread_create(&hwdiag_thread, NULL, (THREADFUNCPTR) &BackEnd::runDiagLoop, this);
    if (err) {
        std::cout << "Thread creation failed : " << strerror(err);
    }

    err = pthread_create(&hwdiag_rs485_thread, NULL, (THREADFUNCPTR) &BackEnd::runRs485Loop, this);
    if (err) {
        std::cout << "RS485 Thread creation failed : " << strerror(err);
    }

    modbus_status_ok = 0;

    dio_status[0] = 0;
    dio_status[1] = 0;
    dio_status[2] = 0;
    dio_status[3] = 0;
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

//    if (ret != 0 && rtc_status_ok == 1) {
//        rtc_status_ok = 0;
//        emit rtcStatusChanged();
//    } else if (ret == 0 && rtc_status_ok == 0) {
//        rtc_status_ok = 1;
//        emit rtcStatusChanged();
//    }

    FILE *fp = popen("date  +\"%H:%M:%S %d/%m/%y\"", "r");
    char buf[128];

    if (fp == NULL)
        ret = -1;

    fread(buf, sizeof(buf), 1, fp);
    rtc_date_str = QString(buf);
    if (ret != 0 && rtc_status_ok == 1) {
        rtc_status_ok = 0;
    } else if (ret == 0 && rtc_status_ok == 0) {
        rtc_status_ok = 1;
    }

    emit rtcStatusChanged();

    return 0;
}

/* Thread for Modbus since it runs much slower than the rest */
void * BackEnd::runRs485Loop(void *)
{
    while(1) {
        _runModbusTest();
        //usleep(1000000);
        sleep(1);
    }
}

void BackEnd::_emitDigitalIOStatusChanged(int io_idx)
{
    switch(io_idx) {
    case 0:
        emit dio1StatusChanged();
        break;
    case 1:
        emit dio2StatusChanged();
        break;
    case 2:
        emit dio3StatusChanged();
        break;
    case 3:
        emit dio4StatusChanged();
        break;
    default:
        break;
    }
}

int BackEnd::_runDigitalIOTest(void)
{
    const char *io_paths[] = {
        "/digital_ios/din1/value",
        "/digital_ios/din2/value",
        "/digital_ios/dout3/value",
        "/digital_ios/dout4/value",
    };
    char buf[64];
    int i, n;

    for (i = 0; i < 4; i++) {
        int fd = open(io_paths[i], O_RDWR);

        if (fd < 0) {
            dio_status[i] = 0;
            continue;
        }

        n = read(fd, buf, sizeof(buf));
        close(fd);

        if (!strncmp(buf, "1", 1) && !dio_status[i]) {
            dio_status[i] = 1;
            _emitDigitalIOStatusChanged(i);
        } else if (!strncmp(buf, "0", 1) && dio_status[i]) {
            dio_status[i] = 0;
            _emitDigitalIOStatusChanged(i);
        }
    }

    return 0;
}

void BackEnd::_emitAnalogInStatusChanged(int io_idx)
{
    switch(io_idx) {
    case 0:
        emit ain1StatusChanged();
        break;
    case 1:
        emit ain2StatusChanged();
        break;
    case 2:
        emit ain3StatusChanged();
        break;
    case 3:
        emit ain4StatusChanged();
        break;
    default:
        break;
    }
}


int BackEnd::_runAnalogInTest(void)
{
    const char *ain_paths[] = {
        "/analog_ios/ain1/input",
        "/analog_ios/ain2/input",
        "/analog_ios/ain3/input",
        "/analog_ios/ain4/input",
    };

    char buf[64];
    int i, n;
    int val;

    for (i = 0; i < 4; i++) {
        int fd = open(ain_paths[i], O_RDONLY);

        if (fd < 0) {
            ain_status[i] = 0;
            continue;
        }

        n = read(fd, buf, sizeof(buf));
        close(fd);

        if (n > 0)
            val = atoi(buf);
        else
            val = 0;

        if (val != ain_status[i]) {
            ain_status[i] = val;
            _emitAnalogInStatusChanged(i);
        }
    }

    return 0;
}

void BackEnd::_emitAnalogOutStatusChanged(int io_idx)
{
    switch(io_idx) {
    case 0:
        emit aout1StatusChanged();
        break;
    case 1:
        emit aout2StatusChanged();
        break;
    default:
        break;
    }
}

int BackEnd::_runAnalogOutTest(void)
{
    const char *aout_paths[] = {
        "/analog_outputs/aout1/out_eng_units",
        "/analog_outputs/aout2/out_eng_units",
    };

    char buf[64];
    int i, n;
    int val;

    for (i = 0; i < 2; i++) {
        int fd = open(aout_paths[i], O_RDONLY);

        if (fd < 0) {
            aout_status[i] = 0;
            continue;
        }

        n = read(fd, buf, sizeof(buf));
        close(fd);

        if (n > 0)
            val = atoi(buf);
        else
            val = 0;

        if (val != aout_status[i]) {
            aout_status[i] = val;
            _emitAnalogOutStatusChanged(i);
        }
    }

    return 0;
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
        _runDigitalIOTest();
        _runAnalogInTest();
        _runAnalogOutTest();
        //usleep(1000000);
        sleep(1);
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
        return rtc_date_str.trimmed(); /* Remove newline */
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

/* Digital I/O */
QString BackEnd::dio1StatusGet()
{
    return dio_status[0] ? "1" : "0";

}

QString BackEnd::dio2StatusGet()
{
    return dio_status[1] ? "1" : "0";

}

QString BackEnd::dio3StatusGet()
{
    return dio_status[2] ? "1" : "0";

}

QString BackEnd::dio4StatusGet()
{
    return dio_status[3] ? "1" : "0";
}


/* Analog Outputs */
QString BackEnd::aout1StatusGet()
{
    return QString::number(aout_status[0]);
}

QString BackEnd::aout2StatusGet()
{
     return QString::number(aout_status[1]);
}


/* Analog Inputs */
QString BackEnd::ain1StatusGet()
{
    return QString::number(ain_status[0]);
}

QString BackEnd::ain2StatusGet()
{
    return QString::number(ain_status[1]);

}

QString BackEnd::ain3StatusGet()
{
    return QString::number(ain_status[2]);
}

QString BackEnd::ain4StatusGet()
{
    return QString::number(ain_status[3]);
}
