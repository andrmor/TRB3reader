#include "mainwindow.h"
#include <QApplication>

#include "adatahub.h"
#include "masterconfig.h"
#include "trb3datareader.h"
#include "trb3signalextractor.h"

int main(int argc, char *argv[])
{
    ADataHub DataHub;    
    MasterConfig Config;
    Trb3dataReader Reader(&Config);
    Trb3signalExtractor Extractor(&Config, &Reader);

    QApplication a(argc, argv);
    MainWindow w(&DataHub, &Config, &Reader, &Extractor);
    w.show();

    return a.exec();
}
