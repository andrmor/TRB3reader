#include "ainterfacetomultithread.h"
#include "ascriptmanager.h"

#include <QThread>
#include <QDebug>
#include <QApplication>
#include <QScriptEngine>

AInterfaceToMultiThread::AInterfaceToMultiThread(AScriptManager *ScriptManager) :
  MasterScriptManager(ScriptManager) {}

void AInterfaceToMultiThread::ForceStop()
{
    abortAll();
}

void AInterfaceToMultiThread::evaluateScript(const QString script)
{
    AScriptManager* sm = MasterScriptManager->createNewScriptManager();
    //  qDebug() << "Cloned SM. master:"<<MasterScriptManager<<"clone:"<<sm;

    AScriptThreadScr* worker = new AScriptThreadScr(sm, script);
    startEvaluation(sm, worker);
}

void AInterfaceToMultiThread::evaluateFunction(const QString functionName, const QVariant arguments)
{
    AScriptManager* sm = MasterScriptManager->createNewScriptManager();
    //  qDebug() << "Cloned SM. master:"<<MasterScriptManager<<"clone:"<<sm;

    AScriptThreadFun* worker = new AScriptThreadFun(sm, functionName, arguments);
    startEvaluation(sm, worker);
}

void AInterfaceToMultiThread::startEvaluation(AScriptManager* sm, AScriptThreadBase *worker)
{
    workers << worker;

    connect(sm, &AScriptManager::showMessage, MasterScriptManager, &AScriptManager::showMessage, Qt::QueuedConnection);

    QThread* t = new QThread();
    QObject::connect(t,  &QThread::started, worker, &AScriptThreadBase::Run);
    QObject::connect(sm, &AScriptManager::onFinished, t, &QThread::quit);
    QObject::connect(t, &QThread::finished, t, &QThread::deleteLater);
    worker->moveToThread(t);
    t->start();

    //  qDebug() << "Started new thread!";
}

void AInterfaceToMultiThread::waitForAll()
{
    while (countNotFinished() > 0)
      {
        qApp->processEvents();
      }
}

void AInterfaceToMultiThread::waitForOne(int IndexOfWorker)
{
  if (IndexOfWorker < 0 || IndexOfWorker >= workers.size()) return;
  if (!workers.at(IndexOfWorker)->isRunning()) return;

  while (workers.at(IndexOfWorker)->isRunning())
    {
      qApp->processEvents();
    }
}

void AInterfaceToMultiThread::abortAll()
{
  for (AScriptThreadBase* w : workers)
     if (w->isRunning()) w->abort();
}

void AInterfaceToMultiThread::abortOne(int IndexOfWorker)
{
  if (IndexOfWorker < 0 || IndexOfWorker >= workers.size()) return;
  if (!workers.at(IndexOfWorker)->isRunning()) return;

  workers.at(IndexOfWorker)->abort();
}

int AInterfaceToMultiThread::countAll()
{
  return workers.size();
}

int AInterfaceToMultiThread::countNotFinished()
{
  //  qDebug() << "Total number of workers:"<< workers.count();
  int counter = 0;
  for (AScriptThreadBase* w : workers)
    {
        //  qDebug() << w << w->ScriptManager << w->ScriptManager->fEngineIsRunning;
        if (w->isRunning()) counter++;
    }
  return counter;
}

QVariant AInterfaceToMultiThread::getResult(int IndexOfWorker)
{
  if (IndexOfWorker < 0 || IndexOfWorker >= workers.size()) return QString("Wrong worker index");
  if (workers.at(IndexOfWorker)->isRunning()) return QString("Still running");

  return workers.at(IndexOfWorker)->getResult();
}

bool AInterfaceToMultiThread::deleteAll()
{
    for (AScriptThreadBase* w : workers)
      if (w->isRunning())
      {
          qDebug() << "Cannot delete all - not all threads finished";
          return false;
      }

    for (AScriptThreadBase* w : workers) w->deleteLater();
    workers.clear();

    return true;
}

const QString AInterfaceToMultiThread::deleteOne(int IndexOfWorker)
{
   if (IndexOfWorker < 0 || IndexOfWorker >= workers.size()) return QString("Wrong worker index");
   if (workers.at(IndexOfWorker)->isRunning()) return QString("Still running");

   delete workers[IndexOfWorker];
   workers.removeAt(IndexOfWorker);

   return "";
}

AScriptThreadBase::AScriptThreadBase(AScriptManager *ScriptManager) : ScriptManager(ScriptManager) {}

AScriptThreadBase::~AScriptThreadBase()
{
    delete ScriptManager;
}

void AScriptThreadBase::abort()
{
    if (ScriptManager) ScriptManager->abortEvaluation();
}

const QVariant AScriptThreadBase::resultToQVariant(const QScriptValue &result) const
{
    if (result.isString()) return result.toString();
    if (result.isNumber()) return result.toNumber();
    return result.toVariant();
}

AScriptThreadScr::AScriptThreadScr(AScriptManager *ScriptManager, const QString &Script) :
    AScriptThreadBase(ScriptManager), Script(Script) {}

void AScriptThreadScr::Run()
{
    bRunning = true;
    ScriptManager->Evaluate(Script);
    Result = resultToQVariant( ScriptManager->EvaluationResult );
    //  qDebug() << Result;
    bRunning = false;
}

AScriptThreadFun::AScriptThreadFun(AScriptManager *ScriptManager, const QString &Function, const QVariant &Arguments) :
    AScriptThreadBase(ScriptManager), Function(Function), Arguments(Arguments) {}

void AScriptThreadFun::Run()
{
    bRunning = true;
    QScriptValue global = ScriptManager->engine->globalObject();
    QScriptValue func = global.property(Function);
    if (!func.isValid())         Result = "Cannot evaluate: " + Function + " not found";
    else if (!func.isFunction()) Result = "Cannot evaluate: " + Function + " is not a function";
    else
    {
        qDebug() << Arguments;

        QVariantList ArgsVL;
        QString typeArr = Arguments.typeName();
        if (typeArr == "QVariantList") ArgsVL = Arguments.toList();
        else ArgsVL << Arguments;

        QScriptValueList args;
        for (const QVariant& v : ArgsVL)
        {
            //to do ***!!! check for objects and array of arrays

            args << ScriptManager->engine->newVariant(v);
        }

        QScriptValue res = func.call(QScriptValue(), args);
        Result = resultToQVariant(res);
    }
    bRunning = false;
}
