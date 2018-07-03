#ifndef TRB3SIGNALEXTRACTOR_H
#define TRB3SIGNALEXTRACTOR_H

#include <QVector>

class Trb3dataReader;
class MasterConfig;

class Trb3signalExtractor
{
public:
    Trb3signalExtractor(const MasterConfig *Config, const Trb3dataReader* Reader);

    bool    ExtractSignals();
    void    GenerateDummyData(); // called from bulk processor if extraction is disabled

    float   GetSignal(int ievent, int ichannel) const; // slow but safe
    float   GetSignalFast(int ievent, int ichannel) const; // no argument validity check!

    const   QVector <float>* GetSignals(int ievent) const; // safe
    const   QVector <float>* GetSignalsFast(int ievent) const; // no argument validity check!

    bool    SetSignal(int ievent, int ichannel, float value);
    void    SetSignalFast(int ievent, int ichannel, float value); // no argument validity check!

    bool    SetSignals(int ievent, const QVector <float>& values);
    void    SetSignalsFast(int ievent, const QVector <float>& values); // no argument validity check! no size check!

    bool    SetRejected(int ievent, bool flag);
    void    SetAllRejected(bool flag);

    void    ClearData();

    int     CountEvents() const {return signalData.size();}
    int     CountChannels() const;

    bool    IsRejectedEvent(int ievent) const;
    bool    IsRejectedEventFast(int ievent) const {return RejectedEvents.at(ievent);}

    float   extractSignalFromWaveform(int ievent, int ichannel, bool *Rejected = 0);

private:
    const   MasterConfig* Config;
    const   Trb3dataReader* Reader;
    QVector < QVector <float> > signalData;  // format:  [ievent] [ichanel]            this is (peak - pedestal)
    QVector<bool> RejectedEvents;

    int     numChannels;

    void    ExtractAllSignals(); // extact signals = peak - pedestal

    float  extractMax(const QVector<float> *arr);
    float  extractMin(const QVector<float> *arr);

    int     iNegMaxSample, iPosMaxSample;
    int     iMax, iMin;
    float  NegMaxValue, PosMaxValue;
};

class Trb3ExtractionMonitor
{
public:
    bool EventRejected;

    int AmplitudeNegPosRejections;
};

#endif // TRB3SIGNALEXTRACTOR_H
