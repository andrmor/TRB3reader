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

    json["CtsControl"] = CtsControl;

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

    parseJson(json, "CtsControl", CtsControl);
}
