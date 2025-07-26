#include "mainwindow.h"
#include "QStyleFactory"
#include <QApplication>

int main(int argc, char *argv[])
{
    qputenv("QT_MEDIA_BACKEND", "windows");
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));
    MainWindow w;
    w.show();
    return a.exec();
}
