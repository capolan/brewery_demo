import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import io.qt.backend 1.0

Window {
    visible: true
    width: 1024
    height: 600
    title: qsTr("hwdiag")

    /* C++ Backend component */
    BackEnd {
        id: backend
    }

    Grid {
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }

        verticalItemAlignment: Grid.AlignVCenter
        horizontalItemAlignment: Grid.AlignHCenter

        spacing: 20
        columns: 3
        rows: 3

        HwStatus {
            device: "USB Host"
            status: backend.usbStatus
        }

        HwStatus {
            device: "USB OTG"
            status: backend.usbOtgStatus
        }

        HwStatus {
            device: "Eth0"
            status: backend.ethStatus
        }

        HwStatus {
            device: "SDCARD"
            status: backend.SDCardStatus
        }

        HwStatus {
            device: "RS485"
            status: backend.modbusStatus
        }

        /* TODO */
        HwStatus {
            device: "CAN"
            status: "FAILURE"
        }

        HwStatus {
            device: "RTC"
            status: backend.rtcStatus
        }
        HwStatus {
            device: "EEPROM FM24CL"
            status: backend.eeprom1Status
        }
        HwStatus {
            device: "EEPROM 24LC256"
            status: backend.eeprom2Status
        }
    }


}
