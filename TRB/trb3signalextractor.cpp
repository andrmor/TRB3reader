#include "trb3signalextractor.h"
#include "trb3datareader.h"
#include "masterconfig.h"

#include <limits>

#include <QDebug>

const float NaN = std::numeric_limits<float>::quiet_NaN();

Trb3signalExtractor::Trb3signalExtractor(const MasterConfig *Config, const Trb3dataReader* Reader) :
    Config(Config), Reader(Reader), numChannels(0) {}

bool Trb3signalExtractor::ExtractSignals()
{    
    if ( Reader->isEmpty() )
    {
        qDebug() << "--- Cannot start extraction, there are no waveform data";
        return false;
    }

    numChannels = Reader->CountChannels();

    qDebug() << "--> Extracting signals from waveforms...";
    ExtractAllSignals();
    qDebug() << "--> Done!\n--> Trb3signalExtractor finished signal extraction.";
    return true;
}

void Trb3signalExtractor::GenerateDummyData()
{
    int numEvents = Reader->CountEvents();
    numChannels = Reader->CountChannels();

    signalData.resize(numEvents);
    for (int i=0; i<numEvents; i++) signalData[i] = QVector<float>(numChannels, 0);
    RejectedEvents = QVector<bool>(numEvents, false);
}

float Trb3signalExtractor::GetSignal(int ievent, int ichannel) const
{
    if (ievent<0 || ievent>=signalData.size()) return NaN;
    if (ichannel<0 || ichannel>=signalData.at(ievent).size()) return NaN;

    return signalData.at(ievent).at(ichannel);
}

float Trb3signalExtractor::GetSignalFast(int ievent, int ichannel) const
{
    return signalData.at(ievent).at(ichannel);
}

const QVector<float>* Trb3signalExtractor::GetSignals(int ievent) const
{
    if (ievent<0 || ievent>=signalData.size()) return 0;

    return &(signalData.at(ievent));
}

const QVector<float> *Trb3signalExtractor::GetSignalsFast(int ievent) const
{
    return &(signalData.at(ievent));
}

bool Trb3signalExtractor::SetSignal(int ievent, int ichannel, float value)
{
    if (ievent<0 || ievent>=signalData.size()) return false;
    if (ichannel<0 || ichannel>=signalData.at(ievent).size()) return false;

    signalData[ievent][ichannel] = value;
    return true;
}

void Trb3signalExtractor::SetSignalFast(int ievent, int ichannel, float value)
{
    signalData[ievent][ichannel] = value;
}

bool Trb3signalExtractor::SetSignals(int ievent, const QVector<float> &values)
{
    if (ievent<0 || ievent>=signalData.size()) return false;
    if (values.size() != numChannels) return false;

    for (int i=0; i<numChannels; i++) signalData[ievent][i] = values.at(i);
    return true;
}

void Trb3signalExtractor::SetSignalsFast(int ievent, const QVector<float> &values)
{
    for (int i=0; i<values.size(); i++) signalData[ievent][i] = values.at(i);
}

bool Trb3signalExtractor::SetRejected(int ievent, bool flag)
{
    if (ievent<0 || ievent>=RejectedEvents.size()) return false;
    RejectedEvents[ievent] = flag;
    return true;
}

void Trb3signalExtractor::SetAllRejected(bool flag)
{
    for (bool& b : RejectedEvents) b = flag;
}

void Trb3signalExtractor::ClearData()
{
    signalData.clear();
}

int Trb3signalExtractor::CountChannels() const
{
    if (signalData.size() == 0) return 0;
    return signalData.at(0).size();
}

bool Trb3signalExtractor::IsRejectedEvent(int ievent) const
{
    if (ievent < 0 || ievent >= RejectedEvents.size()) return true;
    return RejectedEvents.at(ievent);
}

void Trb3signalExtractor::ExtractAllSignals()
{
    //qDebug() << "Method:"<< Config.SignalExtractionMethod;
    const int numEvents = Reader->CountEvents();
    signalData.resize(numEvents);

    //clear -> fill all with false
    RejectedEvents = QVector<bool>(numEvents, false);

    const int numSamples = Reader->CountSamples();
    if (numSamples == 0) return;

    if (Config->SignalExtractionMethod == 2 &&  (Config->CommonSampleNumber<0 || Config->CommonSampleNumber>=numSamples) )
    {
        qWarning() << "Common sample number "<< Config->CommonSampleNumber<<"is not valid. Number of samples in the data:"<< numSamples;
        return;
    }

    for (int ievent=0; ievent<signalData.size(); ievent++)
    {
        signalData[ievent].resize(numChannels);

        // going through all channels, finding where they have reached maximum. Updating global max (+ and -) over all channels for this event
        iNegMaxSample = iPosMaxSample = 0;
        NegMaxValue = PosMaxValue = -1.0e10;
        for (int ichannel=0; ichannel<numChannels; ichannel++)
        {
            if (Config->IsIgnoredHardwareChannel(ichannel) )
            {
                signalData[ievent][ichannel] = 0;
                continue;
            }
            signalData[ievent][ichannel] = extractSignalFromWaveform(ievent, ichannel);
        }

        if (RejectedEvents.at(ievent)) continue;

        // if activated, check that the maximum is reached in the allowed gate
        if (Config->bNegMaxGate)
        {
            if (iNegMaxSample < Config->NegMaxGateFrom  || iNegMaxSample > Config->NegMaxGateTo )
            {
                RejectedEvents[ievent] = true;
                continue;
            }
        }
        else if (Config->bPosMaxGate)
        {
            if (iPosMaxSample < Config->PosMaxGateFrom  || iPosMaxSample > Config->PosMaxGateTo )
            {
                RejectedEvents[ievent] = true;
                continue;
            }
        }

        // for methods 1 and 2 reading signal value at the same sample #
        switch (Config->SignalExtractionMethod)
        {
        case 0: break; //already done
        case 1:
            //qDebug() << "max samples are at: -:"<<iNegMaxSample<<NegMax<<"   +:"<<iPosMaxSample<<PosMax;
            for (int ichannel=0; ichannel<numChannels; ichannel++)
              {
                if (signalData.at(ievent).at(ichannel) == 0) continue; //respect suppression - applicable since it operates with max of waveform
                if ( Config->IsNegativeHardwareChannel(ichannel) )
                    signalData[ievent][ichannel] = -Reader->GetValueFast(ievent, ichannel, iNegMaxSample);
                else
                    signalData[ievent][ichannel] = Reader->GetValueFast(ievent, ichannel, iPosMaxSample);
              }
            break;
        case 2:
            for (int ichannel=0; ichannel<numChannels; ichannel++)
              {
                if (signalData.at(ievent).at(ichannel) == 0) continue; //respect suppression - applicable since it operates with max of waveform
                if ( Config->IsNegativeHardwareChannel(ichannel) )
                    signalData[ievent][ichannel] = -Reader->GetValueFast(ievent, ichannel, Config->CommonSampleNumber);
                else
                    signalData[ievent][ichannel] = Reader->GetValueFast(ievent, ichannel, Config->CommonSampleNumber);
              }
            break;
        case 3:
            for (int ichannel=0; ichannel<numChannels; ichannel++)
            {
                if (signalData.at(ievent).at(ichannel) == 0) continue; //respect suppression - applicable since it operates with max of waveform
                const QVector<float>* wave = Reader->GetWaveformPtrFast(ievent, ichannel);
                float val = 0;
                for (int isam = Config->IntegrateFrom; isam<=Config->IntegrateTo; ++isam)
                {
                    if (isam < wave->size()) val += wave->at(isam);
                    else break;
                }
                signalData[ievent][ichannel] = ( (Config->IsNegativeHardwareChannel(ichannel)) ? -val : val);
            }
            break;
        default:
            qWarning() << "Unknown signal extraction method!";
            break;
        }

        //go to next event
    }


    //statistics on rejected events:
    int rejected = 0;
    for (int ievent=0; ievent<signalData.size(); ievent++)
        if (RejectedEvents.at(ievent)) rejected++;
    qDebug() << "Rejected"<<rejected<<"events from total"<<signalData.size();

    TimeData = Reader->timeData; // !!!***
}

float Trb3signalExtractor::extractSignalFromWaveform(int ievent, int ichannel, bool *WasSetToZero)
{
    float sig;

    if ( Config->IsNegativeHardwareChannel(ichannel) )
    {
        sig = -extractMin(Reader->GetWaveformPtrFast(ievent, ichannel));

        if (Config->bZeroSignalIfPeakOutside_Negative)
        {
            if (iMin < Config->ZeroSignalIfPeakBefore_Negative) return 0;
            if (iMin > Config->ZeroSignalIfPeakAfter_Negative)  return 0;
        }

        if (Config->bNegativeThreshold)
            if (sig < Config->NegativeThreshold)
            {
                if (WasSetToZero) *WasSetToZero = true;
                return 0;
            }

        if (Config->bNegativeIgnore)
            if (sig > Config->NegativeIgnore)
            {
                RejectedEvents[ievent] = true;
                return sig;
            }

        if (Config->bZeroSignalIfReverse)
            if ( extractMax(Reader->GetWaveformPtrFast(ievent, ichannel)) > Config->ReverseMaxThreshold*sig)
            {
                if (WasSetToZero) *WasSetToZero = true;
                return 0;
            }

        if (sig > NegMaxValue)
           {
                NegMaxValue = sig;
                iNegMaxSample = iMin;
            }
    }
    else
    {
        sig = extractMax(Reader->GetWaveformPtrFast(ievent, ichannel));

        if (Config->bZeroSignalIfPeakOutside_Positive)
        {
            if (iMax < Config->ZeroSignalIfPeakBefore_Positive) return 0;
            if (iMax > Config->ZeroSignalIfPeakAfter_Positive)  return 0;
        }

        if (Config->bPositiveThreshold)
            if (sig < Config->PositiveThreshold)
            {
                if (WasSetToZero) *WasSetToZero = true;
                return 0;
            }

        if (Config->bPositiveIgnore)
            if (sig > Config->PositiveIgnore)
            {
                RejectedEvents[ievent] = true;
                return sig;
            }

        if (Config->bZeroSignalIfReverse)
            if ( -extractMin(Reader->GetWaveformPtrFast(ievent, ichannel)) > Config->ReverseMaxThreshold*sig)
            {
                if (WasSetToZero) *WasSetToZero = true;
                return 0;
            }

        if (sig > PosMaxValue)
           {
                PosMaxValue = sig;
                iPosMaxSample = iMax;
            }
    }

    if (WasSetToZero) *WasSetToZero = false;
    return sig;
}

float Trb3signalExtractor::extractMax(const QVector<float> *arr)
{
    int max = (*arr)[0];
    iMax = 0;
    for (int i=1; i<arr->size(); i++)
        if ( (*arr)[i]>max)
        {
            max = (*arr)[i];
            iMax = i;
        }
    return max;
}

float Trb3signalExtractor::extractMin(const QVector<float> *arr)
{
    int min = (*arr)[0];
    iMin = 0;
    for (int i=1; i<arr->size(); i++)
        if ( (*arr)[i]<min)
        {
            min = (*arr)[i];
            iMin = i;
        }
    return min;
}
