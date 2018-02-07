#ifndef ASCRIPTMANAGER_H
#define ASCRIPTMANAGER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QScriptValue>

class QScriptEngine;
class QJsonObject;
class AInterfaceToCore;

class AScriptManager : public QObject
{
    Q_OBJECT

public:
    AScriptManager();
    ~AScriptManager();    

    //configuration
    void          SetInterfaceObject(QObject* interfaceObject, QString name = "");

    //run
    int           FindSyntaxError(const QString& script); //returns line number of the first syntax error; -1 if no errors found
    QString       Evaluate(const QString& Script);
    QScriptValue  EvaluateScriptInScript(const QString& script);
    bool          isUncaughtException() const;
    int           getUncaughtExceptionLineNumber() const;
    const QString getUncaughtExceptionString() const;

    QJsonObject*  CreateJsonOfConfig(); // does not own the created object!
    const QString getFunctionReturnType(const QString UnitFunction);
    void          CollectGarbage();
    void          deleteMsgDialog();         //needed in batch mode to force close MSG window if shown
    void          hideMsgDialog();
    void          restoreMsgDialog();

    QString       LastError;

    bool          fEngineIsRunning;
    bool          fAborted;

    //starter dirs
    QString       LibScripts, LastOpenDir, ExamplesDir;

    //for minimizer
    QString       FunctName;
    double        bestResult;
    int           numVariables;

    //for multithread-in-scripting
    AScriptManager* createNewScriptManager();
    void          abortEvaluation();
    QScriptValue  EvaluationResult;

//private:
    QScriptEngine*  engine;

private:

    //registered objects
    QVector<QObject*> interfaces;
    QVector<QString>  interfaceNames;

    AInterfaceToCore* coreObj;  //core interface - to forward evaluate-script-in-script

public slots:
    void    AbortEvaluation(const QString message = "Aborted!");
    void    EvaluateScriptInScript(const QString& script, QVariant& result);

signals:
    void    onStart();
    void    onAbort();
    void    onFinished(QString eval);

    void    showMessage(QString message);
    void    clearText();
};

#endif // ASCRIPTMANAGER_H
