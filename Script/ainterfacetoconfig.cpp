#include "ainterfacetoconfig.h"
#include "masterconfig.h"
#include "adispatcher.h"
#include "channelmapper.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>

AInterfaceToConfig::AInterfaceToConfig(MasterConfig *Config, ADispatcher* Dispatcher) :
    Config(Config), Dispatcher(Dispatcher)
{

}

QVariant AInterfaceToConfig::getConfigJson()
{
    QJsonObject js;
    Config->WriteToJson(js);
    return js.toVariantMap();
}

void AInterfaceToConfig::setConfigJson(QVariant configJson)
{
    QString type = configJson.typeName();
    if (type != "QVariantMap")
    {
        abort("Failed to set signal values - need object");
        return;
    }

    const QVariantMap mp = configJson.toMap();
    QJsonObject json = QJsonObject::fromVariantMap(mp);

    Dispatcher->LoadConfig(json);
}

int AInterfaceToConfig::countLogicalChannels()
{
    return Config->Map->GetNumLogicalChannels();
}

bool AInterfaceToConfig::isNegative(int iHardwChannel)
{
    return Config->IsNegative(iHardwChannel);
}

bool AInterfaceToConfig::isIgnoredChannel(int iHardwChannel)
{
    return Config->IsIgnoredChannel(iHardwChannel);
}

int AInterfaceToConfig::toHardware(int iLogicalChannel)
{
    int ihardw = Config->Map->LogicalToHardware(iLogicalChannel);
    if ( ihardw < 0 )
    {
        abort("Unmapped logical channel: "+QString::number(iLogicalChannel));
        return -1;
    }
    return ihardw;
}

int AInterfaceToConfig::toLogical(int iHardwChannel)
{
    int ilogical = Config->Map->HardwareToLogical(iHardwChannel);
    if ( ilogical < 0 )
    {
        abort("Invalid hardware channel: "+QString::number(iHardwChannel));
        return -1;
    }
    return ilogical;
}
