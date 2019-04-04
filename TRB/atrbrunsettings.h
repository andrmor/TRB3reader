#ifndef ATRBRUNSETTINGS_H
#define ATRBRUNSETTINGS_H

#include <QString>

class QJsonObject;

class ATrbRunSettings
{
public:
    QString User;
    QString Host;

    QString StartupScriptOnHost;
    QString AcquireScriptOnHost;
    QString StorageXMLOnHost;

    QString HldDirOnHost;
    int     MaxHldSizeMb = 20;

    bool    bLimitTime = false;
    double  TimeLimit  = 10.0;
    int     TimeMultiplier = 1;
    bool    bLimitEvents = false;
    int     MaxEvents  = 100;

    const QJsonObject   WriteToJson() const;
    void                ReadFromJson(const QJsonObject &json);
};

#endif // ATRBRUNSETTINGS_H
