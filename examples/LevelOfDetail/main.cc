#include <iostream>
#include <stdexcept>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSurfaceFormat>

#include "gl_view.hh"

int main(int argc, char *argv[]) try {
    // Explicitly request OpenGL version 4.5
    // Otherwise, QtQuick defaults to version 2.1 (!)
    auto format_request = QSurfaceFormat();
    format_request.setProfile(QSurfaceFormat::CoreProfile);
    format_request.setMajorVersion(4);
    format_request.setMinorVersion(5);
    QSurfaceFormat::setDefaultFormat(format_request);

    const auto &ui_description = QUrl("qrc:/ui.qml");

    const auto &app = QGuiApplication(argc, argv);

    qmlRegisterType<GLView>("LevelOfDetail", 1, 0, "GLView");
    const auto &qml = QQmlApplicationEngine(ui_description);

    return app.exec();
}
catch (const std::exception &exc) {
    std::cerr << "Fatal exception: " << exc.what() << '\n';
    return EXIT_FAILURE;
}
