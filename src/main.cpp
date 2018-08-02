#include "headers/mainwindow.h"
#include <QApplication>

#ifdef Q_OS_DARWIN
    #include "headers/macosuihandler.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    #ifdef Q_OS_DARWIN
    MacOSUIHandler::setTitleBar(w);
    #endif

    return a.exec();
}
