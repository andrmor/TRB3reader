#include "mainwindow.h"
#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QLoggingCategory>

#include "adatahub.h"
#include "masterconfig.h"
#include "trb3datareader.h"
#include "trb3signalextractor.h"
#include "adispatcher.h"
#include "ahldfileprocessor.h"
#include "anetworkmodule.h"

int main(int argc, char *argv[])
{
    //SUPPRESS WARNINGS about ssl
    QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");

    MasterConfig Config;
    ADataHub DataHub(Config);
    Trb3dataReader Reader(&Config);
    Trb3signalExtractor Extractor(&Config, &Reader);
    AHldFileProcessor HldFileProcessor(Config, Reader, Extractor, DataHub);
    ANetworkModule Network;

    ADispatcher Dispatcher(&Config, &Reader, &Extractor, &Network);

    QApplication a(argc, argv);
    MainWindow MW(&Config, &Dispatcher, &DataHub, &Reader, &Extractor, HldFileProcessor, Network);
    MW.show();

    QObject::connect(&Dispatcher, &ADispatcher::RequestUpdateGui, &MW, &MainWindow::UpdateGui);
    QObject::connect(&Dispatcher, &ADispatcher::RequestReadGuiFromJson, &MW, &MainWindow::ReadGUIfromJson);
    QObject::connect(&Dispatcher, &ADispatcher::RequestWriteGuiToJson, &MW, &MainWindow::WriteGUItoJson);
    QObject::connect(&Dispatcher, &ADispatcher::RequestWriteWindowSettings, &MW, &MainWindow::SaveWindowSettings);

    Dispatcher.LoadAutosaveConfig();
    MW.UpdateGui();

    return a.exec();
}
