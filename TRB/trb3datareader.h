#ifndef TRB3READER_H
#define TRB3READER_H

#include <QVector>

class MasterConfig;

class Trb3dataReader
{
public:
    Trb3dataReader(const MasterConfig* Config);

      // Reading waveform data from the file, optional - substract pedestals and apply smoothing
    bool    Read();

    double  GetValue(int ievent, int ichannel, int isample) const;
    double  GetValueFast(int ievent, int ichannel, int isample) const; //no argument validity check!

    const QVector<double>* GetWaveformPtr(int ievent, int ichannel) const;
    const QVector<double>* GetWaveformPtrFast(int ievent, int ichannel) const; //no argument validity check!

    double  GetMax(int ievent, int ichannel) const;
    double  GetMaxFast(int ievent, int ichannel) const;
    double  GetMin(int ievent, int ichannel) const;
    double  GetMinFast(int ievent, int ichannel) const;

    int     GetMaxSample(int ievent, int ichannel) const;
    int     GetMaxSampleFast(int ievent, int ichannel) const;
    int     GetMinSample(int ievent, int ichannel) const;
    int     GetMinSampleFast(int ievent, int ichannel) const;

    int     GetSampleWhereFirstBelow(int ievent, int ichannel, int threshold) const;
    int     GetSampleWhereFirstBelowFast(int ievent, int ichannel, int threshold) const;
    int     GetSampleWhereFirstAbove(int ievent, int ichannel, int threshold) const;
    int     GetSampleWhereFirstAboveFast(int ievent, int ichannel, int threshold) const;

    // processing successful?
    bool    isValid() const {return (waveData.size()>0 && numChannels>0 && numSamples>0);}

    // parameter requests
    int     GetNumSamples()   const {return numSamples;}      // number of samples in the waveform
    int     GetNumChannels()  const {return numChannels;}     // number of channels per event
    int     GetNumEvents()    const {return waveData.size();} // number of events in the datafile
    int     GetNumBadEvents() const {return numBadEvents;}    // number of disreguarded events - they had wrong number of samples

    void    ClearData();

private:
    const MasterConfig* Config;
    QVector < QVector < QVector <double> > > waveData;  // format:  [event] [hardware chanel] [sample]

    int     numSamples;
    int     numChannels;
    int     numBadEvents;

    void    readRawData();    // read raw data from the hld file
    void    smoothData();     // smooth raw data

    void    doAdjacentAverage(QVector<double> &arr, int numPoints);
    void    doAdjacentWeightedAverage(QVector<double> &arr, int numPoints);

    void    substractPedestals();

};

#endif // TRB3READER_H
