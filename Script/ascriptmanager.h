#ifndef ASCRIPTMANAGER_H
#define ASCRIPTMANAGER_H

#include <QObject>
#include <QVector>
#include <QString>

class QScriptEngine;

class AScriptManager : public QObject
{
    Q_OBJECT

public:
    AScriptManager();
    ~AScriptManager();    

    //configuration
    void SetInterfaceObject(QObject* interfaceObject, QString name = "");

    //run
    int     FindSyntaxError(const QString& script); //returns line number of the first syntax error; -1 if no errors found
    QString Evaluate(const QString& Script);

    void CollectGarbage();

    QScriptEngine* engine;
    QString LastError;

    bool fEngineIsRunning;
    bool fAborted;

    //registered objects
    QVector<QObject*> interfaces;
    QVector<QString> interfaceNames;

    //starter dirs
    QString LibScripts, LastOpenDir, ExamplesDir;

    //for minimizer
    QString FunctName;
    double bestResult;
    int numVariables;

    void deleteMsgDialog();  //needed in batch mode to force close MSG window if shown
    void hideMsgDialog();
    void restoreMsgDialog();

public slots:    
    void AbortEvaluation(QString message = "Aborted!");

signals:
    void onStart();
    void onAbort();
    void success(QString eval);
    void showMessage(QString message);
    void clearText();
};

#endif // ASCRIPTMANAGER_H
