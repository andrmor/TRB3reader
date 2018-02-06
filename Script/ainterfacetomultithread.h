#ifndef AINTERFACETOMULTITHREAD_H
#define AINTERFACETOMULTITHREAD_H

#include "ascriptinterface.h"

#include <QObject>

class AScriptManager;

class AInterfaceToMultiThread : public AScriptInterface
{
    Q_OBJECT

public:
    AInterfaceToMultiThread(AScriptManager *ScriptManager);

public slots:

    QString evaluateInNewThread(const QString script);

private:
    AScriptManager *ScriptManager;

};

#endif // AINTERFACETOMULTITHREAD_H
