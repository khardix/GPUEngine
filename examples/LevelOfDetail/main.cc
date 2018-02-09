#include <iostream>
#include <stdexcept>

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[]) try {
    const auto &ui_description = QUrl("qrc:/ui.qml");

    const auto &app = QGuiApplication(argc, argv);
    const auto &qml = QQmlApplicationEngine(ui_description);

    return app.exec();
}
catch (const std::exception &exc) {
    std::cerr << "Fatal exception: " << exc.what() << '\n';
    return EXIT_FAILURE;
}
