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

        spacing: 10
        columns: 4
        rows: 5

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
//        HwStatus {
//            device: "CAN"
//            status: "FAILURE"
//        }

        HwStatus {
            id: rtcstat
            device: "RTC"
            status: backend.rtcStatus
            onStatusChanged: {
                if (rtcstat.status != "FAILURE")
                    color = "green";
            }
            textfontsize: 18
        }
        HwStatus {
            device: "FM24CL"
            status: backend.eeprom1Status
        }
        HwStatus {
            device: "24LC256"
            status: backend.eeprom2Status
        }

        HwStatus {
            device: "DIO1 (IN)"
            status: backend.dio1Status
            onStatusChanged: { color = "blue"}
        }

        HwStatus {
            device: "DIO2 (IN)"
            status: backend.dio2Status
            onStatusChanged: { color = "blue"}
        }

        HwStatus {
            device: "DIO3 (OUT)"
            status: backend.dio3Status
            onStatusChanged: { color = "blue"}
        }

        HwStatus {
            device: "DIO4 (OUT)"
            status: backend.dio4Status
            onStatusChanged: { color = "blue"}
        }

        HwStatus {
            id: touch
            device: "Touch"
            status: tap_count
            property int tap_count: 0
            color: "green"
            onStatusChanged: { color = "green"}
            onClicked: {
                touch.tap_count += 1;
            }
        }

        /* Free Slot */
        HwStatus {
            color: "lightgrey"
        }

        HwStatus {
            device: "AOUT1"
            status: backend.aout1Status
            onStatusChanged: { color = "blue"}
        }

        HwStatus {
            device: "AOUT2"
            status: backend.aout2Status
            onStatusChanged: { color = "blue"}
        }

        HwStatus {
            device: "AIN1 (TC)"
            status: backend.ain1Status
            onStatusChanged: { color = "blue"}
        }

        HwStatus {
            device: "AIN2 (PT100)"
            status: backend.ain2Status
            onStatusChanged: { color = "blue"}
        }

        HwStatus {
            device: "AIN3 (0-10V)"
            status: backend.ain3Status
            onStatusChanged: { color = "blue"}
        }

        HwStatus {
            device: "AIN4 (4-20mA)"
            status: backend.ain4Status
            onStatusChanged: { color = "blue" }
        }

    }
}
