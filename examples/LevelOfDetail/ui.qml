import QtQuick 2.3
import QtQuick.Window 2.2

import LevelOfDetail 1.0

Window {
    id: main_window
    width: 600
    height: 800
    color: "black"
    visible: true

    GLView {  // direct OpenGL rendering
        id: opengl_view
        anchors.fill: parent
    }

    MouseArea {  // rotate scene on drag
        anchors.fill: parent;
        onPressed: { opengl_view.rotation_start(Qt.point(mouse.x, mouse.y)); }
        onReleased: { opengl_view.rotation_finished(); }
        onPositionChanged: {
            opengl_view.rotation_changed(Qt.point(mouse.x, mouse.y));
        }
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
