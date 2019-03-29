#ifndef BACKEND_H
#define BACKEND_H

#include <stdint.h>
#include <QObject>
#include <QString>
#include <pthread.h>

class BackEnd : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString modbusStatus READ modbusStatusGet NOTIFY modbusStatusChanged)
    Q_PROPERTY(QString usbStatus READ usbStatusGet NOTIFY usbStatusChanged)
    Q_PROPERTY(QString usbOtgStatus READ usbOtgStatusGet NOTIFY usbOtgStatusChanged)
    Q_PROPERTY(QString SDCardStatus READ SDCardStatusGet NOTIFY SDCardStatusChanged)
    Q_PROPERTY(QString ethStatus READ ethStatusGet NOTIFY ethStatusChanged)
    Q_PROPERTY(QString rtcStatus READ rtcStatusGet NOTIFY rtcStatusChanged)
    Q_PROPERTY(QString eeprom1Status READ eeprom1StatusGet NOTIFY eeprom1StatusChanged)
    Q_PROPERTY(QString eeprom2Status READ eeprom2StatusGet NOTIFY eeprom2StatusChanged)

public:
    explicit BackEnd(QObject *parent = nullptr);

    QString modbusStatusGet();
    QString usbStatusGet();
    QString usbOtgStatusGet();
    QString SDCardStatusGet();
    QString ethStatusGet();
    QString rtcStatusGet();
    QString eeprom1StatusGet();
    QString eeprom2StatusGet();

    void * runDiagLoop(void *);
    void * runRs485Loop(void *);
    uint16_t regs[16];

signals:
    void modbusStatusChanged();
    void usbStatusChanged();
    void usbOtgStatusChanged();
    void SDCardStatusChanged();
    void ethStatusChanged();
    void rtcStatusChanged();
    void eeprom1StatusChanged();
    void eeprom2StatusChanged();

private:

    pthread_t hwdiag_thread;
    pthread_t hwdiag_rs485_thread;

    int modbus_status_ok;
    int usb_status_ok;
    int usb_otg_status_ok;
    int sdcard_status_ok;
    int eth_status_ok;
    int rtc_status_ok;
    int eeprom1_status_ok;
    int eeprom2_status_ok;

    int _runModbusTest();
    int _runUsbTest();
    int _runUsbOtgTest();
    int _runSDCardTest();
    int _runEthTest();
    int _runEEPROMTest();
    int _runRTCTest();

    void _setEthStatus(int);
    void _eeprom_set_status(int, int);

};

#endif // BACKEND_H
