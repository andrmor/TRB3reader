#include "masterconfig.h"
#include "ajsontools.h"
#include "channelmapper.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

MasterConfig::MasterConfig()
{
    Map = new ChannelMapper();
}

MasterConfig::~MasterConfig()
{
    delete Map;
}

void MasterConfig::SetNegativeChannels(const std::vector<int> &listOfChannels)
{
    ListNegativeChannels = listOfChannels;
    updatePolarityQuickAccessData();
}

void MasterConfig::WriteToJson(QJsonObject &json)
{    
    writeSignalPolarityToJson(json);
    writeChannelMapToJson(json);
    writeIgnoreChannelsToJson(json);
    writePedestalsToJson(json);
    writeSmoothingToJson(json);
    writeSignalExtractionToJson(json);
    writeMaxGateToJson(json);
    writeScriptSettingsToJson(json);

    json["FileName"] = FileName;
}

bool MasterConfig::ReadFromJson(QJsonObject &json)
{
    readSignalPolarityFromJson(json);
    readChannelMapFromJson(json);
    readIgnoreChannelsFromJson(json);
    readPedestalsFromJson(json);
    readSmoothingFromJson(json);
    readSignalExtractionFromJson(json);
    readMaxGateFromJson(json);
    readScriptSettingsFromJson(json);

    parseJson(json, "FileName", FileName);

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
    for (int i: ListNegativeChannels) arr << i;
    json["NegativeChannels"] = arr;
}

bool MasterConfig::readSignalPolarityFromJson(QJsonObject &json)
{    
    if (!json.contains("NegativeChannels")) return false;

    ListNegativeChannels.clear();
    QJsonArray arr = json["NegativeChannels"].toArray();
    for (int i=0; i<arr.size(); i++)
        ListNegativeChannels.push_back(arr[i].toInt());

    updatePolarityQuickAccessData();
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

    Map->Clear();
    Map->SetChannels_OrderedByLogical(ChannelMap);

    return true;
}

void MasterConfig::writeIgnoreChannelsToJson(QJsonObject &json)
{
    QJsonArray arr;
    for (int i: IgnoreHardwareChannels) arr << i;
    json["IgnoreHardwareChannels"] = arr;
}

bool MasterConfig::readIgnoreChannelsFromJson(QJsonObject &json)
{
    if (!json.contains("IgnoreHardwareChannels")) return false;

    IgnoreHardwareChannels.clear();
    QJsonArray arr = json["IgnoreHardwareChannels"].toArray();
    for (int i=0; i<arr.size(); i++)
        IgnoreHardwareChannels.insert(arr[i].toInt());

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

bool MasterConfig::readPedestalsFromJson(QJsonObject &json)
{
    if (!json.contains("Pedestals")) return false;

    QJsonObject js = json["Pedestals"].toObject();

    bPedestalSubstraction = js["SubstractPedestals"].toBool();
    PedestalFrom = js["PedestalsFrom"].toInt();
    PedestalTo = js["PedestalsTo"].toInt();

    return true;
}

void MasterConfig::writeSignalExtractionToJson(QJsonObject &json)
{
    QJsonObject js;
        js["ExtractionMethod"] = SignalExtractionMethod;
        js["CommonSample"] = CommonSampleNumber;

        QJsonObject rm_js;
            rm_js["On"] = bZeroSignalIfReverse;
            rm_js["Threshold"] = ReverseMaxThreshold;
        js["ReverseMax"] = rm_js;

        js["ApplyPositiveThreshold"] = bPositiveThreshold;
        js["PositiveThreshold"] = PositiveThreshold;

        js["ApplyNegativeThreshold"] = bNegativeThreshold;
        js["NegativeThreshold"] = NegativeThreshold;

        js["ApplyPositiveIgnore"] = bPositiveIgnore;
        js["PositiveIgnore"] = PositiveIgnore;

        js["ApplyNegativeIgnore"] = bNegativeIgnore;
        js["NegativeIgnore"] = NegativeIgnore;
    json["SignalExtraction"] = js;
}

bool MasterConfig::readSignalExtractionFromJson(QJsonObject &json)
{    
    QJsonObject js;

    if (!parseJson(json, "SignalExtraction", js)) parseJson(json, "SignalSuppression", js);

    QJsonObject rm_js = js["ReverseMax"].toObject();
        bZeroSignalIfReverse = rm_js["On"].toBool();
        ReverseMaxThreshold = rm_js["Threshold"].toDouble();

    parseJson(js, "ExtractionMethod", SignalExtractionMethod);
    parseJson(js, "CommonSample", CommonSampleNumber);

    parseJson(js, "ApplyPositiveThreshold", bPositiveThreshold);
    parseJson(js, "PositiveThreshold", PositiveThreshold);

    parseJson(js, "ApplyNegativeThreshold", bNegativeThreshold);
    parseJson(js, "NegativeThreshold", NegativeThreshold);

    parseJson(js, "ApplyPositiveIgnore", bPositiveIgnore);
    parseJson(js, "PositiveIgnore", PositiveIgnore);

    parseJson(js, "ApplyNegativeIgnore", bNegativeIgnore);
    parseJson(js, "NegativeIgnore", NegativeIgnore);

    return true;
}

void MasterConfig::writeMaxGateToJson(QJsonObject &json)
{
    QJsonObject js;

    js["ApplyNegative"] = bNegMaxGate;
    js["NegativeMaxGateFrom"] = NegMaxGateFrom;
    js["NegativeMaxGateTo"]   = NegMaxGateTo;
    js["ApplyPositive"] = bPosMaxGate;
    js["PositiveMaxGateFrom"] = PosMaxGateFrom;
    js["PositiveMaxGateTo"]   = PosMaxGateTo;

    json["MaxGate"] = js;
}

bool MasterConfig::readMaxGateFromJson(QJsonObject &json)
{
    if (!json.contains("MaxGate")) return false;

    QJsonObject js = json["MaxGate"].toObject();
    parseJson(js, "ApplyNegative",       bNegMaxGate);
    parseJson(js, "NegativeMaxGateFrom", NegMaxGateFrom);
    parseJson(js, "NegativeMaxGateTo",   NegMaxGateTo);
    parseJson(js, "ApplyPositive",       bPosMaxGate);
    parseJson(js, "PositiveMaxGateFrom", PosMaxGateFrom);
    parseJson(js, "PositiveMaxGateTo",   PosMaxGateTo);

    return true;
}

void MasterConfig::writeScriptSettingsToJson(QJsonObject &json)
{
    QJsonObject js;

    js["GlobScript"] = GlobScript;
    js["ScriptWindowJson"] = ScriptWindowJson;
    js["DefaultFontSize_ScriptWindow"] = DefaultFontSize_ScriptWindow;
    js["DefaultFontFamily_ScriptWindow"] = DefaultFontFamily_ScriptWindow;
    js["DefaultFontWeight_ScriptWindow"] = DefaultFontWeight_ScriptWindow;
    js["DefaultFontItalic_ScriptWindow"] = DefaultFontItalic_ScriptWindow;
    QJsonArray mspAr;
    for (int w : MainSplitterSizes_ScriptWindow) mspAr << w;
    js["MainSplitterSizes_ScriptWindow"] = mspAr;

    json["ScriptWinSettings"] = js;
}

bool MasterConfig::readScriptSettingsFromJson(QJsonObject &json)
{
    QJsonObject js;
    parseJson(json, "ScriptWinSettings", js);

    if (!js.isEmpty())
    {
        parseJson(js, "DefaultFontSize_ScriptWindow", DefaultFontSize_ScriptWindow);
        parseJson(js, "DefaultFontFamily_ScriptWindow", DefaultFontFamily_ScriptWindow);
        parseJson(js, "DefaultFontWeight_ScriptWindow", DefaultFontWeight_ScriptWindow);
        parseJson(js, "DefaultFontItalic_ScriptWindow", DefaultFontItalic_ScriptWindow);
        QJsonArray mspAr;
        parseJson(js, "MainSplitterSizes_ScriptWindow", mspAr);
        for (int imsa=0; imsa<mspAr.size(); imsa++)
            MainSplitterSizes_ScriptWindow << mspAr[imsa].toInt(50);
    }
}

void MasterConfig::updatePolarityQuickAccessData()
{
    for (int ichannel : ListNegativeChannels)
    {
        if (ichannel >= NegPol.size())
            NegPol.resize(ichannel+1, false);
        NegPol[ichannel] = true;
    }
}

bool MasterConfig::IsNegative(int ichannel) const
{
    if (ichannel>=NegPol.size() || ichannel<0) return false;
    return NegPol.at(ichannel);
}

void MasterConfig::SetMapping(const std::vector<size_t> &mapping)
{
    Map->Clear();
    ChannelMap.clear();

    ChannelMap = mapping;
    Map->SetChannels_OrderedByLogical(ChannelMap);
}
