#ifndef MASTERCONFIG_H
#define MASTERCONFIG_H

#include <string>
#include <vector>
#include <QSet>
#include <QList>
#include <QJsonObject>

class QJsonObject;

class MasterConfig
{
public:
    MasterConfig();

    std::vector<int> NegativeChannels;
    std::vector<std::size_t> ChannelMap;
    QSet<int> IgnoreHardwareChannels;

    bool bSmoothWaveforms = false;
        bool AdjacentAveraging_bOn = false;
        int AdjacentAveraging_NumPoints = 1;
        bool AdjacentAveraging_bWeighted = false;

    bool bPedestalSubstraction = false;
    bool bSmoothingBeforePedestals = false;
        int PedestalFrom = 0;
        int PedestalTo = 0;

    int SignalExtractionMethod = 0; //0 - independent max, 1 - common sample, at global max
    int CommonSampleNumber = 0;

    bool bZeroSignalIfReverse = false;
    double ReverseMaxThreshold = 0.25;

    bool bPositiveThreshold = false;
    double PositiveThreshold = 0;

    bool bNegativeThreshold = false;
    double NegativeThreshold = 0;

    bool bPositiveIgnore = false;
    double PositiveIgnore = 1.0e10;

    bool bNegativeIgnore = false;
    double NegativeIgnore = 1.0e10;

    bool bNegMaxGate = false;
    int  NegMaxGateFrom = 0;
    int  NegMaxGateTo = 1000;

    bool bPosMaxGate = false;
    int  PosMaxGateFrom = 0;
    int  PosMaxGateTo = 1000;


    std::string filename;

    QString GlobScript;
    QJsonObject ScriptWindowJson;
    int DefaultFontSize_ScriptWindow = 10;
    QString DefaultFontFamily_ScriptWindow;
    bool DefaultFontWeight_ScriptWindow;
    bool DefaultFontItalic_ScriptWindow;
    QList<int> MainSplitterSizes_ScriptWindow;


    void WriteToJson(QJsonObject& json);
    bool ReadFromJson(QJsonObject& json);

private:
    void writeSignalPolarityToJson(QJsonObject& json);
    bool readSignalPolarityFromJson(QJsonObject& json);

    void writeChannelMapToJson(QJsonObject& json);
    bool readChannelMapFromJson(QJsonObject& json);

    void writeIgnoreChannelsToJson(QJsonObject& json);
    bool readIgnoreChannelsFromJson(QJsonObject& json);

    void writePedestalsToJson(QJsonObject& json);
    bool readPedestalsFromJson(QJsonObject& json);

    void writeSmoothingToJson(QJsonObject& json);
    bool readSmoothingFromJson(QJsonObject& json);

    void writeSignalExtractionToJson(QJsonObject& json);
    bool readSignalExtractionFromJson(QJsonObject& json);

    void writeMaxGateToJson(QJsonObject& json);
    bool readMaxGateFromJson(QJsonObject& json);

    void writeScriptSettingsToJson(QJsonObject& json);
    bool readScriptSettingsFromJson(QJsonObject& json);
};

#endif // MASTERCONFIG_H
