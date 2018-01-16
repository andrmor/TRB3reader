#include "ascriptmanager.h"
#include "ainterfacetomessagewindow.h"
#include "coreinterfaces.h"
#include "ainterfacetoconfig.h"

#include <QScriptEngine>
#include <QMetaMethod>
#include <QDebug>

AScriptManager::AScriptManager()
{
    engine = new QScriptEngine();
    engine->setProcessEventsInterval(200);

    fEngineIsRunning = false;
    bestResult = 1e30;
    numVariables = 0;
}

AScriptManager::~AScriptManager()
{
    for (int i=0; i<interfaces.size(); i++) delete interfaces[i];
    interfaces.clear();

    if (engine)
    {
        engine->abortEvaluation();
        engine->deleteLater();
        engine = 0;
    }
}

QString AScriptManager::Evaluate(const QString& Script)
{
    LastError = "";
    fAborted = false;

    emit onStart();

    //running InitOnRun method (if defined) for all defined interfaces
    for (int i=0; i<interfaces.size(); i++)
      {
          AScriptInterface* bi = dynamic_cast<AScriptInterface*>(interfaces[i]);
          if (bi)
            {
              if (!bi->InitOnRun())
                {
                  LastError = "Init failed for unit: "+interfaceNames.at(i);
                  return LastError;
                }
            }
      }

    fEngineIsRunning = true;
    QScriptValue scriptreturn = engine->evaluate(Script);
    fEngineIsRunning = false;

    QString result = scriptreturn.toString();
    emit success(result); //bad name here :) could be not successful at all, but still want to trigger

    return result;
}

QScriptValue AScriptManager::EvaluateScriptInScript(const QString &script)
{
    return engine->evaluate(script);
}

bool AScriptManager::isUncaughtException() const
{
    return engine->hasUncaughtException();
}

int AScriptManager::getUncaughtExceptionLineNumber() const
{
    return engine->uncaughtExceptionLineNumber();
}

const QString AScriptManager::getUncaughtExceptionString() const
{
    return engine->uncaughtException().toString();
}

QJsonObject *AScriptManager::CreateJsonOfConfig()
{
    for (int i=0; i<interfaces.size(); i++)
    {
        AInterfaceToConfig* inter = dynamic_cast<AInterfaceToConfig*>(interfaces[i]);
        if (!inter) continue;

        QJsonObject* json = inter->MakeConfigJson();
        return json;
    }
    return 0;
}

void AScriptManager::CollectGarbage()
{
    engine->collectGarbage();
}

void AScriptManager::AbortEvaluation(const QString message)
{
    //qDebug() << "ScriptManager: Abort requested!"<<fAborted<<fEngineIsRunning;

    if (fAborted || !fEngineIsRunning) return;
    fAborted = true;

    engine->abortEvaluation();
    fEngineIsRunning = false;

    // going through registered units and requesting abort
    for (int i=0; i<interfaces.size(); i++)
      {
        AScriptInterface* bi = dynamic_cast<AScriptInterface*>(interfaces[i]);
        if (bi) bi->ForceStop();
      }

    emit showMessage("<font color=\"red\">"+ message +"</font><br>");
    emit onAbort();
}

void AScriptManager::EvaluateScriptInScript(const QString &script, QVariant &result)
{
    qDebug() << "ahaaaaaaaa"<< script;
    result = coreObj->evaluate(script);
    qDebug() << "res:"<<result;
}

void AScriptManager::SetInterfaceObject(QObject *interfaceObject, QString name)
{
    //qDebug() << "Registering:" << interfaceObject << name;
    QScriptValue obj = ( interfaceObject ? engine->newQObject(interfaceObject, QScriptEngine::QtOwnership) : QScriptValue() );

    if (name.isEmpty())
      { // empty name means the main module
        if (interfaceObject)
           engine->setGlobalObject(obj); //do not replace the global object for global script - in effect (non zero pointer) only for local scripts
        // registering service object
        coreObj = new AInterfaceToCore(this);
        QScriptValue coreVal = engine->newQObject(coreObj, QScriptEngine::QtOwnership);
        QString coreName = "core";
        engine->globalObject().setProperty(coreName, coreVal);
        interfaces.append(coreObj);  //CORE OBJECT IS FIRST in interfaces!
        interfaceNames.append(coreName);
        //registering math module
        QObject* mathObj = new AInterfaceToMath();
        QScriptValue mathVal = engine->newQObject(mathObj, QScriptEngine::QtOwnership);
        QString mathName = "math";
        engine->globalObject().setProperty(mathName, mathVal);
        interfaces.append(mathObj);
        interfaceNames.append(mathName);
      }
    else
      { // name is not empty - this is one of the secondary modules
        engine->globalObject().setProperty(name, obj);
      }

    if (interfaceObject)
      {
        interfaces.append(interfaceObject);
        interfaceNames.append(name);

        //connecting abort request from main interface to serviceObj
        int index = interfaceObject->metaObject()->indexOfSignal("AbortScriptEvaluation(QString)");
        if (index != -1) QObject::connect(interfaceObject, "2AbortScriptEvaluation(QString)",
                                          this, SLOT(AbortEvaluation(QString)));  //1-slot, 2-signal
        //connecting evaluate script-in-script to core object
        index = interfaceObject->metaObject()->indexOfSignal("RequestEvaluate(QString,QVariant&)");
        qDebug() << name << index;
        if (index != -1) QObject::connect(interfaceObject, "2RequestEvaluate(QString,QVariant&)",
                                          this, SLOT(EvaluateScriptInScript(const QString&, QVariant&)));  //1-slot, 2-signal
      }
}

int AScriptManager::FindSyntaxError(const QString& script)
{
    QScriptSyntaxCheckResult check = QScriptEngine::checkSyntax(script);
    if (check.state() == QScriptSyntaxCheckResult::Valid) return -1;
    else
      {
        int lineNumber = check.errorLineNumber();
        qDebug()<<"Syntax error at line"<<lineNumber;
        return lineNumber;
      }
}

void AScriptManager::deleteMsgDialog()
{
    for (int i=0; i<interfaces.size(); i++)
    {
        AInterfaceToMessageWindow* t = dynamic_cast<AInterfaceToMessageWindow*>(interfaces[i]);
        if (t)
        {
            t->deleteDialog();
            return;
        }
    }
}

void AScriptManager::hideMsgDialog()
{
    for (int i=0; i<interfaces.size(); i++)
    {
        AInterfaceToMessageWindow* t = dynamic_cast<AInterfaceToMessageWindow*>(interfaces[i]);
        if (t)
        {
            t->hideDialog();
            return;
        }
    }
}

void AScriptManager::restoreMsgDialog()
{
    for (int i=0; i<interfaces.size(); i++)
    {
        AInterfaceToMessageWindow* t = dynamic_cast<AInterfaceToMessageWindow*>(interfaces[i]);
        if (t)
        {
            if (t->isActive()) t->restore();
            return;
        }
    }
}

const QString AScriptManager::getFunctionReturnType(const QString UnitFunction)
{
  QStringList f = UnitFunction.split(".");
  if (f.size() != 2) return "";

  QString unit = f.first();
  int unitIndex = interfaceNames.indexOf(unit);
  if (unitIndex == -1) return "";
  //qDebug() << "Found unit"<<unit<<" with index"<<unitIndex;
  QString met = f.last();
  //qDebug() << met;
  QStringList skob = met.split("(", QString::SkipEmptyParts);
  if (skob.size()<2) return "";
  QString funct = skob.first();
  QString args = skob[1];
  args.chop(1);
  //qDebug() << funct << args;

  QString insert;
  if (!args.isEmpty())
    {
      QStringList argl = args.split(",");
      for (int i=0; i<argl.size(); i++)
        {
          QStringList a = argl.at(i).simplified().split(" ");
          if (!insert.isEmpty()) insert += ",";
          insert += a.first();
        }
    }
  //qDebug() << insert;

  QString methodName = funct + "(" + insert + ")";
  //qDebug() << "method name" << methodName;
  int mi = interfaces.at(unitIndex)->metaObject()->indexOfMethod(methodName.toLatin1().data());
  //qDebug() << "method index:"<<mi;
  if (mi == -1) return "";

  QString returnType = interfaces.at(unitIndex)->metaObject()->method(mi).typeName();
  //qDebug() << returnType;
  return returnType;
}
