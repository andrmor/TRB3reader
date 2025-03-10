#ifndef ACONFIG_SI_H
#define ACONFIG_SI_H

#include "ascriptinterface.h"

#include <vector>

#include <QObject>
#include <QString>
#include <QVariant>

class QJsonObject;
class QJsonValue;
class AScriptHub;
class MasterConfig;

class AConfig_SI : public AScriptInterface
{
    Q_OBJECT

public:
    AConfig_SI();

    AScriptInterface * cloneBase() const {return new AConfig_SI();}

public slots:
    bool        load(QString FileName);
    bool        save(QString FileName);

    QVariantMap getConfig();
    bool        setConfig(QVariantMap ConfigObject);

    bool        replace(QString Key, QVariant Value);
    QVariant    getKeyValue(QString Key);

    int         countLogicalChannels() const;

    bool        isNegativeHardwareChannel(int iHardwChannel);
    bool        isNegativeLogicalChannel(int iLogicalChannel);

    bool        isIgnoredHardwareChannel(int iHardwChannel);
    bool        isIgnoredLogicalChannel(int iLogicalChannel);

    int         toHardware(int iLogicalChannel);
    int         toLogical(int iHardwChannel);

private:
    AScriptHub   & ScriptHub;
    MasterConfig & Config;
    QString        LastError;

    bool modifyJsonValue(QJsonObject & obj, const QString & path, const QJsonValue & newValue);
    void find(const QJsonObject & obj, QStringList Keys, QStringList & Found, QString Path = "");
    bool keyToNameAndIndex(QString Key, QString & Name, std::vector<int> & Indexes);
    bool expandKey(QString & Key);

};

#endif // ACONFIG_SI_H
