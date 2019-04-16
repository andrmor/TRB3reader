#ifndef ATRBRUNSETTINGS_H
#define ATRBRUNSETTINGS_H

#include <QString>

#include <vector>

class QJsonObject;

class ATrbRunSettings
{
public:
    QString User;
    QString Host;

    QString ScriptDirOnHost = "/home/rpcuser/trbsoft/userscripts/trb130";
    QString StartupScriptOnHost = "startup_Andr";
    const QString getScriptDir() const;
    QString StorageXML = "EventBuilder_Andr.xml";

    QString HldDirOnHost;
    int     MaxHldSizeMb = 20;

    bool    bLimitTime = false;
    double  TimeLimit  = 10.0;
    int     TimeMultiplier = 1;
    bool    bLimitEvents = false;
    int     MaxEvents  = 100;

    bool    bMP_0 = false;
    bool    bMP_1 = false;
    bool    bMP_2 = false;
    bool    bMP_3 = false;
    bool    bMP_4 = false;
    bool    bMP_5 = false;
    bool    bMP_6 = false;
    bool    bMP_7 = false;
    bool    bRandPulser = false;
    bool    bPeriodicPulser0 = false;
    bool    bPeriodicPulser1 = false;

    int     Mask = 0xffff;
    QString RandomPulserFrequency = "0"; //QJsonObject is not friendly for ulong...
    QString Period0 = "0x30000000";
    QString Period1 = "0x30000000";

    ulong   getTriggerInt() const;

    const QJsonObject   WriteToJson() const;
    void                ReadFromJson(const QJsonObject &json);
};

#endif // ATRBRUNSETTINGS_H
