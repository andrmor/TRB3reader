#ifndef TRB3READER_H
#define TRB3READER_H

#include "masterconfig.h"
#include <vector>
#include <string>

class Trb3dataReader
{
public:
    Trb3dataReader();

    void UpdateConfig(MasterConfig* config) {Config = *config;}

      // Reading waveform data from the file, optional - substract pedestals and apply smoothing
    bool Read();

    int  GetValue(int ievent, int ichannel, int isample) const;             //no argument validity check!
    const std::vector<int>* GetWaveformPtr(int ievent, int ichannel) const; //no argument validity check!

    // processing successful?
    bool isValid() const {return (waveData.size()>0 && numChannels>0 && numSamples>0);}

    // parameter requests
    int  GetNumSamples()   const {return numSamples;}      // number of samples in the waveform
    int  GetNumChannels()  const {return numChannels;}     // number of channels per event
    int  GetNumEvents()    const {return waveData.size();} // number of events in the datafile
    int  GetNumBadEvents() const {return numBadEvents;}    // number of disreguarded events - they had wrong number of samples

    void ClearData();

private:
    MasterConfig Config;
    std::vector < std::vector < std::vector <int> > > waveData;  // format:  [event] [hardware chanel] [sample]

    int numSamples;
    int numChannels;    
    int numBadEvents;

    void readRawData();    // read raw data from the hld file
    void smoothData();     // smooth raw data

    void doAdjacentAverage(std::vector<int> &arr, int numPoints);
    void doAdjacentWeightedAverage(std::vector<int> &arr, int numPoints);

    void substractPedestals();

};

#endif // TRB3READER_H
