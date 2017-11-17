#ifndef TRB3SIGNALEXTRACTOR_H
#define TRB3SIGNALEXTRACTOR_H

#include "masterconfig.h"
#include <vector>

class Trb3dataReader;

class Trb3signalExtractor
{
public:
    Trb3signalExtractor(const Trb3dataReader* reader);

    void UpdateConfig(const MasterConfig *config);

    bool ExtractSignals();

    double GetSignal(int ievent, int ichannel) const; // slow but safe
    double GetSignalFast(int ievent, int ichannel) const; // no argument validity check!

    const std::vector <double>* GetSignals(int ievent) const; // safe
    const std::vector <double>* GetSignalsFast(int ievent) const; // no argument validity check!

    bool SetSignal(int ievent, int ichannel, double value);
    void SetSignalFast(int ievent, int ichannel, double value); // no argument validity check!

    bool SetSignals(int ievent, const std::vector <double>& values);
    void SetSignalsFast(int ievent, const std::vector <double>& values); // no argument validity check! no size check!

    bool SetRejected(int ievent, bool flag);
    void SetAllRejected(bool flag);

    void ClearData();

    std::size_t GetNumEvents() const;
    std::size_t GetNumChannels() const;

    bool IsNegative(std::size_t channel) const;
    bool IsRejectedEvent(int ievent) const;
    bool IsRejectedEventFast(int ievent) const {return RejectedEvents.at(ievent);}

    double extractSignalFromWaveform(int ievent, int ichannel, bool *Rejected = 0);

private:
    const Trb3dataReader* reader;
    MasterConfig Config;
    std::vector < std::vector <double> > signalData;  // format:  [ievent] [ichanel]            this is (peak - pedestal)
    std::vector<bool> NegPolChannels;
    std::vector<bool> RejectedEvents;

    int numChannels;

    void setPolarity(std::vector<int> negativeChannels);

    void ExtractAllSignals(); // extact signals = peak - pedestal

    double extractMax(const std::vector<int>* arr);
    double extractMin(const std::vector<int>* arr);

    int iNegMaxSample, iPosMaxSample;
    int iMax, iMin;
    double NegMaxValue, PosMaxValue;
};

class Trb3ExtractionMonitor
{
public:
    bool EventRejected;

    int AmplitudeNegPosRejections;
};

#endif // TRB3SIGNALEXTRACTOR_H
