#include "mainwindow.h"
#include <QApplication>

#include "adatahub.h"

int main(int argc, char *argv[])
{
    ADataHub DataHub;

    QApplication a(argc, argv);
    MainWindow w(&DataHub);
    w.show();

    return a.exec();
}
