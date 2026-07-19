#include "app.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);
    ToolkitApp toolkitApp;
    return a.exec();
}