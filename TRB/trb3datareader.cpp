#include "trb3datareader.h"
#include "masterconfig.h"

#ifdef DABC
#include "hadaq/defines.h"
#include "hadaq/api.h"
#endif

#include <QDebug>

const float NaN = std::numeric_limits<float>::quiet_NaN();

Trb3dataReader::Trb3dataReader(MasterConfig *Config) :
    Config(Config), numSamples(0), numChannels(0) {}

const QString Trb3dataReader::Read(const QString& FileName)
{
    qDebug() << "--> Reading hld file...";
    readRawData(FileName, Config->HldProcessSettings.NumChannels, Config->HldProcessSettings.NumSamples);

    if ( Config->HldProcessSettings.NumChannels != 0)
    {
        if ( Config->HldProcessSettings.NumChannels != numChannels )
        {
            waveData.clear();
            numChannels = 0;
            numSamples = 0;
            return "--- Number of channels in file is different from requested";
        }
    }

    if (isEmpty()) return "--- Read of hld file failed or all events were rejected!";

    bool bOK = Config->UpdateNumberOfHardwareChannels(numChannels);
    if (!bOK) return "The number of hardware channels in the file (" + QString::number(numChannels) + ") is incompatible with the defined number of logical channels";

    if (Config->bSmoothingBeforePedestals)
    {
        if (Config->bSmoothWaveforms)
        {
            qDebug() << "--> Smoothing waveforms...";
            smoothData();
        }
        if (Config->bPedestalSubstraction)
        {
            qDebug() << "--> Substracting pedestals...";
            substractPedestals();
        }
    }
    else
    {
        if (Config->bPedestalSubstraction)
        {
            qDebug() << "--> Substracting pedestals...";
            substractPedestals();
        }
        if (Config->bSmoothWaveforms)
        {
            qDebug() << "--> Smoothing waveforms...";
            smoothData();
        }
    }

    //qDebug() << "--> Done!";

    return "";
}

float Trb3dataReader::GetValue(int ievent, int ichannel, int isample) const
{
    if (ievent<0 || ievent>=waveData.size()) return NaN;
    if (ichannel<0 || ichannel>=numChannels) return NaN;
    if (isample<0 || isample>=numSamples) return NaN;

    return waveData.at(ievent).at(ichannel).at(isample);
}

float Trb3dataReader::GetValueFast(int ievent, int ichannel, int isample) const
{
    return waveData.at(ievent).at(ichannel).at(isample);
}

bool Trb3dataReader::SetValue(int ievent, int ichannel, int isample, float value)
{
    if (ievent<0 || ievent>=waveData.size()) return false;
    if (ichannel<0 || ichannel>=numChannels) return false;
    if (isample<0 || isample>=numSamples) return false;

    waveData[ievent][ichannel][isample] = value;
    return true;
}

void Trb3dataReader::SetValueFast(int ievent, int ichannel, int isample, float value)
{
    waveData[ievent][ichannel][isample] = value;
}

const QVector<float> *Trb3dataReader::GetWaveformPtr(int ievent, int ichannel) const
{
    if (ievent<0 || ievent>=waveData.size()) return 0;
    if (ichannel<0 || ichannel>=numChannels) return 0;

    return &(waveData.at(ievent).at(ichannel));
}

const QVector<float> *Trb3dataReader::GetWaveformPtrFast(int ievent, int ichannel) const
{
    return &(waveData.at(ievent).at(ichannel));
}

bool Trb3dataReader::SetWaveform(int ievent, int ichannel, const QVector<float>& array)
{
    if (ievent<0 || ievent>=waveData.size()) return false;
    if (ichannel<0 || ichannel>=numChannels) return false;
    if (array.size() != numSamples)
    {
        if (numSamples != 0) return false;
        else numSamples = array.size();
    }

    waveData[ievent][ichannel] = array;
    return true;
}

void Trb3dataReader::SetWaveformFast(int ievent, int ichannel, const QVector<float> &array)
{
    waveData[ievent][ichannel] = array;
}

float Trb3dataReader::GetMax(int ievent, int ichannel) const
{
    if (ievent<0 || ievent>=waveData.size()) return NaN;
    if (ichannel<0 || ichannel>=numChannels) return NaN;

    return GetMaxFast(ievent, ichannel);
}

float Trb3dataReader::GetMaxFast(int ievent, int ichannel) const
{
    const QVector <float>& vec = waveData.at(ievent).at(ichannel);

    float max = vec.at(0);
    for (int i=1; i<vec.size(); i++)
        if (vec.at(i)>max) max = vec.at(i);
    return max;
}

float Trb3dataReader::GetMin(int ievent, int ichannel) const
{
    if (ievent<0 || ievent>=waveData.size()) return NaN;
    if (ichannel<0 || ichannel>=numChannels) return NaN;

    return GetMinFast(ievent, ichannel);
}

float Trb3dataReader::GetMinFast(int ievent, int ichannel) const
{
    const QVector <float>& vec = waveData.at(ievent).at(ichannel);

    float min = vec.at(0);
    for (int i=1; i<vec.size(); i++)
        if (vec.at(i)<min) min = vec.at(i);
    return min;
}

int Trb3dataReader::GetMaxSample(int ievent, int ichannel) const
{
    if (ievent<0 || ievent>=waveData.size()) return -1;
    if (ichannel<0 || ichannel>=numChannels) return -1;

    return GetMaxSampleFast(ievent, ichannel);
}

int Trb3dataReader::GetMaxSampleFast(int ievent, int ichannel) const
{
    const QVector <float>& vec = waveData.at(ievent).at(ichannel);

    float max = vec.at(0);
    int imax = 0;
    for (int i=1; i<vec.size(); i++)
        if (vec.at(i)>max)
        {
            imax = i;
            max = vec.at(i);
        }
    return imax;
}

int Trb3dataReader::GetMinSample(int ievent, int ichannel) const
{
    if (ievent<0 || ievent>=waveData.size()) return -1;
    if (ichannel<0 || ichannel>=numChannels) return -1;

    return GetMinSampleFast(ievent, ichannel);
}

int Trb3dataReader::GetMinSampleFast(int ievent, int ichannel) const
{
    const QVector <float>& vec = waveData.at(ievent).at(ichannel);

    float min = vec.at(0);
    int imin = 0;
    for (int i=1; i<vec.size(); i++)
        if (vec.at(i)<min)
        {
            imin = i;
            min = vec.at(i);
        }
    return imin;
}

int Trb3dataReader::GetSampleWhereFirstBelow(int ievent, int ichannel, int threshold) const
{
    if (ievent<0 || ievent>=waveData.size()) return -1;
    if (ichannel<0 || ichannel>=numChannels) return -1;

    return GetSampleWhereFirstBelowFast(ievent, ichannel, threshold);
}

int Trb3dataReader::GetSampleWhereFirstBelowFast(int ievent, int ichannel, int threshold) const
{
    const QVector <float>& vec = waveData.at(ievent).at(ichannel);

    for (int i=0; i<vec.size(); i++)
        if (vec.at(i)<threshold) return i;
    return -1;
}

int Trb3dataReader::GetSampleWhereFirstAbove(int ievent, int ichannel, int threshold) const
{
    if (ievent<0 || ievent>=waveData.size()) return -1;
    if (ichannel<0 || ichannel>=numChannels) return -1;

    return GetSampleWhereFirstAboveFast(ievent, ichannel, threshold);
}

int Trb3dataReader::GetSampleWhereFirstAboveFast(int ievent, int ichannel, int threshold) const
{
    const QVector <float>& vec = waveData.at(ievent).at(ichannel);

    for (int i=0; i<vec.size(); i++)
        if (vec.at(i)>threshold) return i;
    return -1;
}

void Trb3dataReader::substractPedestals()
{
    for (int ievent=0; ievent<waveData.size(); ievent++)
        for (int ichannel=0; ichannel<numChannels; ichannel++)
        {
            float pedestal = 0;
            for (int isample = Config->PedestalFrom; isample <= Config->PedestalTo; isample++)
                pedestal += waveData.at(ievent).at(ichannel).at(isample);
            pedestal /= ( Config->PedestalTo + 1 - Config->PedestalFrom );

            for (int isample = 0; isample < numSamples; isample++)
                waveData[ievent][ichannel][isample] -= pedestal;
        }
}

void Trb3dataReader::readRawData(const QString &FileName, int enforceNumChannels, int enforceNumSamples)
{
#ifdef DABC
    waveData.clear();

    numChannels = enforceNumChannels;
    numSamples = enforceNumSamples;

    hadaq::ReadoutHandle ref = hadaq::ReadoutHandle::Connect(FileName.toLocal8Bit().data());
    hadaq::RawEvent* evnt = 0;
    //evnt = ref.NextEvent(1.0);
    //evnt = ref.NextEvent(1.0);
    bool bReportOnStart = true;

    numBadEvents = 0;
    numAllEvents = 0;
    while ( (evnt = ref.NextEvent(1.0)) )
    {
        bool bBadEvent = false;
        int foundChannels = 0;
        QVector < QVector <float> > thisEventData;  //format: [channel] [sample]

        // loop over sections
        hadaq::RawSubevent* sub = 0;
        while ( (sub=evnt->NextSubevent(sub)) )
        {
            unsigned trbSubEvSize = sub->GetSize() / 4 - 4;
            //qDebug() << "--> Section found, size: "<< trbSubEvSize << "\n";

            unsigned ix = 0;            

            while (ix < trbSubEvSize)
            { // loop over subsubevents

                unsigned hadata = sub->Data(ix++);

                unsigned datalen = (hadata >> 16) & 0xFFFF;
                int datakind = hadata & 0xFFFF;

                if (bReportOnStart) qDebug() << "--> Data block found with datakind: " << datakind;

                unsigned ixTmp = ix;

                if (Config->IsGoodDatakind(datakind))
                {
                    // last word in the data block identifies max. ADC# and max. channel
                    // assuming they are written consecutively - seems to be the case so far
                    unsigned lastword = sub->Data(ix+datalen-1);
                    int ch_per_adc = ((lastword >> 16) & 0xF) + 1;
                    int n_adcs = ((lastword >> 20) & 0xF) + 1;

                    int channels = ch_per_adc * n_adcs;

                    if (channels > 0)
                    {
                        int samples = datalen/channels;
                        if (bReportOnStart) qDebug() << "--> This is an ADC block. Channels: "<<channels<<"   Samples: "<< samples;

                        // reserve the necessary vectors for the waveforms and fill the data
                        int oldSize = thisEventData.size();
                        thisEventData.resize( oldSize + channels );
                        for (int ic=0; ic<channels; ic++)
                        {
                            thisEventData[oldSize+ic].resize(samples);
                            for (int is=0; is<samples; is++)
                                thisEventData[oldSize+ic][is] = (sub->Data(ix++) & 0xFFFF);
                        }
                        foundChannels = oldSize + channels;

                        if (numSamples != 0)
                        {
                            if (samples != numSamples)
                            {
                                bBadEvent = true;
                                if (numBadEvents<50) qDebug() << "----- Event #" << waveData.size()-1 << " has wrong number of samples ("<< samples <<")\n";
                            }
                        }
                        else numSamples = samples;
                    }
                }
                ix = ixTmp + datalen;
            }            
        }

        if (numChannels != 0)
        {
            if (foundChannels != numChannels)
            {
                qDebug() << "Found event with wrong number of channels:"<<foundChannels<<"while expecting"<<numChannels;
                ClearData();
                break;
            }
        }
        else numChannels = foundChannels;

        //qDebug() << "Event processed.\n";
        if (bBadEvent)
        {
            numBadEvents++;
            //qDebug() << "Ignored!";
        }
        else
        {
            waveData << thisEventData;
            //qDebug() << "New data size: "<<data.size();
        }
        bReportOnStart = false;
        numAllEvents++;
    }

    ref.Disconnect();

    qDebug() << "--> Data read completed\n--> Events: "<< waveData.size() <<" Channels: "<<numChannels << "  Samples: "<<numSamples;
    if (numBadEvents > 0) qDebug() << "--> " << numBadEvents << " bad events were disreguarded!";
#endif
}

const QString Trb3dataReader::GetFileInfo(const QString& FileName) const
{
    QString output;
#ifdef DABC
    bool bReportOnStart = true;
    int numEvents = 0;

    hadaq::ReadoutHandle ref = hadaq::ReadoutHandle::Connect(FileName.toLocal8Bit().data());
    hadaq::RawEvent* evnt = 0;

    while ( (evnt = ref.NextEvent(1.0)) )
    {
        // loop over sections
        hadaq::RawSubevent* sub = 0;
        while ( (sub=evnt->NextSubevent(sub)) )
        {
            unsigned trbSubEvSize = sub->GetSize() / 4 - 4;
            //qDebug() << "--> Section found, size: "<< trbSubEvSize << "\n";

            unsigned ix = 0;

            while (ix < trbSubEvSize)
            { // loop over subsubevents

                unsigned hadata = sub->Data(ix++);

                unsigned datalen = (hadata >> 16) & 0xFFFF;
                int datakind = hadata & 0xFFFF;

                if (bReportOnStart) output += "Data block with datakind: " + QString::number(datakind) + "\n";

                unsigned ixTmp = ix;

                if (Config->IsGoodDatakind(datakind))
                {
                    // last word in the data block identifies max. ADC# and max. channel
                    // assuming they are written consecutively - seems to be the case so far
                    unsigned lastword = sub->Data( ix + datalen - 1 );
                    int ch_per_adc = ((lastword >> 16) & 0xF) + 1;
                    int n_adcs = ((lastword >> 20) & 0xF) + 1;

                    int channels = ch_per_adc * n_adcs;

                    if (channels > 0)
                    {
                        int samples = datalen/channels;
                        if (bReportOnStart) output += "--> This is an ADC block. Channels: " +QString::number(channels) +"   Samples: " +QString::number(samples) +"\n";
                    }
                    else
                        if (bReportOnStart) output += "==> This is an ADC block. Error: number of channels is 0!\n";
                }
                ix = ixTmp + datalen;
            }
        }
        bReportOnStart = false;
        numEvents++;
    }

    if (output.isEmpty())
    {
        output = "Read failed or bad file format";
    }
    else
        output += "Number of events: " + QString::number(numEvents);

    ref.Disconnect();
#endif

    return output;
}

void Trb3dataReader::smoothData()
{
    for (int ievent=0; ievent<waveData.size(); ievent++)
        for (int ichannel=0; ichannel<numChannels; ichannel++)
        {
            if (Config->AdjacentAveraging_bOn)
            {
                if (Config->AdjacentAveraging_bWeighted)
                    doAdjacentWeightedAverage(waveData[ievent][ichannel], Config->AdjacentAveraging_NumPoints);
                else
                    doAdjacentAverage        (waveData[ievent][ichannel], Config->AdjacentAveraging_NumPoints);
            }
        }
}

void Trb3dataReader::doAdjacentAverage(QVector<float> &arr, int numPoints)
{
   QVector<float> arrOriginal = arr;
   for (int is=0; is<numSamples; is++)
   {
       int num = 0;
       float sum = 0;
       for (int id=-numPoints; id<numPoints+1; id++)
       {
           int i = is + id;
           if (i<0 || i>numSamples-1) continue;
           num++;
           sum += arrOriginal[i];
       }
       arr[is] = sum/num;
   }
}

void Trb3dataReader::doAdjacentWeightedAverage(QVector<float> &arr, int numPoints)
{
    QVector<float> arrOriginal = arr;
    for (int is=0; is<numSamples; is++)
    {
        float sum = 0;
        float sumWeights = 0;
        for (int id=-numPoints; id<numPoints+1; id++)
        {
            int i = is + id;
            if (i<0 || i>numSamples-1) continue;

            float weight = id/(numPoints+1.0);
            weight = 1.0 - weight*weight;
            sumWeights += weight;
            sum += arrOriginal[i]*weight;
        }
        arr[is] = sum/sumWeights;
    }
}

void Trb3dataReader::ClearData()
{
    waveData.clear();
    numChannels = 0;
    numSamples = 0;
    numBadEvents = 0;
    numAllEvents = 0;
}
