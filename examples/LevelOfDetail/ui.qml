import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

import LevelOfDetail 1.0

Window {
    id: main_window
    width: 1024
    height: 768
    color: "black"
    visible: true

    MessageDialog {  // error reporting
        id: error_report
        title: "Processing error!"
        icon: StandardIcon.Warning
        onAccepted: processingIndication.visible = false
    }

    FileDialog {  // model selection
        id: model_selector
        title: "Select a model to display"
        folder: shortcuts.home
        onAccepted: {
            opengl_view.model_selected(model_selector.fileUrl);
        }
    }

    BusyIndicator {
        id: processingIndication
        anchors.centerIn: parent
        visible: false
    }

    GLView {  // direct OpenGL rendering
        id: opengl_view
        focus: true
        anchors.fill: parent
        onErrorEncountered: {
            error_report.text = what;
            error_report.visible = true;
        }
        onSceneReset: {
            if (current_levels > 0) {
                level_selection.maximumValue = current_levels - 1;
            }
            else {
                level_selection.maximumValue = 0;
            }
            level_selection.value = 0;
            processingIndication.visible = false;
            opengl_view.focus = true;
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

        Keys.onPressed: {
            switch (event.key) {
                default: return;
                case Qt.Key_L:
                case Qt.Key_Right:
                    opengl_view.rotation_start(Qt.point(0, 0));
                    opengl_view.rotation_changed(Qt.point(3, 0));
                    opengl_view.rotation_finished();
                    break;
                case Qt.Key_H:
                case Qt.Key_Left:
                    opengl_view.rotation_start(Qt.point(0, 0));
                    opengl_view.rotation_changed(Qt.point(-3, 0));
                    opengl_view.rotation_finished();
                    break;

                case Qt.Key_Q:
                case Qt.Key_Escape:
                    Qt.quit();
                    break;
            }
            event.accepted = true;
        }
    }

    Rectangle {
        id: controls
        width: parent.width
        height: 48

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        color: Qt.rgba(1.0, 1.0, 1.0, 0.7)

        GridLayout {
            columns: 3
            rowSpacing: 1
            columnSpacing: 1

            anchors.fill: parent

            Slider {
                id: level_selection
                maximumValue: 0.0
                stepSize: 1.0
                tickmarksEnabled: true

                Layout.column: 0
                Layout.row: 0
                Layout.columnSpan: 3
                Layout.fillWidth: true
                Layout.fillHeight: true

                onValueChanged: {
                    console.log(level_selection.value);
                    opengl_view.level_selected(level_selection.value);
                }
            }

            Button {
                id: btn_select
                text: "Select model to display"

                onClicked: {
                    model_selector.visible = true;
                }

                Layout.column: 0
                Layout.row: 1
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            SpinBox {
                id: num_steps
                value: 10

                Layout.column: 1
                Layout.row: 1
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Button {
                id: btn_simplify;
                text: "Simplify"

                onClicked: {
                    processingIndication.visible = true;
                    opengl_view.generate_levels(num_steps.value);
                }

                Layout.column: 2
                Layout.row: 1
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
