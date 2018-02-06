#include "ainterfacetomultithread.h"
#include "ascriptmanager.h"

#include <QDebug>

AInterfaceToMultiThread::AInterfaceToMultiThread(AScriptManager *ScriptManager) :
    ScriptManager(ScriptManager) {}

QString AInterfaceToMultiThread::evaluateInNewThread(const QString script)
{
    AScriptManager* sm = ScriptManager->createNewScriptManager();

    //connect(sm, &AScriptManager::showMessage, ScriptManager, &AScriptManager::showMessage);

    QString res = sm->Evaluate(script);

    delete sm;
    return res;
}
