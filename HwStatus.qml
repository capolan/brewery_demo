import QtQuick 2.9

Rectangle {
    id: rect

    property string status: "FAILURE"
    property string device: ""

    width: 250
    height: 100
    color: "red"

    /* Rectangle color reflect its state */
    onStatusChanged: {
        if (rect.status === "OK") {
            rect.color = "green";
        } else {
            rect.color = "red";
        }
    }

    Grid {

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        spacing: 10
        columns: 1
        rows: 2

        verticalItemAlignment: Grid.AlignVCenter
        horizontalItemAlignment: Grid.AlignHCenter

        Text {
            color: "lightgrey"
            text: rect.device
            font.bold: true
            font.pointSize: 22
        }

        Text {
            color: "lightgrey"
            font.bold: true
            font.pointSize: 28
            text: rect.status
        }
    }

    signal clicked;
    MouseArea {
        anchors.fill: parent
        onClicked: { rect.clicked(); }
    }
}
