#include "mainwin.h"

#include <QtGui>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationName("Marum");
    a.setApplicationName("canview");

    MainWin w;
    w.show();
    return a.exec();
}
