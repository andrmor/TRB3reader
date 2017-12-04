#ifndef TRB3READER_H
#define TRB3READER_H

#include <QVector>
#include <QString>

class MasterConfig;

class Trb3dataReader
{
public:
    Trb3dataReader(const MasterConfig* Config);

      // Reading waveform data from the file, optional - substract pedestals and apply smoothing
    bool    Read();
    const QString GetFileInfo(const QString FileName) const;

    float   GetValue(int ievent, int ichannel, int isample) const;
    float   GetValueFast(int ievent, int ichannel, int isample) const; //no argument validity check!

    bool    SetValue(int ievent, int ichannel, int isample, float value);
    void    SetValueFast(int ievent, int ichannel, int isample, float value); //no argument validity check!

    const QVector<float>* GetWaveformPtr(int ievent, int ichannel) const;
    const QVector<float>* GetWaveformPtrFast(int ievent, int ichannel) const; //no argument validity check!

    bool    SetWaveform(int ievent, int ichannel, const QVector<float> &array);
    void    SetWaveformFast(int ievent, int ichannel, const QVector<float> &array);

    float   GetMax(int ievent, int ichannel) const;
    float   GetMaxFast(int ievent, int ichannel) const;
    float   GetMin(int ievent, int ichannel) const;
    float   GetMinFast(int ievent, int ichannel) const;

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
    int     CountSamples()   const {return numSamples;}      // number of samples in the waveform
    int     CountChannels()  const {return numChannels;}     // number of channels per event
    int     CountEvents()    const {return waveData.size();} // number of events in the datafile
    int     CountBadEvents() const {return numBadEvents;}    // number of disreguarded events - they had wrong number of samples

    void    ClearData();

private:
    const MasterConfig* Config;
    QVector < QVector < QVector <float> > > waveData;  // format:  [event] [hardware chanel] [sample]

    int     numSamples;
    int     numChannels;
    int     numBadEvents;

    void    readRawData();    // read raw data from the hld file
    void    smoothData();     // smooth raw data

    void    doAdjacentAverage(QVector<float> &arr, int numPoints);
    void    doAdjacentWeightedAverage(QVector<float> &arr, int numPoints);

    void    substractPedestals();

};

#endif // TRB3READER_H
