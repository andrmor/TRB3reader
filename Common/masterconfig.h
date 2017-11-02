#ifndef MASTERCONFIG_H
#define MASTERCONFIG_H

#include <string>
#include <vector>

class QJsonObject;

class MasterConfig
{
public:
    MasterConfig();

    std::vector<int> NegativeChannels;
    std::vector<std::size_t> ChannelMap;

    bool bSmoothWaveforms = false;
        bool AdjacentAveraging_bOn = false;
        int AdjacentAveraging_NumPoints = 1;
        bool AdjacentAveraging_bWeighted = false;

    bool bPedestalSubstraction = false;
    bool bSmoothingBeforePedestals = false;
        int PedestalFrom = 0;
        int PedestalTo = 0;

    bool bZeroSignalIfReverse = false;
    double ReverseMaxThreshold = 0.25;

    std::string filename = "/home/andr/QtProjects/run191/te17081105154.hld";


    void WriteToJson(QJsonObject& json);
    bool ReadFromJson(QJsonObject& json);

private:
    void writeSignalPolarityToJson(QJsonObject& json);
    bool readSignalPolarityFromJson(QJsonObject& json);

    void writeChannelMapToJson(QJsonObject& json);
    bool readChannelMapFromJson(QJsonObject& json);

    void writePedestalsToJson(QJsonObject& json);
    bool readPedestalsToJson(QJsonObject& json);

    void writeSmoothingToJson(QJsonObject& json);
    bool readSmoothingFromJson(QJsonObject& json);

    void writeSignalSuppressionToJson(QJsonObject& json);
    bool readSignalSuppressionFromJson(QJsonObject& json);
};

#endif // MASTERCONFIG_H
