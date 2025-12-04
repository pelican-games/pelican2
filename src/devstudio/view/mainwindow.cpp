#include "mainwindow.hpp"
#include "pelicancoreglue.hpp"
#include "playerscreen.hpp"
#include "viewmodel/testbackend.hpp"
#include <QDockWidget>
#include <QObject>
#include <QQmlContext>
#include <QQuickWidget>
#include <iostream>

namespace PelicanStudio {

MainWindow::MainWindow(Pelican::PelicanCore &pcore) {
    engine = new QQmlEngine(this);
    engine->rootContext()->setContextProperty("backend", new TestBackend());

    // for test
    auto player = new PlayerScreen();
    setCentralWidget(player);

    auto glue = new PelicanCoreGlue(player);
    pcore.setScreen(glue);

    QDockWidget *leftDock = new QDockWidget("left", this);
    QQuickWidget *leftQml = new QQuickWidget(leftDock);
    leftQml->setSource(QUrl(QStringLiteral("qrc:/qt/qml/pelican_studio/ProjectEdit.qml")));
    leftQml->setResizeMode(QQuickWidget::SizeRootObjectToView);
    leftDock->setWidget(leftQml);
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);
}

} // namespace PelicanStudio
