#include "ascriptmanager.h"
#include "ainterfacetomessagewindow.h"
#include "coreinterfaces.h"
#include "ainterfacetoconfig.h"

#include <QScriptEngine>
#include <QMetaMethod>
#include <QDebug>
#include <QElapsedTimer>

AScriptManager::AScriptManager()
{
    engine = new QScriptEngine();
    engine->setProcessEventsInterval(200);

    fEngineIsRunning = false;
    bestResult = 1e30;
    numVariables = 0;

    timer = 0;
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

    delete timer;
}

QString AScriptManager::Evaluate(const QString& Script)
{
    LastError = "";
    fAborted = false;
    EvaluationResult = QScriptValue::UndefinedValue;

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
                  emit onFinished(LastError);
                  return LastError;
                }
            }
      }

    timer = new QElapsedTimer;
    timeOfStart = timer->restart();

    fEngineIsRunning = true;
    EvaluationResult = engine->evaluate(Script);
    //  qDebug() << "Just finished!" << EvaluationResult.toString();
    fEngineIsRunning = false;

    timerEvalTookMs = timer->elapsed();
    delete timer; timer = 0;

    QString result = EvaluationResult.toString();
    emit onFinished(result);

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
    //qDebug() << "Request recieved to evaluate script inside script:"<< script;
    result = coreObj->evaluate(script);
    //qDebug() << "res:"<<result;
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
        coreObj->setObjectName(coreName);
        interfaceNames.append(coreName);
        //registering math module
        QObject* mathObj = new AInterfaceToMath();
        QScriptValue mathVal = engine->newQObject(mathObj, QScriptEngine::QtOwnership);
        QString mathName = "math";
        engine->globalObject().setProperty(mathName, mathVal);
        interfaces.append(mathObj);
        interfaceNames.append(mathName);
        mathObj->setObjectName(mathName);
      }
    else
      { // name is not empty - this is one of the secondary modules
        engine->globalObject().setProperty(name, obj);
      }

    if (interfaceObject)
      {
        interfaces.append(interfaceObject);
        interfaceNames.append(name);
        interfaceObject->setObjectName(name);

        //connecting abort request from main interface to serviceObj
        int index = interfaceObject->metaObject()->indexOfSignal("AbortScriptEvaluation(QString)");
        if (index != -1) QObject::connect(interfaceObject, "2AbortScriptEvaluation(QString)",
                                          this, SLOT(AbortEvaluation(QString)));  //1-slot, 2-signal
        //connecting evaluate script-in-script to core object
        index = interfaceObject->metaObject()->indexOfSignal("RequestEvaluate(QString,QVariant&)");
        //  qDebug() << name << index;
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

#include "ascriptinterfacefactory.h"
#include "QScriptValueIterator"
//https://stackoverflow.com/questions/5020459/deep-copy-of-a-qscriptvalue-as-global-object

class ScriptCopier
{
public:
    ScriptCopier(QScriptEngine& toEngine)
        : m_toEngine(toEngine) {}

    QScriptValue copy(const QScriptValue& obj);

    QScriptEngine& m_toEngine;
    QMap<quint64, QScriptValue> copiedObjs;
};

QScriptValue ScriptCopier::copy(const QScriptValue& obj)
{
    QScriptEngine& engine = m_toEngine;

    if (obj.isUndefined()) return QScriptValue(QScriptValue::UndefinedValue);
    if (obj.isNull())      return QScriptValue(QScriptValue::NullValue);
    if (obj.isNumber() || obj.isString() || obj.isBool() || obj.isBoolean() || obj.isVariant())
    {
        //  qDebug() << "variant" << obj.toVariant();
        return engine.newVariant(obj.toVariant());
    }

    // If we've already copied this object, don't copy it again.
    QScriptValue copy;
    if (obj.isObject())
    {
        if (copiedObjs.contains(obj.objectId()))
        {
            return copiedObjs.value(obj.objectId());
        }
        copiedObjs.insert(obj.objectId(), copy);
    }

    if (obj.isQObject())
    {
        copy = engine.newQObject(copy, obj.toQObject());
        copy.setPrototype(this->copy(obj.prototype()));
    }
    else if (obj.isQMetaObject())
    {
        copy = engine.newQMetaObject(obj.toQMetaObject());
    }
    else if (obj.isFunction())
    {
        // Calling .toString() on a pure JS function returns
        // the function's source code.
        // On a native function however toString() returns
        // something like "function() { [native code] }".
        // That's why we do a syntax on the code.

        QString code = obj.toString();
        auto syntaxCheck = engine.checkSyntax(code);

        if (syntaxCheck.state() == syntaxCheck.Valid)
        {
            copy = engine.evaluate(QString() + "(" + code + ")");
        }
        else if (code.contains("[native code]"))
        {
            copy.setData(obj.data());
        }
        else
        {
            // Do error handling…
            qDebug() << "-----------------problem---------------";
        }

    }
    else if (obj.isObject() || obj.isArray())
    {
        if (obj.isObject()) {
            if (obj.scriptClass()) {
                copy = engine.newObject(obj.scriptClass(), this->copy(obj.data()));
            } else {
                copy = engine.newObject();
            }
        } else {
            copy = engine.newArray();
        }
        copy.setPrototype(this->copy(obj.prototype()));

        QScriptValueIterator it(obj);
        while ( it.hasNext())
        {
            it.next();

            const QString& name = it.name();
            const QScriptValue& property = it.value();

            copy.setProperty(name, this->copy(property));
        }
    }
    else
    {
        // Error handling…
        qDebug() << "-----------------problem---------------";
    }

    return copy;
}

AScriptManager *AScriptManager::createNewScriptManager()
{
    AScriptManager* sm = new AScriptManager();   

    for (QObject* io : interfaces)
    {
        AScriptInterface* si = dynamic_cast<AScriptInterface*>(io);
        if (!si) continue;

        if (!si->IsMultithreadCapable()) continue;

        QObject* copy = AScriptInterfaceFactory::makeCopy(io);
        if (copy)
        {
            qDebug() << "Making avauilable for multi-thread use: "<<io->objectName();
            sm->SetInterfaceObject(copy, io->objectName());

            //special for core unit
            AInterfaceToCore* core = dynamic_cast<AInterfaceToCore*>(copy);
            if (core)
            {
                //qDebug() << "--this is core";
                core->SetScriptManager(sm);
            }
        }
        else
        {
            qDebug() << "Unknown interface object type for unit" << io->objectName();
        }
    }

    QScriptValue global = engine->globalObject();
    ScriptCopier SC(*sm->engine);
    QScriptValueIterator it(global);
    while (it.hasNext())
    {
        it.next();
        //  qDebug() << it.name() << ": " << it.value().toString();

        if (!sm->engine->globalObject().property(it.name()).isValid())
        {
            //do not copy QObjects - the multi-thread friendly ones were already copied
            if (!it.value().isQObject())
            {
                sm->engine->globalObject().setProperty(it.name(), SC.copy(it.value()));
                //  qDebug() << "Registered:"<<it.name() << "-:->" << sm->engine->globalObject().property(it.name()).toVariant();
            }
            else
            {
                //  qDebug() << "Skipping QObject" << it.name();
            }
        }
    }

    return sm;
}

void AScriptManager::abortEvaluation()
{
    engine->abortEvaluation();
}

qint64 AScriptManager::getElapsedTime()
{
    if (timer) return timer->elapsed();
    return timerEvalTookMs;
}
