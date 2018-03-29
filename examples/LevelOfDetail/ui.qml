import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.4

import LevelOfDetail 1.0

Window {
    id: main_window
    width: 600
    height: 800
    color: "black"
    visible: true

    MessageDialog {  // error reporting
        id: error_report
        title: "Processing error!"
        icon: StandardIcon.Warning
    }

    FileDialog {  // model selection
        id: model_selector
        title: "Select a model to display"
        folder: shortcuts.home
        onAccepted: {
            opengl_view.select_model(model_selector.fileUrl);
        }
    }

    GLView {  // direct OpenGL rendering
        id: opengl_view
        anchors.fill: parent
        onErrorEncountered: {
            error_report.text = what;
            error_report.visible = true;
        }
    }

    MouseArea {  // rotate scene on drag
        anchors.fill: parent;
        onPressed: { opengl_view.rotation_start(Qt.point(mouse.x, mouse.y)); }
        onReleased: { opengl_view.rotation_finished(); }
        onPositionChanged: {
            opengl_view.rotation_changed(Qt.point(mouse.x, mouse.y));
        }
        onWheel: {
            opengl_view.update_zoom(wheel.angleDelta.y);
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

        Button {
            text: "Select model to display"
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 24

            onClicked: {
                model_selector.visible = true;
            }
        }
    }
}
