#pragma once

#include <QMainWindow>
#include <QQmlEngine>
#include <pelican_core.hpp>

namespace PelicanStudio {

class MainWindow : public QMainWindow {
    QQmlEngine *engine;

  public:
    MainWindow(Pelican::PelicanCore &pcore);
};

} // namespace PelicanStudio