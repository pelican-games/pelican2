#include "uimain.hpp"
#include "mainwindow.hpp"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <iostream>

#include <pelican_core.hpp>
#include <thread>

namespace PelicanStudio {

int uimain(int argc, char **argv) {
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    std::cout << "starting Pelican Studio..." << std::endl;

    // necessary for exiting if error
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() {
            std::cerr << "failed to launch Qt application" << std::endl;
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    Pelican::PelicanCore pcore;
    MainWindow window{pcore};

    std::thread th([&]() { pcore.run(); });
    window.show();
    return app.exec();
}

} // namespace PelicanStudio
