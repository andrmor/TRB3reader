#include "atrbrunsettings.h"

#include "ajsontools.h"

#include <QDebug>

const QString ATrbRunSettings::getScriptDir() const
{
    QString dir = ScriptDirOnHost;
    if (!dir.endsWith('/')) dir += '/';
    return dir;
}

ulong ATrbRunSettings::getTriggerInt() const
{
    ulong r = 1    * bPeriodicPulser +  // 0
              2    * bRandPulser +      // 1
              4    * bMP_0 +            // 2
              8    * bMP_1 +            // 3
              16   * bMP_2 +            // 4
              32   * bMP_3 +            // 5
              64   * bMP_4 +            // 6
              128  * bMP_5 +            // 7
              256  * bMP_6 +            // 8
              512  * bMP_7 +            // 9
              1024 * bPeripheryFPGA0 +  // 10
              2048 * bPeripheryFPGA1 ;  // 11

    r += (ulong)Mask * 0x10000;

    return r;
}

bool CheckBit(ulong val, int pos)
{
    return ((val) & (1<<(pos)));
}
void ATrbRunSettings::setTriggerInt(ulong val)
{
    Mask = val / 0x10000;
    qDebug() << "mask-->" << QString::number(Mask, 16);

    bPeriodicPulser = CheckBit(val, 0);
    bRandPulser     = CheckBit(val, 1);
    bMP_0           = CheckBit(val, 2);
    bMP_1           = CheckBit(val, 3);
    bMP_2           = CheckBit(val, 4);
    bMP_3           = CheckBit(val, 5);
    bMP_4           = CheckBit(val, 6);
    bMP_5           = CheckBit(val, 7);
    bMP_6           = CheckBit(val, 8);
    bMP_7           = CheckBit(val, 9);
    bPeripheryFPGA0 = CheckBit(val, 10);
    bPeripheryFPGA1 = CheckBit(val, 11);
}

QJsonObject ATrbRunSettings::WriteToJson() const
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
        cj["PeriodicPulser"] = bPeriodicPulser;

        cj["bPeripheryFPGA0"] = bPeripheryFPGA0;
        cj["bPeripheryFPGA1"] = bPeripheryFPGA1;

        cj["OR_0_FPGA3"] = OR_0_FPGA3;
        cj["OR_1_FPGA3"] = OR_1_FPGA3;
        cj["OR_0_FPGA4"] = OR_0_FPGA4;
        cj["OR_1_FPGA4"] = OR_1_FPGA4;

        cj["TimeEnable_FPGA3"] = TimeEnable_FPGA3;
        cj["TimeEnable_FPGA4"] = TimeEnable_FPGA4;
        cj["TimeChannels_FPGA3"] = TimeChannels_FPGA3;
        cj["TimeChannels_FPGA4"] = TimeChannels_FPGA4;
        cj["TimeWinBefore_FPGA3"] = TimeWinBefore_FPGA3;
        cj["TimeWinAfter_FPGA3"]  = TimeWinAfter_FPGA3;
        cj["TimeWinBefore_FPGA4"] = TimeWinBefore_FPGA4;
        cj["TimeWinAfter_FPGA4"]  = TimeWinAfter_FPGA4;

        cj["Mask"] = Mask;
        cj["RandomPulserFrequency"] = RandomPulserFrequency;
        cj["Period"] = Period;

        cj["PeripheryTriggerInputs0"] = PeripheryTriggerInputs0;
        cj["PeripheryTriggerInputs1"] = PeripheryTriggerInputs1;

        QJsonArray ar;
        for (const QString & s : TheRestCTScontrols) ar << s;
        cj["TheRestControls"] = ar;

    json["CtsControl"] = cj;

    // Trigger gains
    {
        QJsonObject js;
            js["Enabled"] = bTriggerGains;
            js["DefaultTriggerGain"] = DefaultTriggerGain;
                QJsonArray ar;
                for (int g : TriggerGains)
                    ar.push_back(g);
            js["Gains"] = ar;
        json["TriggerGains"] = js;
    }

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
        parseJson(cj, "PeriodicPulser", bPeriodicPulser);

        parseJson(cj, "bPeripheryFPGA0", bPeripheryFPGA0);
        parseJson(cj, "bPeripheryFPGA1", bPeripheryFPGA1);

        parseJson(cj, "OR_0_FPGA3", OR_0_FPGA3);
        parseJson(cj, "OR_1_FPGA3", OR_1_FPGA3);
        parseJson(cj, "OR_0_FPGA4", OR_0_FPGA4);
        parseJson(cj, "OR_1_FPGA4", OR_1_FPGA4);

        parseJson(cj, "TimeEnable_FPGA3", TimeEnable_FPGA3);
        parseJson(cj, "TimeEnable_FPGA4", TimeEnable_FPGA4);
        parseJson(cj, "TimeChannels_FPGA3", TimeChannels_FPGA3);
        parseJson(cj, "TimeChannels_FPGA4", TimeChannels_FPGA4);
        parseJson(cj, "TimeWinBefore_FPGA3", TimeWinBefore_FPGA3);
        parseJson(cj, "TimeWinAfter_FPGA3",  TimeWinAfter_FPGA3);
        parseJson(cj, "TimeWinBefore_FPGA4", TimeWinBefore_FPGA4);
        parseJson(cj, "TimeWinAfter_FPGA4",  TimeWinAfter_FPGA4);

        parseJson(cj, "Mask", Mask);
        parseJson(cj, "RandomPulserFrequency", RandomPulserFrequency);
        parseJson(cj, "Period0", Period);

        parseJson(cj, "PeripheryTriggerInputs0", PeripheryTriggerInputs0);
        parseJson(cj, "PeripheryTriggerInputs1", PeripheryTriggerInputs1);

        QJsonArray ar;
        parseJson(cj, "TheRestControls", ar);
        TheRestCTScontrols.clear();
        for (int i=0; i<ar.size(); i++) TheRestCTScontrols << ar[i].toString();

    // Trigger gains
    {
        TriggerGains.clear();
        bTriggerGains = false;
        DefaultTriggerGain = 30;

        QJsonObject js;
        bool ok = parseJson(json, "TriggerGains", js);
        if (ok)
        {
            parseJson(js, "Enabled", bTriggerGains);
            parseJson(js, "DefaultTriggerGain", DefaultTriggerGain);
            QJsonArray ar;
            parseJson(js, "Gains", ar);
            for (int i = 0; i < ar.size(); i++)
                TriggerGains.push_back(ar[i].toInt());
        }
    }
}
