#include "mainwindow.h"
#include "ascripthub.h"
#include "trb3datareader.h"
#include "trb3signalextractor.h"
#include "adispatcher.h"
#include "ahldfileprocessor.h"
#include "anetworkmodule.h"

#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QLoggingCategory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //SUPPRESS WARNINGS about ssl
    QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");

    Trb3dataReader Reader;
    Trb3signalExtractor Extractor(&Reader);
    AHldFileProcessor HldFileProcessor(Reader, Extractor);

//    !!!***
//    ANetworkModule Network(&ScriptManager); // !!!***
    ANetworkModule Network(nullptr); // !!!***

    ADispatcher Dispatcher(&Reader, &Extractor, &Network);

    AScriptHub & ScriptHub = AScriptHub::getInstance();
    ScriptHub.registerReaderModule(&Reader);
    ScriptHub.registerExtractorModule(&Extractor);
    ScriptHub.createInterfaces();
    ScriptHub.finalizeInit();

    MainWindow MW(&Dispatcher, &Reader, &Extractor, HldFileProcessor, Network);
    MW.show();

    QObject::connect(&Dispatcher, &ADispatcher::RequestUpdateGui, &MW, &MainWindow::UpdateGui);
    QObject::connect(&Dispatcher, &ADispatcher::RequestReadGuiFromJson, &MW, &MainWindow::ReadGUIfromJson);
    QObject::connect(&Dispatcher, &ADispatcher::RequestWriteGuiToJson, &MW, &MainWindow::WriteGUItoJson);
    QObject::connect(&Dispatcher, &ADispatcher::RequestWriteWindowSettings, &MW, &MainWindow::SaveWindowSettings);

    Dispatcher.LoadAutosaveConfig();
    MW.UpdateGui();

    return a.exec();
}
