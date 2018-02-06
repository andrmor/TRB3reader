#include "ainterfacetomultithread.h"
#include "ascriptmanager.h"

#include <QThread>
#include <QDebug>
#include <QApplication>

AInterfaceToMultiThread::AInterfaceToMultiThread(AScriptManager *ScriptManager) :
  MasterScriptManager(ScriptManager) {}

void AInterfaceToMultiThread::ForceStop()
{
    abortAll();
}

const QString AInterfaceToMultiThread::evaluateInNewThread(const QString script)
{
    AScriptManager* sm = MasterScriptManager->createNewScriptManager();
    qDebug() << "Cloned SM. master:"<<MasterScriptManager<<"clone:"<<sm;

    AScriptEvalWorker* worker = new AScriptEvalWorker(sm, script);
    workers << worker;

    connect(sm, &AScriptManager::showMessage, MasterScriptManager, &AScriptManager::showMessage, Qt::QueuedConnection);

    QThread* t = new QThread();
    QObject::connect(t,  &QThread::started, worker, &AScriptEvalWorker::Start);
    QObject::connect(sm, &AScriptManager::onFinished, t, &QThread::quit);
    QObject::connect(t, &QThread::finished, t, &QThread::deleteLater);
    worker->moveToThread(t);
    t->start();

    qDebug() << "Started!";
    return "started";
}

void AInterfaceToMultiThread::waitForAll()
{
    while (countActiveWorkers() > 0)
      {
        qApp->processEvents();
      }
}

void AInterfaceToMultiThread::waitForWorker(int IndexOfWorker)
{
  if (IndexOfWorker < 0 || IndexOfWorker >= workers.size()) return;
  if (!workers.at(IndexOfWorker)->ScriptManager->fEngineIsRunning) return;

  while (workers.at(IndexOfWorker)->ScriptManager->fEngineIsRunning)
    {
      qApp->processEvents();
    }
}

void AInterfaceToMultiThread::abortAll()
{
  for (AScriptEvalWorker* w : workers)
     if (w->ScriptManager->fEngineIsRunning) w->ScriptManager->abortEvaluation();
}

void AInterfaceToMultiThread::abortWorker(int IndexOfWorker)
{
  if (IndexOfWorker < 0 || IndexOfWorker >= workers.size()) return;
  if (!workers.at(IndexOfWorker)->ScriptManager->fEngineIsRunning) return;

  workers.at(IndexOfWorker)->ScriptManager->abortEvaluation();
}

int AInterfaceToMultiThread::countWorkers()
{
  return workers.size();
}

int AInterfaceToMultiThread::countActiveWorkers()
{
  //  qDebug() << "Total number of workers:"<< workers.count();
  int counter = 0;
  for (AScriptEvalWorker* w : workers)
    {
        //  qDebug() << w << w->ScriptManager << w->ScriptManager->fEngineIsRunning;
        if (w->ScriptManager->fEngineIsRunning) counter++;
    }
  return counter;
}

QScriptValue AInterfaceToMultiThread::getResult(int IndexOfWorker)
{
  if (IndexOfWorker < 0 || IndexOfWorker >= workers.size()) return QString("Wrong worker index");
  if (workers.at(IndexOfWorker)->ScriptManager->fEngineIsRunning) return QString("Still running");

  return workers.at(IndexOfWorker)->ScriptManager->EvaluationResult;
}

bool AInterfaceToMultiThread::deleteAllWorkers()
{
    for (AScriptEvalWorker* w : workers)
      if (w->ScriptManager->fEngineIsRunning) return false;

    for (AScriptEvalWorker* w : workers) delete w;
    workers.clear();
    return true;
}

const QString AInterfaceToMultiThread::deleteWorker(int IndexOfWorker)
{
   if (IndexOfWorker < 0 || IndexOfWorker >= workers.size()) return QString("Wrong worker index");
   if (workers.at(IndexOfWorker)->ScriptManager->fEngineIsRunning) return QString("Still running");

   delete workers[IndexOfWorker];
   workers.removeAt(IndexOfWorker);
}

AScriptEvalWorker::AScriptEvalWorker(AScriptManager *ScriptManager, const QString &Script) :
  ScriptManager(ScriptManager), Script(Script) {}

void AScriptEvalWorker::Start()
{
   ScriptManager->Evaluate(Script);
}
