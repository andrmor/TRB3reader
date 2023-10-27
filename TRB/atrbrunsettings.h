#ifndef ATRBRUNSETTINGS_H
#define ATRBRUNSETTINGS_H

#include <QString>
#include <QStringList>

#include <vector>

class QJsonObject;

class ATrbRunSettings
{
public:
    QString User;
    QString Host;

    QString ScriptDirOnHost = "/home/rpcuser/trbsoft/userscripts/trb130";
    QString StartupScriptOnHost = "startup_Andr.sh";
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
    bool    bPeriodicPulser = false;
    bool    bPeripheryFPGA0 = false;
    bool    bPeripheryFPGA1 = false;

    QString PeripheryTriggerInputs0 = "0x00000000";
    QString PeripheryTriggerInputs1 = "0x00000000";

    int     Mask = 0xffff;
    QString RandomPulserFrequency = "0"; //QJsonObject is not friendly for ulong...
    QString Period = "0x30000000";
    QStringList TheRestCTScontrols;

    ulong   getTriggerInt() const;
    void    setTriggerInt(ulong val);

    const QJsonObject   WriteToJson() const;
    void                ReadFromJson(const QJsonObject &json);
};

#endif // ATRBRUNSETTINGS_H
