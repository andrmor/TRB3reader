#include "trb3signalextractor.h"
#include "trb3datareader.h"

#include <iostream>
#include <limits>

#include <QDebug>

const double NaN = std::numeric_limits<double>::quiet_NaN();

Trb3signalExtractor::Trb3signalExtractor(const Trb3dataReader *reader) :
    reader(reader), numChannels(0) {}

void Trb3signalExtractor::UpdateConfig(const MasterConfig* config)
{
    Config = *config;
    setPolarity(Config.NegativeChannels);
}

void Trb3signalExtractor::setPolarity(std::vector<int> negativeChannels)
{   
    for (int ichannel : negativeChannels)
    {
        if (ichannel >= NegPolChannels.size())
            NegPolChannels.resize(ichannel+1, false);
        NegPolChannels[ichannel] = true;
    }
}

bool Trb3signalExtractor::ExtractSignals()
{    
    if (!reader->isValid())
    {
        std::cout << "--- Cannot start extraction, TRBreader reports not ready status\n"<< std::flush;
        return false;
    }

    numChannels = reader->GetNumChannels();
    if (numChannels >= NegPolChannels.size()) NegPolChannels.resize(numChannels+1, false);

    std::cout << "--> Extracting signals from waveforms...\n" << std::flush;
    ExtractAllSignals();
    std::cout << "--> Done!\n--> Trb3signalExtractor finished signal extraction.\n" << std::flush;
    return true;
}

double Trb3signalExtractor::GetSignal(int ievent, int ichannel) const
{
    if (ievent<0 || ievent>=signalData.size()) return NaN;
    if (ichannel<0 || ichannel>=signalData[ievent].size()) return NaN;

    return signalData[ievent][ichannel];
}

double Trb3signalExtractor::GetSignalFast(int ievent, int ichannel) const
{
    return signalData[ievent][ichannel];
}

void Trb3signalExtractor::ClearData()
{
    signalData.clear();
}

std::size_t Trb3signalExtractor::GetNumEvents() const
{
    return signalData.size();
}

std::size_t Trb3signalExtractor::GetNumChannels() const
{
    if (signalData.size() == 0) return 0;
    return signalData[0].size();
}

bool Trb3signalExtractor::IsNegative(std::size_t channel) const
{
    if (channel>=NegPolChannels.size()) return false;
    return NegPolChannels[channel];
}

bool Trb3signalExtractor::IsRejectedEvent(int ievent) const
{
    if (ievent < 0 || ievent >= RejectedEvents.size()) return true;
    return RejectedEvents.at(ievent);
}

void Trb3signalExtractor::ExtractAllSignals()
{
    //qDebug() << "Method:"<< Config.SignalExtractionMethod;
    int numEvents = reader->GetNumEvents();
    signalData.resize(numEvents);
    RejectedEvents.clear();
    RejectedEvents.resize(numEvents, false);
    if (reader->GetNumSamples() == 0) return;

    for (std::size_t ievent=0; ievent<signalData.size(); ievent++)
    {
        signalData[ievent].resize(numChannels);

        iNegMaxSample = iPosMaxSample = 0;
        NegMax = PosMax = -1.0e10;
        for (int ichannel=0; ichannel<numChannels; ichannel++)
        {
            if (Config.IgnoreHardwareChannels.contains(ichannel) )
            {
                signalData[ievent][ichannel] = 0;
                continue;
            }
            signalData[ievent][ichannel] = extractSignalFromWaveform(ievent, ichannel);
        }

        if ( !RejectedEvents.at(ievent) && Config.SignalExtractionMethod==1 )
        {
            //qDebug() << "max samples are at: -:"<<iNegMaxSample<<NegMax<<"   +:"<<iPosMaxSample<<PosMax;
            for (int ichannel=0; ichannel<numChannels; ichannel++)
            {
                if (signalData.at(ievent).at(ichannel) == 0) continue; //respect suppression - applicable since it operates with max of waveform
                if (IsNegative(ichannel))
                    signalData[ievent][ichannel] = -reader->GetValueFast(ievent, ichannel, iNegMaxSample);
                else
                    signalData[ievent][ichannel] = reader->GetValueFast(ievent, ichannel, iPosMaxSample);

            }
        }

    }
}

double Trb3signalExtractor::extractSignalFromWaveform(int ievent, int ichannel, bool *WasSetToZero)
{
    double sig;

    if ( NegPolChannels.at(ichannel) )
    {
        sig = -extractMin(reader->GetWaveformPtrFast(ievent, ichannel));

        if (Config.bNegativeThreshold)
            if (sig < Config.NegativeThreshold)
            {
                if (WasSetToZero) *WasSetToZero = true;
                return 0;
            }

        if (Config.bNegativeIgnore)
            if (sig > Config.NegativeIgnore)
            {
                RejectedEvents[ievent] = true;
                return sig;
            }

        if (Config.bZeroSignalIfReverse)
            if ( extractMax(reader->GetWaveformPtrFast(ievent, ichannel)) > Config.ReverseMaxThreshold*sig)
            {
                if (WasSetToZero) *WasSetToZero = true;
                return 0;
            }

        if (sig > NegMax)
           {
                NegMax = sig;
                iNegMaxSample = iMin;
            }
    }
    else
    {
        sig = extractMax(reader->GetWaveformPtrFast(ievent, ichannel));

        if (Config.bPositiveThreshold)
            if (sig < Config.PositiveThreshold)
            {
                if (WasSetToZero) *WasSetToZero = true;
                return 0;
            }

        if (Config.bPositiveIgnore)
            if (sig > Config.PositiveIgnore)
            {
                RejectedEvents[ievent] = true;
                return sig;
            }

        if (Config.bZeroSignalIfReverse)
            if ( -extractMin(reader->GetWaveformPtrFast(ievent, ichannel)) > Config.ReverseMaxThreshold*sig)
            {
                if (WasSetToZero) *WasSetToZero = true;
                return 0;
            }

        if (sig > PosMax)
           {
                PosMax = sig;
                iPosMaxSample = iMax;
            }
    }

    if (WasSetToZero) *WasSetToZero = false;
    return sig;
}

double Trb3signalExtractor::extractMax(const std::vector<int> *arr)
{
    int max = (*arr)[0];
    for (int i=1; i<arr->size(); i++)
        if ( (*arr)[i]>max)
        {
            max = (*arr)[i];
            iMax = i;
        }
    return max;
}

double Trb3signalExtractor::extractMin(const std::vector<int> *arr)
{
    int min = (*arr)[0];
    for (int i=1; i<arr->size(); i++)
        if ( (*arr)[i]<min)
        {
            min = (*arr)[i];
            iMin = i;
        }
    return min;
}
