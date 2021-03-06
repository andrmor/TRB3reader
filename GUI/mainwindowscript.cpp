#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "masterconfig.h"
#include "afiletools.h"
#include "ascriptwindow.h"
#include "coreinterfaces.h"
#include "histgraphinterfaces.h"
#include "ainterfacetomessagewindow.h"
#include "ainterfacetoextractor.h"
#include "ainterfacetowaveforms.h"
#include "ainterfacetoconfig.h"
#include "adispatcher.h"
#include "ainterfacetodata.h"
#include "ainterfacetowebsocket.h"
#include "ainterfacetohldfileprocessor.h"
#include "ainterfacetomultithread.h"
#include "awebserverinterface.h"
#include "anetworkmodule.h"

#ifdef CERN_ROOT
  #include "cernrootmodule.h"
#endif

#ifdef SPEECH
  #include "ainterfacetospeech.h"
#endif

#include <QDebug>

void MainWindow::CreateScriptWindow()
{
    //  qDebug() << "Creating script window...";
    ScriptWindow = new AScriptWindow(Config, Network.getScriptManager(), this);

    //  qDebug() << "Registering script units...";

    //  qDebug() << "-> main...";
    ScriptWindow->SetInterfaceObject(0); //initialization

    //  qDebug() << "-> config...";
    AInterfaceToConfig* conf = new AInterfaceToConfig(Config, Dispatcher);
    ScriptWindow->SetInterfaceObject(conf, "config");

    //  qDebug() << "-> hld file processor...";
    AInterfaceToHldFileProcessor* hld = new AInterfaceToHldFileProcessor(HldFileProcessor);
    ScriptWindow->SetInterfaceObject(hld, "hld");

    //  qDebug() << "-> data hub...";
    AInterfaceToData* dat = new AInterfaceToData(DataHub);
    ScriptWindow->SetInterfaceObject(dat, "events");

    //  qDebug() << "-> waveforms...";
    AInterfaceToWaveforms* wav = new AInterfaceToWaveforms(Config, Reader);
    ScriptWindow->SetInterfaceObject(wav, "wav");

    //  qDebug() << "-> extractor...";
    AInterfaceToExtractor* ext = new AInterfaceToExtractor(Config, Extractor);
    ScriptWindow->SetInterfaceObject(ext, "ext");

#ifdef CERN_ROOT
    //  qDebug() << "-> graph...";
    AInterfaceToGraph* graph = new AInterfaceToGraph(RootModule->GetTmpHub());
    ScriptWindow->SetInterfaceObject(graph, "graph");

    //  qDebug() << "-> hist...";
    AInterfaceToHist* hist = new AInterfaceToHist(RootModule->GetTmpHub());
    ScriptWindow->SetInterfaceObject(hist, "hist");
#endif

#ifdef SPEECH
    //  qDebug() << "-> speech...";
    speech = new AInterfaceToSpeech();
    ScriptWindow->SetInterfaceObject(speech, "speech");
#endif

    AInterfaceToWebSocket* web = new AInterfaceToWebSocket();
    ScriptWindow->SetInterfaceObject(web, "web");

    AWebServerInterface* server = new AWebServerInterface(*Network.WebSocketServer);
    ScriptWindow->SetInterfaceObject(server, "server");



    //  qDebug() << "-> msg...";
    AInterfaceToMessageWindow* txt = new AInterfaceToMessageWindow(ScriptWindow);
    ScriptWindow->SetInterfaceObject(txt, "msg");

    AInterfaceToMultiThread* threads = new AInterfaceToMultiThread(ScriptWindow->GetScriptManager());
    ScriptWindow->SetInterfaceObject(threads, "threads");

    //  qDebug() << "Done!";

    ScriptWindow->SetShowEvaluationResult(true);

    QObject::connect(ScriptWindow, SIGNAL(onStart()), this, SLOT(onGlobalScriptStarted()));
    QObject::connect(ScriptWindow, SIGNAL(success(QString)), this, SLOT(onGlobalScriptFinished()));
    QObject::connect(ScriptWindow, &AScriptWindow::RequestStateSave, this, &MainWindow::saveCompleteState);
    QObject::connect(ScriptWindow, &AScriptWindow::RequestUpdateMainWindowGui, this, &MainWindow::UpdateGui);

#ifdef CERN_ROOT
    QObject::connect(ScriptWindow, SIGNAL(RequestDraw(TObject*,QString,bool)), RootModule, SLOT(onDrawRequested(TObject*,QString,bool)));
#endif

    ScriptWindow->UpdateHighlight();
}

void MainWindow::onGlobalScriptStarted()
{
  this->setEnabled(true);
  qApp->processEvents();
}

void MainWindow::onGlobalScriptFinished()
{
  UpdateGui();
  this->setEnabled(true);
}

void MainWindow::on_actionOpen_script_window_triggered()
{
  ScriptWindow->showNormal();
  ScriptWindow->raise();
}
