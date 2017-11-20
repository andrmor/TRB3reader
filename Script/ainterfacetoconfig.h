#ifndef AINTERFACETOCONFIG_H
#define AINTERFACETOCONFIG_H

#include "ascriptinterface.h"
#include <QObject>
#include <QVariant>

class MasterConfig;
class ADispatcher;

class AInterfaceToConfig : public AScriptInterface
{
    Q_OBJECT

public:
    AInterfaceToConfig(MasterConfig* Config, ADispatcher *Dispatcher);

public slots:
    QVariant getConfigJson();
    void setConfigJson(QVariant configJson);

    bool isNegative_hardware(int ichannel);
    bool isNegative_logical(int ichannel);

private:
    MasterConfig* Config;
    ADispatcher* Dispatcher;

};

#endif // AINTERFACETOCONFIG_H
