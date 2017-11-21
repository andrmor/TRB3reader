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
    Config->Map->GetNumLogicalChannels();
}

bool AInterfaceToConfig::isNegative_hardware(int ichannel)
{
    return Config->IsNegative(ichannel);
}

bool AInterfaceToConfig::isNegative_logical(int ichannel)
{
    int ihardw = Config->Map->LogicalToHardware(ichannel);
    if (std::isnan(ihardw))
    {
        abort("Unmapped logical channel: "+QString::number(ichannel));
        return false;
    }
    return Config->IsNegative(ihardw);
}

bool AInterfaceToConfig::isIgnoredChannel_hardware(int ichannel)
{
    return Config->IgnoreHardwareChannels.contains(ichannel);
}

bool AInterfaceToConfig::isIgnoredChannel_logical(int ichannel)
{
    int ihardw = Config->Map->LogicalToHardware(ichannel);
    if (std::isnan(ihardw))
    {
        abort("Unmapped logical channel: "+QString::number(ichannel));
        return false;
    }
    return Config->IgnoreHardwareChannels.contains(ihardw);
}

int AInterfaceToConfig::toHardware(int iLogicalChannel)
{
    int ihardw = Config->Map->LogicalToHardware(iLogicalChannel);
    if (std::isnan(ihardw))
    {
        abort("Unmapped logical channel: "+QString::number(iLogicalChannel));
        return -1;
    }
    return ihardw;
}

int AInterfaceToConfig::toLogical(int iHardwChannel)
{
    int ilogical = Config->Map->HardwareToLogical(iHardwChannel);
    if (std::isnan(ilogical))
    {
        abort("Invalid hardware channel: "+QString::number(iHardwChannel));
        return -1;
    }
    return ilogical;
}
