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
    QJsonObject* MakeConfigJson() const;  //does not own this object - delete after use

public slots:
    //QVariant getConfigJson();
    //void     setConfigJson(QVariant configJson);

    bool     setKeyValue(QString Key, QVariant val);
    QVariant getKeyValue(QString Key);

    int      countLogicalChannels();

    bool     isNegativeHardwareChannel(int iHardwChannel);
    bool     isNegativeLogicalChannel(int iLogicalChannel);

    bool     isIgnoredHardwareChannel(int iHardwChannel);
    bool     isIgnoredLogicalChannel(int iLogicalChannel);

    int      toHardware(int iLogicalChannel);
    int      toLogical(int iHardwChannel);

private:
    MasterConfig* Config;
    ADispatcher* Dispatcher;

    QString LastError;

    bool expandKey(QString &Key);
    void find(const QJsonObject &obj, QStringList Keys, QStringList& Found, QString Path = "");
    bool keyToNameAndIndex(QString Key, QString &Name, QVector<int> &Indexes);
    bool modifyJsonValue(QJsonObject& obj, const QString& path, const QJsonValue& newValue);
};

#endif // AINTERFACETOCONFIG_H
