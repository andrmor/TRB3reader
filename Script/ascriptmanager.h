#ifndef ASCRIPTMANAGER_H
#define ASCRIPTMANAGER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QScriptValue>

class QScriptEngine;
class QJsonObject;
class AInterfaceToCore;
class QElapsedTimer;

class AScriptManager : public QObject
{
    Q_OBJECT

public:
    AScriptManager();
    ~AScriptManager();    

    //configuration
    void            SetInterfaceObject(QObject* interfaceObject, QString name = ""); // empty name -> first (master) object

    //run
    int             FindSyntaxError(const QString& script); //returns line number of the first syntax error; -1 if no errors found
    QString         Evaluate(const QString& Script);
    QScriptValue    EvaluateScriptInScript(const QString& script);
    bool            isUncaughtException() const;
    int             getUncaughtExceptionLineNumber() const;
    const QString   getUncaughtExceptionString() const;

    QJsonObject*    CreateJsonOfConfig(); // does not own the created object!
    const QString   getFunctionReturnType(const QString UnitFunction);
    void            CollectGarbage();
    void            deleteMsgDialog();         //used in batch mode to force-close MSG window if shown
    void            hideMsgDialog();
    void            restoreMsgDialog();

    const QString   getLastError() const {return LastError;}

    bool            isEngineRunning() const {return fEngineIsRunning;}
    bool            isEvalAborted() const {return fAborted;}

    //starter dirs
    QString         LibScripts, LastOpenDir, ExamplesDir;

    //for minimizer
    QString         FunctName;
    double          bestResult;
    int             numVariables;

    //for multithread-in-scripting
    AScriptManager* createNewScriptManager();
    void            abortEvaluation();    
    QScriptValue    getProperty(const QString& properyName) const;
    QScriptValue    registerNewVariant(const QVariant &Variant);

    QScriptValue    EvaluationResult;

    qint64          getElapsedTime();

private:
    QScriptEngine*  engine;

    //registered objects
    QVector<QObject*> interfaces;
    AInterfaceToCore* coreObj;  //core interface - to forward evaluate-script-in-script

    bool              fEngineIsRunning;
    bool              fAborted;
    QString           LastError;

    QElapsedTimer*    timer;
    qint64            timeOfStart;
    qint64            timerEvalTookMs;


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
