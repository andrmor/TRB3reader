#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "masterconfig.h"
#include "afiletools.h"
#include "ascriptwindow.h"
#include "coreinterfaces.h"
#include "histgraphinterfaces.h"
#include "ainterfacetomessagewindow.h"
#include "ainterfacetosignals.h"

#ifdef CERN_ROOT
  #include "cernrootmodule.h"
#endif

#include <QDebug>

void MainWindow::CreateScriptWindow()
{
    qDebug() << "Creating script window...";
    ScriptWindow = new AScriptWindow(Config, this);

    qDebug() << "Registering script units...";

    qDebug() << "-> main...";
    ScriptWindow->SetInterfaceObject(0); //initialization

    qDebug() << "-> signals...";
    AInterfaceToSignals* sig = new AInterfaceToSignals(Extractor, Map);
    ScriptWindow->SetInterfaceObject(sig, "sig");

#ifdef CERN_ROOT
    qDebug() << "-> graph...";
    AInterfaceToGraph* graph = new AInterfaceToGraph(RootModule->GetTmpHub());
    ScriptWindow->SetInterfaceObject(graph, "graph");

    qDebug() << "-> hist...";
    AInterfaceToHist* hist = new AInterfaceToHist(RootModule->GetTmpHub());
    ScriptWindow->SetInterfaceObject(hist, "hist");
#endif

    qDebug() << "-> msg...";
    AInterfaceToMessageWindow* txt = new AInterfaceToMessageWindow(ScriptWindow);
    ScriptWindow->SetInterfaceObject(txt, "msg");

    qDebug() << "Done!";

    ScriptWindow->SetShowEvaluationResult(true);

    QObject::connect(ScriptWindow, SIGNAL(onStart()), this, SLOT(onGlobalScriptStarted()));
    QObject::connect(ScriptWindow, SIGNAL(success(QString)), this, SLOT(onGlobalScriptFinished()));

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
  //Config->AskForAllGuiUpdate();
  //if (ScriptWindow) ScriptWindow->updateJsonTree();
  this->setEnabled(true);
}

void MainWindow::on_actionOpen_script_window_triggered()
{
  ScriptWindow->showNormal();
  ScriptWindow->raise();
}
