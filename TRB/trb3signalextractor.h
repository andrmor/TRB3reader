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

    void ClearData();

    std::size_t GetNumEvents() const;
    std::size_t GetNumChannels() const;
    bool IsNegative(std::size_t channel) const;

    double extractSignal_SingleChannel(int ievent, int ichannel, bool *Rejected = 0) const;

private:
    const Trb3dataReader* reader;
    MasterConfig Config;
    std::vector < std::vector <double> > signalData;  // format:  [ievent] [ichanel]            this is (peak - pedestal)
    std::vector<bool> NegPolChannels;

    int numChannels;

    void setPolarity(std::vector<int> negativeChannels);

    void extractSignals_AllEvents(); // extact signals = peak - pedestal

    double extractMax(const std::vector<int>* arr) const;
    double extractMin(const std::vector<int>* arr) const;
};

class Trb3ExtractionMonitor
{
public:
    bool EventRejected;

    int AmplitudeNegPosRejections;
};

#endif // TRB3SIGNALEXTRACTOR_H
