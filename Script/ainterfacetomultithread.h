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

class AScriptEvalWorker : public QObject
{
  Q_OBJECT

public:
  AScriptEvalWorker(AScriptManager* ScriptManager, const QString& Script);
  AScriptEvalWorker();

  AScriptManager* ScriptManager;
  QString         Script;

public slots:
  void            Start();
};

#endif // AINTERFACETOMULTITHREAD_H
