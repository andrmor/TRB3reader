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
    QVariant getConfigJson();
    void     setConfigJson(QVariant configJson);
    void     saveConfig(const QString FileName);
    void     loadConfig(const QString FileName);

    void     setKeyValue(QString Key, const QVariant val);
    const QVariant getKeyValue(QString Key);

    int      countLogicalChannels() const;

    bool     isNegativeHardwareChannel(int iHardwChannel) const;
    bool     isNegativeLogicalChannel(int iLogicalChannel) const;

    bool     isIgnoredHardwareChannel(int iHardwChannel) const;
    bool     isIgnoredLogicalChannel(int iLogicalChannel) const;

    int      toHardware(int iLogicalChannel) const;
    int      toLogical(int iHardwChannel) const;

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
