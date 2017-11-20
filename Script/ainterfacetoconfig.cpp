#include "ainterfacetoconfig.h"
#include "masterconfig.h"
#include "adispatcher.h"

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

bool AInterfaceToConfig::isNegative_hardware(int ichannel)
{
    Config->IsNegative(ichannel);
}

bool AInterfaceToConfig::isNegative_logical(int ichannel)
{

}
