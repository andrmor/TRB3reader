#include "atrbrunsettings.h"

#include "ajsontools.h"

const QString ATrbRunSettings::getScriptDir() const
{
    QString dir = ScriptDirOnHost;
    if (!dir.endsWith('/')) dir += '/';
    return dir;
}

const QJsonObject ATrbRunSettings::WriteToJson() const
{
    QJsonObject json;
    json["User"] = User;
    json["Host"] = Host;

    json["ScriptDirOnHost"] = ScriptDirOnHost;
    json["StartupScriptOnHost"] = StartupScriptOnHost;
    json["StorageXMLOnHost"] = StorageXML;

    json["HldDirOnHost"] = HldDirOnHost;
    json["MaxHldSizeMb"] = MaxHldSizeMb;

    json["bLimitTime"] = bLimitTime;
    json["TimeLimit"] = TimeLimit;
    json["TimeMultiplier"] = TimeMultiplier;
    json["bLimitEvents"] = bLimitEvents;
    json["MaxEvents"] = MaxEvents;

    QJsonObject cj;
        cj["MP_0"] = bMP_0;
        cj["MP_1"] = bMP_1;
        cj["MP_2"] = bMP_2;
        cj["MP_3"] = bMP_3;
        cj["MP_4"] = bMP_4;
        cj["MP_5"] = bMP_5;
        cj["MP_6"] = bMP_6;
        cj["MP_7"] = bMP_7;

        cj["RandPulser"] = bRandPulser;
        cj["PeriodicPulser0"] = bPeriodicPulser0;
        cj["PeriodicPulser0"] = bPeriodicPulser0;
    json["CtsControl"] = cj;

    return json;
}

void ATrbRunSettings::ReadFromJson(const QJsonObject &json)
{
    parseJson(json, "User", User);
    parseJson(json, "Host", Host);

    parseJson(json, "ScriptDirOnHost", ScriptDirOnHost);
    parseJson(json, "StartupScriptOnHost", StartupScriptOnHost);
    parseJson(json, "StorageXMLOnHost", StorageXML);

    parseJson(json, "HldDirOnHost", HldDirOnHost);
    parseJson(json, "MaxHldSizeMb", MaxHldSizeMb);

    parseJson(json, "bLimitTime", bLimitTime);
    parseJson(json, "TimeLimit", TimeLimit);
    parseJson(json, "TimeMultiplier", TimeMultiplier);
    parseJson(json, "bLimitEvents", bLimitEvents);
    parseJson(json, "MaxEvents", MaxEvents);

    QJsonObject cj;
    parseJson(json, "CtsControl", cj);
        parseJson(cj, "MP_0", bMP_0);
        parseJson(cj, "MP_1", bMP_1);
        parseJson(cj, "MP_2", bMP_2);
        parseJson(cj, "MP_3", bMP_3);
        parseJson(cj, "MP_4", bMP_4);
        parseJson(cj, "MP_5", bMP_5);
        parseJson(cj, "MP_6", bMP_6);
        parseJson(cj, "MP_7", bMP_7);
        parseJson(cj, "RandPulser", bRandPulser);
        parseJson(cj, "PeriodicPulser0", bPeriodicPulser0);
        parseJson(cj, "PeriodicPulser0", bPeriodicPulser0);
    json["CtsControl"] = cj;
}
