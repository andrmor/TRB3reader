#include "masterconfig.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

MasterConfig::MasterConfig() {}

void MasterConfig::WriteToJson(QJsonObject &json)
{
    writeSignalPolarityToJson(json);
    writeChannelMapToJson(json);
    writePedestalsToJson(json);
    writeSmoothingToJson(json);
    writeSignalSuppressionToJson(json);
}

bool MasterConfig::ReadFromJson(QJsonObject &json)
{
    readSignalPolarityFromJson(json);
    readChannelMapFromJson(json);
    readPedestalsToJson(json);
    readSmoothingFromJson(json);
    readSignalSuppressionFromJson(json);

    return true;
}

void MasterConfig::writeSmoothingToJson(QJsonObject &json)
{
    QJsonObject js;

    js["Activated"] = bSmoothWaveforms;
    js["SmoothingBeforePedestals"] = bSmoothingBeforePedestals;

    QJsonObject aa_js;
        aa_js["On"] = AdjacentAveraging_bOn;
        aa_js["Weighted"] = AdjacentAveraging_bWeighted;
        aa_js["Points"] =  AdjacentAveraging_NumPoints;
    js["AdjacentAveraging"] = aa_js;

    json["Smoothing"] = js;
}

bool MasterConfig::readSmoothingFromJson(QJsonObject &json)
{
    if (!json.contains("Smoothing")) return false;

    QJsonObject js = json["Smoothing"].toObject();

    bSmoothWaveforms = js["Activated"].toBool();
    bSmoothingBeforePedestals = js["SmoothingBeforePedestals"].toBool();

    QJsonObject aa_js = js["AdjacentAveraging"].toObject();

    AdjacentAveraging_bOn =  aa_js["On"].toBool();
    AdjacentAveraging_bWeighted  =  aa_js["Weighted"].toBool();
    AdjacentAveraging_NumPoints = aa_js["Points"].toInt();
    return true;
}

void MasterConfig::writeSignalPolarityToJson(QJsonObject &json)
{
    QJsonArray arr;
    for (int i: NegativeChannels) arr << i;
    json["NegativeChannels"] = arr;
}

bool MasterConfig::readSignalPolarityFromJson(QJsonObject &json)
{
    if (json.contains("SignalExtraction"))
    {
        QJsonObject js = json["SignalExtraction"].toObject();

        NegativeChannels.clear();
        QJsonArray arr = js["NegativeChannels"].toArray();
        for (int i=0; i<arr.size(); i++)
            NegativeChannels.push_back(arr[i].toInt());
        PedestalFrom = js["PedestalsFrom"].toInt();
        PedestalTo = js["PedestalsTo"].toInt();
        return true;
    }

    if (!json.contains("NegativeChannels")) return false;

    NegativeChannels.clear();
    QJsonArray arr = json["NegativeChannels"].toArray();
    for (int i=0; i<arr.size(); i++)
        NegativeChannels.push_back(arr[i].toInt());

    return true;
}

void MasterConfig::writeChannelMapToJson(QJsonObject &json)
{
    QJsonArray arr;
    for (int i: ChannelMap) arr << i;
    json["ChannelMap"] = arr;
}

bool MasterConfig::readChannelMapFromJson(QJsonObject &json)
{
    if (!json.contains("ChannelMap")) return false;

    ChannelMap.clear();
    QJsonArray arr = json["ChannelMap"].toArray();
    for (int i=0; i<arr.size(); i++)
        ChannelMap.push_back(arr[i].toInt());

    return true;
}

void MasterConfig::writePedestalsToJson(QJsonObject &json)
{
    QJsonObject js;

    js["SubstractPedestals"] = bPedestalSubstraction;
    js["PedestalsFrom"] = PedestalFrom;
    js["PedestalsTo"] = PedestalTo;

    json["Pedestals"] = js;
}

bool MasterConfig::readPedestalsToJson(QJsonObject &json)
{
    if (!json.contains("Pedestals")) return false;

    QJsonObject js = json["Pedestals"].toObject();

    bPedestalSubstraction = js["SubstractPedestals"].toBool();
    PedestalFrom = js["PedestalsFrom"].toInt();
    PedestalTo = js["PedestalsTo"].toInt();

    return true;
}

void MasterConfig::writeSignalSuppressionToJson(QJsonObject &json)
{
    QJsonObject js;

    QJsonObject rm_js;
        rm_js["On"] = bZeroSignalIfReverse;
        rm_js["Threshold"] = ReverseMaxThreshold;
    js["ReverseMax"] = rm_js;

    json["SignalSuppression"] = js;
}

bool MasterConfig::readSignalSuppressionFromJson(QJsonObject &json)
{
    if (!json.contains("SignalSuppression")) return false;

    QJsonObject js = json["SignalSuppression"].toObject();

    QJsonObject rm_js = js["ReverseMax"].toObject();
        bZeroSignalIfReverse = rm_js["On"].toBool();
        ReverseMaxThreshold = rm_js["Threshold"].toDouble();

    return true;
}
