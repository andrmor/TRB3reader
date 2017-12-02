#include "mainwindow.h"
#include <QApplication>
#include <QObject>

#include "adatahub.h"
#include "masterconfig.h"
#include "trb3datareader.h"
#include "trb3signalextractor.h"
#include "adispatcher.h"

int main(int argc, char *argv[])
{
    ADataHub DataHub;    
    MasterConfig Config;
    Trb3dataReader Reader(&Config);
    Trb3signalExtractor Extractor(&Config, &Reader);

    ADispatcher Dispatcher(&Config, &Reader, &Extractor);

    QApplication a(argc, argv);
    MainWindow MW(&Config, &Dispatcher, &DataHub, &Reader, &Extractor);
    MW.show();

    QObject::connect(&Dispatcher, &ADispatcher::RequestUpdateGui, &MW, &MainWindow::UpdateGui);
    QObject::connect(&Dispatcher, &ADispatcher::RequestReadGuiFromJson, &MW, &MainWindow::ReadGUIfromJson);
    QObject::connect(&Dispatcher, &ADispatcher::RequestWriteGuiToJson, &MW, &MainWindow::WriteGUItoJson);
    QObject::connect(&Dispatcher, &ADispatcher::RequestWriteWindowSettings, &MW, &MainWindow::SaveWindowSettings);

    Dispatcher.LoadAutosaveConfig();
    MW.UpdateGui();

    return a.exec();
}
