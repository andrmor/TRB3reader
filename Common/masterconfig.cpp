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

ABufferRecord *MasterConfig::findBufferRecord(int datakind)
{
    for (int i=0; i<DatakindSet.size(); i++)
    {
        if (DatakindSet.at(i).Datakind == datakind)
            return &DatakindSet[i];
    }
    return nullptr;
}

const QVector<int> MasterConfig::GetListOfDatakinds() const
{
    QVector<int> vec;
    for (const int & i : ValidDatakinds)
        vec << i;

    if ( vec.size() > 1 ) std::sort(vec.begin(), vec.end());
    return vec;
}

void MasterConfig::AddDatakind(int datakind)
{
    if (ValidDatakinds.contains(datakind)) return;

    DatakindSet << ABufferRecord(datakind);
    ValidDatakinds << datakind;
}

void MasterConfig::RemoveDatakind(int datakind)
{
    for (int i=0; i<DatakindSet.size(); i++)
        if (DatakindSet.at(i).Datakind == datakind)
            DatakindSet.remove(i);

    ValidDatakinds.remove(datakind);
}

void MasterConfig::SetNegativeChannels(const QVector<int> &list)
{
    QSet<int> set;
    for (auto i : list) set << i;

    QVector<int> vec;
    for (auto i: set) vec << i;
    std::sort(vec.begin(), vec.end());

    ListNegativeChannels = vec;
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

    json["HldProcessSettings"] = HldProcessSettings.WriteToJson();

    json["TrbRunSettings"] = TrbRunSettings.WriteToJson();

    json["FileName"] = FileName;
    json["WorkingDir"] = WorkingDir;

    QJsonArray ar;
    for (const ABufferRecord & r : DatakindSet)
        ar << r.toJson();
    json["DatakindSets"] = ar;
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

    HldProcessSettings.ReadFromJson( json["HldProcessSettings"].toObject() );

    if (json.contains("TrbRunSettings"))
        TrbRunSettings.ReadFromJson( json["TrbRunSettings"].toObject() );

    parseJson(json, "FileName", FileName);
    parseJson(json, "WorkingDir", WorkingDir);

    DatakindSet.clear();
    ValidDatakinds.clear();
    if (json.contains("DatakindSets"))
    {
        QJsonArray ar = json["DatakindSets"].toArray();
        for (int i=0; i<ar.size(); i++)
        {
            QJsonObject js = ar[i].toObject();
            ABufferRecord rec;
            rec.readFromJson(js);
            DatakindSet << rec;
            ValidDatakinds << rec.Datakind;
        }
    }
    else if (json.contains("Datakinds")) //compatibility
    {
        QJsonArray ar = json["Datakinds"].toArray();
        for (int i=0; i<ar.size(); i++)
        {
            int dk = ar[i].toInt();
            DatakindSet << ABufferRecord(dk);
            ValidDatakinds << dk;
        }
    }

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
        ListNegativeChannels << arr[i].toInt();

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
        ChannelMap << arr[i].toInt();

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
    js["Method"] = PedestalExtractionMethod;
    js["PedestalsFrom"] = PedestalFrom;
    js["PedestalsTo"] = PedestalTo;
    js["PeakSigma"] = PedestalPeakSigma;
    js["PeakThreshold"] = PedestalPeakThreshold;

    json["Pedestals"] = js;
}

bool MasterConfig::readPedestalsFromJson(QJsonObject &json)
{
    if (!json.contains("Pedestals")) return false;

    QJsonObject js = json["Pedestals"].toObject();

    bPedestalSubstraction = js["SubstractPedestals"].toBool();
    parseJson(js, "Method", PedestalExtractionMethod);
    PedestalFrom = js["PedestalsFrom"].toInt();
    PedestalTo = js["PedestalsTo"].toInt();
    parseJson(js, "PeakSigma", PedestalPeakSigma);
    parseJson(js, "PeakThreshold", PedestalPeakThreshold);

    return true;
}

void MasterConfig::writeSignalExtractionToJson(QJsonObject &json)
{
    QJsonObject js;
        js["ExtractionMethod"] = SignalExtractionMethod;
        js["CommonSample"] = CommonSampleNumber;
        js["IntegrateFrom"] = IntegrateFrom;
        js["IntegrateTo"] = IntegrateTo;

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
    parseJson(js, "IntegrateFrom", IntegrateFrom);
    parseJson(js, "IntegrateTo", IntegrateTo);

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

void MasterConfig::updatePolarityQuickAccessData()
{
    int imax = 0;
    for (int ichannel : ListNegativeChannels)
        if (ichannel>imax) imax = ichannel;
    NegPol = QVector<bool>(imax+1, false);

    for (int ichannel : ListNegativeChannels)
        NegPol[ichannel] = true;
}

bool MasterConfig::IsNegativeHardwareChannel(int iHardwChannel) const
{
    if ( iHardwChannel >= NegPol.size() || iHardwChannel < 0 ) return false;
    return NegPol.at(iHardwChannel);
}

bool MasterConfig::IsNegativeLogicalChannel(int iLogical) const
{
    int iHardwChannel = Map->LogicalToHardware(iLogical);
    if ( iHardwChannel >= NegPol.size() || iHardwChannel < 0 ) return false;
    return NegPol.at(iHardwChannel);
}

bool MasterConfig::SetMapping(const QVector<int> &mapping)
{
    QSet<int> tmp;
    for (int i : mapping) tmp << i;
    if (tmp.size() != mapping.size()) return false;

    Map->Clear();
    ChannelMap.clear();

    ChannelMap = mapping;
    Map->SetChannels_OrderedByLogical(ChannelMap);
    return true;
}

bool MasterConfig::UpdateNumberOfHardwareChannels(int numHardwChannels)
{
    return Map->UpdateNumberOfHardwareChannels(numHardwChannels);
}

int MasterConfig::CountLogicalChannels() const
{
    return ChannelMap.size();
}

QVector<int> MasterConfig::GetListOfIgnoreChannels() const
{
    QVector<int> vec;
    for (int i : IgnoreHardwareChannels) vec << i;
    std::sort(vec.begin(), vec.end());
    return vec;
}

void MasterConfig::SetListOfIgnoreChannels(const QVector<int> &list)
{
    IgnoreHardwareChannels.clear();
    for (int i : list) IgnoreHardwareChannels << i;
}

bool MasterConfig::IsIgnoredLogicalChannel(int iLogical) const
{
    int iHardwChan = Map->LogicalToHardware(iLogical);
    return IgnoreHardwareChannels.contains(iHardwChan);
}

// -------- hld file processor settings ---------

const QJsonObject AHldProcessSettings::WriteToJson() const
{
    QJsonObject js;

    js["NumChannels"] =      NumChannels;
    js["NumSamples"]  =      NumSamples;
    js["DoExtraction"] =     bDoSignalExtraction;
    js["DoScript"] =         bDoScript;
    js["DoSave"] =           bDoSave;
    js["AddToFileName"] =    AddToFileName;
    js["DoCopyToDatahub"] =  bDoCopyToDatahub;
    js["IncludeWaveforms"] = bCopyWaveforms;

    return js;
}

void AHldProcessSettings::ReadFromJson(const QJsonObject &json)
{
    parseJson(json, "NumChannels",      NumChannels);
    parseJson(json, "NumSamples",       NumSamples);
    parseJson(json, "DoExtraction",     bDoSignalExtraction);
    parseJson(json, "DoScript",         bDoScript);
    parseJson(json, "DoSave",           bDoSave);
    parseJson(json, "AddToFileName",    AddToFileName);
    parseJson(json, "DoCopyToDatahub",  bDoCopyToDatahub);
    parseJson(json, "IncludeWaveforms", bCopyWaveforms);
}

bool ABufferRecord::updateValues(int samples, int delay, int downs)
{
    if (Samples == samples && Delay == delay && Downsampling == downs) return false;

    Samples = samples;
    Delay = delay;
    Downsampling = downs;

    return true;
}

const QJsonObject ABufferRecord::toJson() const
{
    QJsonObject j;
    j["Datakind"] = Datakind;
    j["Samples"] = Samples;
    j["Delay"] = Delay;
    j["Downsampling"] = Downsampling;
    return j;
}

void ABufferRecord::readFromJson(const QJsonObject &json)
{
    parseJson(json, "Datakind", Datakind);
    parseJson(json, "Samples", Samples);
    parseJson(json, "Delay", Delay);
    parseJson(json, "Downsampling", Downsampling);
}
