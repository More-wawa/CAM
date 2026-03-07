#include <QApplication>
#include <qvtkopenglnativewidget.h>
#include "../include/mainwindow.h"

int main(int argc, char* argv[]) {
    QSurfaceFormat::setDefaultFormat(QSurfaceFormat::defaultFormat());

    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return QApplication::exec();
}
