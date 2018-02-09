import QtQuick 2.3
import QtQuick.Window 2.2

Window {
    id: main_window
    width: 600
    height: 800
    color: "black"
    visible: true

    MouseArea {  // close on click
        anchors.fill: parent;
        onClicked: { Qt.quit(); }
    }

    Rectangle {
        id: controls
        width: parent.width
        height: 200 // parent.heigth / 4

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        color: Qt.rgba(1.0, 1.0, 1.0, 0.7)

        Text {
            id: controls_placeholder
            text: "Placeholder for control elements"
            color: "black"
            anchors.centerIn: parent
        }
    }
}
