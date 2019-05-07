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

    Q_PROPERTY(QString dio1Status READ dio1StatusGet NOTIFY dio1StatusChanged)
    Q_PROPERTY(QString dio2Status READ dio2StatusGet NOTIFY dio2StatusChanged)
    Q_PROPERTY(QString dio3Status READ dio3StatusGet NOTIFY dio3StatusChanged)
    Q_PROPERTY(QString dio4Status READ dio4StatusGet NOTIFY dio4StatusChanged)

    Q_PROPERTY(QString aout1Status READ aout1StatusGet NOTIFY aout1StatusChanged)
    Q_PROPERTY(QString aout2Status READ aout2StatusGet NOTIFY aout2StatusChanged)

    Q_PROPERTY(QString ain1Status READ ain1StatusGet NOTIFY ain1StatusChanged)
    Q_PROPERTY(QString ain2Status READ ain2StatusGet NOTIFY ain2StatusChanged)
    Q_PROPERTY(QString ain3Status READ ain3StatusGet NOTIFY ain3StatusChanged)
    Q_PROPERTY(QString ain4Status READ ain4StatusGet NOTIFY ain4StatusChanged)




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

    QString dio1StatusGet();
    QString dio2StatusGet();
    QString dio3StatusGet();
    QString dio4StatusGet();

    QString aout1StatusGet();
    QString aout2StatusGet();

    QString ain1StatusGet();
    QString ain2StatusGet();
    QString ain3StatusGet();
    QString ain4StatusGet();

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

    void dio1StatusChanged();
    void dio2StatusChanged();
    void dio3StatusChanged();
    void dio4StatusChanged();

    void aout1StatusChanged();
    void aout2StatusChanged();

    void ain1StatusChanged();
    void ain2StatusChanged();
    void ain3StatusChanged();
    void ain4StatusChanged();

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

    int dio_status[4];
    int aout_status[2];
    int ain_status[4];

    QString rtc_date_str;

    int _runModbusTest();
    int _runUsbTest();
    int _runUsbOtgTest();
    int _runSDCardTest();
    int _runEthTest();
    int _runEEPROMTest();
    int _runRTCTest();
    int _runDigitalIOTest();
    int _runAnalogInTest();
    int _runAnalogOutTest();

    void _setEthStatus(int);
    void _eeprom_set_status(int, int);

    void _emitDigitalIOStatusChanged(int);
    void _emitAnalogInStatusChanged(int);
    void _emitAnalogOutStatusChanged(int);

};

#endif // BACKEND_H
