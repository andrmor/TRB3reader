#include "trb3datareader.h"

#include "hadaq/defines.h"
#include "hadaq/api.h"

Trb3dataReader::Trb3dataReader() :
    numSamples(0), numChannels(0) {}

bool Trb3dataReader::Read()
{
    if (Config.filename == "")
    {
        std::cout << "--- File name is not set!\n" << std::flush;
        return false;
    }

    std::cout << "--> Reading hld file...\n" << std::flush;
    readRawData();
    if (!isValid())
    {
        std::cout << "--- Read of hld file failed!\n" << std::flush;
        return false;
    }

    if (Config.bSmoothingBeforePedestals)
    {
        if (Config.bSmoothWaveforms)
        {
            std::cout << "--> Smoothing waveforms...\n" << std::flush;
            smoothData();
        }
        if (Config.bPedestalSubstraction)
        {
            std::cout << "--> Substracting pedestals...\n" << std::flush;
            substractPedestals();
        }
    }
    else
    {
        if (Config.bPedestalSubstraction)
        {
            std::cout << "--> Substracting pedestals...\n" << std::flush;
            substractPedestals();
        }
        if (Config.bSmoothWaveforms)
        {
            std::cout << "--> Smoothing waveforms...\n" << std::flush;
            smoothData();
        }
    }

    //std::cout << "--> Done!\n" << std::flush;

    return true;
}

void Trb3dataReader::substractPedestals()
{
    for (int ievent=0; ievent<waveData.size(); ievent++)
        for (int ichannel=0; ichannel<numChannels; ichannel++)
        {
            double pedestal = 0;
            for (int isample = Config.PedestalFrom; isample <= Config.PedestalTo; isample++)
                pedestal += waveData.at(ievent).at(ichannel).at(isample);
            pedestal /= ( Config.PedestalTo + 1 - Config.PedestalFrom );

            for (int isample = 0; isample < numSamples; isample++)
                waveData[ievent][ichannel][isample] -= pedestal;
        }
}

void Trb3dataReader::readRawData()
{
    waveData.clear();
    numChannels = 0;
    numSamples = 0;

    hadaq::ReadoutHandle ref = hadaq::ReadoutHandle::Connect(Config.filename);
    hadaq::RawEvent* evnt = 0;
    //evnt = ref.NextEvent(1.0);
    //evnt = ref.NextEvent(1.0);
    bool bReportOnStart = true;

    numBadEvents = 0;
    while ( (evnt = ref.NextEvent(1.0)) )
    {
        bool bBadEvent = false;
        std::vector < std::vector <int> > thisEventData;  //format: [channel] [sample]

        // loop over sections
        hadaq::RawSubevent* sub = 0;
        while ( (sub=evnt->NextSubevent(sub)) )
        {
            unsigned trbSubEvSize = sub->GetSize() / 4 - 4;
            //std::cout << "--> Section found, size: "<< trbSubEvSize << "\n";

            unsigned ix = 0;            

            while (ix < trbSubEvSize)
            { // loop over subsubevents

                unsigned hadata = sub->Data(ix++);

                unsigned datalen = (hadata >> 16) & 0xFFFF;
                unsigned datakind = hadata & 0xFFFF;

                if (bReportOnStart) std::cout << "--> Data block found with datakind: " << datakind << "\n"<<std::flush;

                unsigned ixTmp = ix;

                if (datakind == 0xc313 || datakind == 49152 || datakind == 49155)
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
                        if (bReportOnStart) std::cout << "--> This is an ADC block. Channels: "<<channels<<"   Samples: "<< samples << "\n"<<std::flush;

                        // reserve the necessary vectors for the waveforms and fill the data
                        int oldSize = thisEventData.size();
                        thisEventData.resize( oldSize + channels );
                        for (int ic=0; ic<channels; ic++)
                        {
                            thisEventData[oldSize+ic].resize(samples);
                            for (int is=0; is<samples; is++)
                                thisEventData[oldSize+ic][is] = (sub->Data(ix++) & 0xFFFF);
                        }

                        if (numSamples!=0 && numSamples!=samples)
                        {                            
                            bBadEvent = true;
                            if (numBadEvents<500) std::cout << "----- Event #" << waveData.size()-1 << " has wrong number of samples ("<< samples <<")\n"<<std::flush;
                        }
                        else
                        {
                            numSamples = samples;
                            numChannels = oldSize + channels;
                        }
                    }
                }

                //else ix+=datalen;
                ix = ixTmp + datalen;  //more general (and safer) way to do the same
                //std::cout << "ix:"<< ix <<"\n"<<std::flush;
            }            
        }

        //std::cout << "Event processed.\n";
        if (bBadEvent)
        {
            numBadEvents++;
            //std::cout << "Ignored!"<<"\n"<<std::flush;
        }
        else
        {
            waveData.push_back( thisEventData );
            //std::cout << "New data size: "<<data.size()<<"\n"<<std::flush;
        }
        bReportOnStart = false;
    }

    ref.Disconnect();
    std::cout << "--> Data read completed\n--> Events: "<< waveData.size() <<" Channels: "<<numChannels << "  Samples: "<<numSamples<<"\n"<< std::flush;
    if (numBadEvents > 0) std::cout << "--> " << numBadEvents << " bad events were disreguarded!\n"<< std::flush;
}

void Trb3dataReader::smoothData()
{
    for (int ievent=0; ievent<waveData.size(); ievent++)
        for (int ichannel=0; ichannel<numChannels; ichannel++)
        {
            if (Config.AdjacentAveraging_bOn)
            {
                if (Config.AdjacentAveraging_bWeighted)
                    doAdjacentWeightedAverage(waveData[ievent][ichannel], Config.AdjacentAveraging_NumPoints);
                else
                    doAdjacentAverage        (waveData[ievent][ichannel], Config.AdjacentAveraging_NumPoints);
            }
        }
}

void Trb3dataReader::doAdjacentAverage(std::vector<int> &arr, int numPoints)
{
   std::vector<int> arrOriginal = arr;
   for (int is=0; is<numSamples; is++)
   {
       int num = 0;
       int sum = 0;
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

void Trb3dataReader::doAdjacentWeightedAverage(std::vector<int> &arr, int numPoints)
{
    std::vector<int> arrOriginal = arr;
    for (int is=0; is<numSamples; is++)
    {
        double sum = 0;
        double sumWeights = 0;
        for (int id=-numPoints; id<numPoints+1; id++)
        {
            int i = is + id;
            if (i<0 || i>numSamples-1) continue;

            double weight = id/(numPoints+1.0);
            weight = 1.0 - weight*weight;
            sumWeights += weight;
            sum += arrOriginal[i]*weight;
        }
        arr[is] = sum/sumWeights;
    }
}

const std::vector<int>* Trb3dataReader::GetWaveformPtrFast(int ievent, int ichannel) const
{
    return &(waveData[ievent][ichannel]);
}

void Trb3dataReader::ClearData()
{
    waveData.clear();
    numChannels = 0;
    numSamples = 0;
}
