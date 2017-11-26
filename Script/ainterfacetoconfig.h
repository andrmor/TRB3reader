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
    void     setConfigJson(QVariant configJson);

    int      countLogicalChannels();

    bool     isNegative(int iHardwChannel);
    bool     isIgnoredChannel(int iHardwChannel);

    int      toHardware(int iLogicalChannel);
    int      toLogical(int iHardwChannel);

private:
    MasterConfig* Config;
    ADispatcher* Dispatcher;

};

#endif // AINTERFACETOCONFIG_H