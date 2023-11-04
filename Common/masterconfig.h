#ifndef MASTERCONFIG_H
#define MASTERCONFIG_H

#include "atrbrunsettings.h"

#include <QVector>
#include <QSet>
#include <QList>
#include <QJsonObject>

class QJsonObject;
class ChannelMapper;

class AHldProcessSettings
{
public:
    int     NumChannels = 0;
    int     NumSamples = 0;
    bool    bDoSignalExtraction = true;
    bool    bDoScript = false;
    bool    bDoSave = true;
    QString AddToFileName = "_proc.dat";
    bool    bDoCopyToDatahub = false;
    bool    bCopyWaveforms = false;

    const QJsonObject   WriteToJson() const;
    void                ReadFromJson(const QJsonObject &json);
};

class ABufferRecord
{
public:
    ABufferRecord(){}
    ABufferRecord(int datakind) : Datakind(datakind) {}

    int Datakind = 0;

    int Samples = 80;
    int Delay = 1023;
    int Downsampling = 10;

    bool updateValues(int samples, int delay, int downs); // return false if the new values are the same as old

    const QJsonObject toJson() const;
    void readFromJson(const QJsonObject & json);
};

class MasterConfig
{
public:
    MasterConfig();
    ~MasterConfig();

    //recognized datakinds
    bool                isBufferRecordsEmpty() const {return DatakindSet.isEmpty();}
    QVector<ABufferRecord> & getBufferRecords() {return DatakindSet;}
    ABufferRecord *     findBufferRecord(int datakind);
    const QVector<int>  GetListOfDatakinds() const;
    bool                isADCboard(int datakind) const {return ValidDatakinds.contains(datakind);}
    bool                isTimerBoard(int datakind) const {return (datakind == 0xa004);}//{return (datakind == 0xc001);}
    void                AddDatakind(int datakind);
    void                RemoveDatakind(int datakind);

    //negative/positive channels
    const QVector<int>& GetListOfNegativeChannels() const {return ListNegativeChannels;}
    void                SetNegativeChannels(const QVector<int> &list);
    bool                IsNegativeHardwareChannel(int iHardwChannel) const;
    bool                IsNegativeLogicalChannel(int iLogical) const;

    //channel map (hardware / logical)
    ChannelMapper*      Map;  //use this class to access convertion methods!
    bool                SetMapping(const QVector<int> &mapping);
    const QVector<int>& GetMapping() const {return ChannelMap;}
    bool                UpdateNumberOfHardwareChannels(int numHardwChannels);
    int                 CountLogicalChannels() const;

    QVector<int>        GetListOfIgnoreChannels() const;
    void                SetListOfIgnoreChannels(const QVector<int>& list);
    void                ClearListOfIgnoreChannels() {IgnoreHardwareChannels.clear();}
    bool                IsIgnoredHardwareChannel(int iHardwChannel) const {return IgnoreHardwareChannels.contains(iHardwChannel); }
    bool                IsIgnoredLogicalChannel(int iLogical) const;

    bool                bSmoothWaveforms = false;
    bool                AdjacentAveraging_bOn = false;
    int                 AdjacentAveraging_NumPoints = 1;
    bool                AdjacentAveraging_bWeighted = false;
    bool                bTrapezoidal = false;
    int                 TrapezoidalL = 3;
    int                 TrapezoidalG = 3;

    bool                bPedestalSubstraction = false;
    bool                bSmoothingBeforePedestals = false;
    int                 PedestalExtractionMethod = 0;
    int                 PedestalFrom = 0;
    int                 PedestalTo = 0;
    double              PedestalPeakSigma = 3;
    double              PedestalPeakThreshold = 0.5;

    int                 SignalExtractionMethod = 0; //0 - independent max, 1 - common sample, at global max
    int                 CommonSampleNumber = 0;
    int                 IntegrateFrom = 0;
    int                 IntegrateTo = 100;

    bool                bZeroSignalIfReverse = false;
    double              ReverseMaxThreshold = 0.25;

    bool                bZeroSignalIfPeakOutside_Positive = false;
    int                 ZeroSignalIfPeakBefore_Positive = 10;
    int                 ZeroSignalIfPeakAfter_Positive = 30;

    bool                bZeroSignalIfPeakOutside_Negative = false;
    int                 ZeroSignalIfPeakBefore_Negative = 10;
    int                 ZeroSignalIfPeakAfter_Negative = 30;

    bool                bPositiveThreshold = false;
    double              PositiveThreshold = 0;

    bool                bNegativeThreshold = false;
    double              NegativeThreshold = 0;

    bool                bPositiveIgnore = false;
    double              PositiveIgnore = 1.0e10;

    bool                bNegativeIgnore = false;
    double              NegativeIgnore = 1.0e10;

    bool                bNegMaxGate = false;
    int                 NegMaxGateFrom = 0;
    int                 NegMaxGateTo = 1000;

    bool                bPosMaxGate = false;
    int                 PosMaxGateFrom = 0;
    int                 PosMaxGateTo = 1000;

    QString             FileName;
    QString             WorkingDir;

    // hld file processor settings
    AHldProcessSettings HldProcessSettings;

    //trb acqusition-related settings
    ATrbRunSettings     TrbRunSettings;

    // config <-> JSON handling
    void                WriteToJson(QJsonObject& json);
    bool                ReadFromJson(QJsonObject& json);

private:
    QVector<ABufferRecord> DatakindSet;
    QSet<int>           ValidDatakinds; // must be synchronized with DatakindSet

    QVector<int>        ListNegativeChannels;
    QVector<bool>       NegPol; //Quick access

    QVector<int>        ChannelMap;

    QSet<int>           IgnoreHardwareChannels;

private:
    void updatePolarityQuickAccessData();

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
};

#endif // MASTERCONFIG_H
