#ifndef AINTERFACETOMULTITHREAD_H
#define AINTERFACETOMULTITHREAD_H

#include "ascriptinterface.h"

#include <QString>
#include <QObject>
#include <QVector>
#include <QScriptValue>

class AScriptManager;
class QThread;

class AScriptEvalWorker;

class AInterfaceToMultiThread : public AScriptInterface
{
    Q_OBJECT

public:
    AInterfaceToMultiThread(AScriptManager *MasterScriptManager);

    virtual void ForceStop();

public slots:

    void          evaluateScript(const QString script);
    void          evaluateFunction(const QString functionName);

    void          waitForAll();
    void          waitForWorker(int IndexOfWorker);

    void          abortAll();
    void          abortWorker(int IndexOfWorker);

    int           countWorkers();
    int           countActiveWorkers();

    QScriptValue  getResult(int IndexOfWorker);

    bool          deleteAllWorkers();
    const QString deleteWorker(int IndexOfWorker);

private:
    AScriptManager *MasterScriptManager;
    QVector<AScriptEvalWorker*> workers;
};

class AScriptThreadBase : public QObject
{
    Q_OBJECT

public:
    AScriptThreadBase(){}
    AScriptThreadBase(AScriptManager* ScriptManager) : ScriptManager(ScriptManager) {}
    virtual ~AScriptThreadBase(){}

    virtual bool    isRunning() = 0;

public slots:
    virtual void    Run();

private:
    AScriptManager* ScriptManager = 0;
    bool            bRunning = false;
    QScriptValue    Result = "Evaluation was not yet performed";
};

class AScriptEvalWorker : public QObject
{
  Q_OBJECT

public:
  AScriptEvalWorker(AScriptManager* ScriptManager, const QString& Script);
  AScriptEvalWorker(AScriptManager* ScriptManager, const QString& Function, const QVariantList& Args);
  AScriptEvalWorker();

  AScriptManager* ScriptManager;
  QString         Script;
  QString         Function;

  QScriptValue    Result;

public slots:
  void            RunScript();
  void            RunFunction();
};

#endif // AINTERFACETOMULTITHREAD_H
