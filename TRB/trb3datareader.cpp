#include "trb3datareader.h"
#include "masterconfig.h"

#ifdef DABC
#include "hadaq/defines.h"
#include "hadaq/api.h"
#endif

#include <stdexcept>

#include <QDebug>

const float NaN = std::numeric_limits<float>::quiet_NaN();

Trb3dataReader::Trb3dataReader(MasterConfig *Config) :
    Config(Config), numSamples(0), numChannels(0) {}

/*
QString Trb3dataReader::GetFileInfo(const QString& FileName) const
{
    QString output;

    bool bReportOnStart = true;
    int numEvents = 0;

    hadaq::ReadoutHandle ref = hadaq::ReadoutHandle::Connect(FileName.toLocal8Bit().data());
    hadaq::RawEvent* evnt = 0;

    while ( (evnt = ref.NextEvent(1.0)) )
    {
        // loop over sections
        qDebug() << "---Event---" << numEvents;
        hadaq::RawSubevent * sub = 0;
        while ( (sub=evnt->NextSubevent(sub)) )
        {
            qDebug() << "==>Id:" << QString::number(sub->GetId(), 16) << "==>Decoding:" << QString::number(sub->GetDecoding(), 16);
            unsigned trbSubEvSize = sub->GetSize() / 4 - 4;
            qDebug() << "==>Subevent size: "<< trbSubEvSize;// << "\n";

            unsigned ix = 0;

            while (ix < trbSubEvSize)
            { // loop over subsubevents

                unsigned hadata = sub->Data(ix++);

                unsigned datalen = (hadata >> 16) & 0xFFFF;
                int datakind = hadata & 0xFFFF;

                if (bReportOnStart) output += "Data block with datakind: 0x" + QString::number(datakind, 16) + "\n";
                qDebug() << "====>" << QString::number(datakind, 16);

                unsigned ixTmp = ix;

                //if (Config->IsGoodDatakind(datakind))
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

        // !!!
        break;
        // !!!
    }

    if (output.isEmpty())
    {
        output = "Read failed or bad file format";
    }
    else
        output += "Number of events: " + QString::number(numEvents);

    ref.Disconnect();

    return output;
}
*/

QString Trb3dataReader::GetFileInfo(const QString& FileName)
{
    QString output;

    int numEvents = 0;

    hadaq::ReadoutHandle ref = hadaq::ReadoutHandle::Connect(FileName.toLocal8Bit().data());
    hadaq::RawEvent * evnt = nullptr;

    bool bReportOnStart = true;
    while ( (evnt = ref.NextEvent(1.0)) )
    {
        // loop over sections
        qDebug() << "---Event---" << numEvents << evnt->GetDate() << QString::number(evnt->GetDecoding(), 16);

        hadaq::RawSubevent * sub = nullptr;
        while ( (sub = evnt->NextSubevent(sub)) )
        {
            const int boardID = sub->GetId();
            QString msg = "BoardId: " + QString::number(boardID, 16);// + "Decoding:" + QString::number(sub->GetDecoding(), 16);
            qDebug() << "==>" << msg;
            if (bReportOnStart) output += msg + '\n';

            const unsigned trbSubEvSize = sub->GetSize() / 4 - 4;
            qDebug() << "==>Subevent size: "<< trbSubEvSize;// << "\n";

            if (Config->isTimerBoard(boardID)) processTimingSubEvent(sub, trbSubEvSize, nullptr);
            else if (Config->isADCboard(boardID))
            {
                // time processing is to add later
                const unsigned lastRec = sub->Data(trbSubEvSize-3);
                //qDebug() << "Last" << QString::number(lastRec, 16);
                const unsigned lastId   = (lastRec >> 16) & 0xFFFF;
                const unsigned lastChan = lastId & 0xF;
                const unsigned lastAdc  = (lastId >> 4) & 0xF;
                const unsigned numChan = (lastChan+1) * (lastAdc+1);
                const unsigned numSamples =  (trbSubEvSize-2) / numChan;
                msg = "-->ADC block; Num channels: " + QString::number(numChan) + " Num samples: " + QString::number(numSamples);
                //qDebug() << msg;
                if (bReportOnStart) output += msg + '\n';

                unsigned ix = 0;

                while (ix < trbSubEvSize)
                { // loop over subsubevents

                    unsigned hadata = sub->Data(ix++);

                    unsigned id   = (hadata >> 16) & 0xFFFF;
                    unsigned data = hadata & 0xFFFF;

                    if (data == 0x5555 && id == 1) break;

                    //if (bReportOnStart) output += "Data block with datakind: 0x" + QString::number(datakind, 16) + "\n";
                    //                qDebug() << "====>  id:" << QString::number(id, 16) << "data:" <<  QString::number(data, 16);

                    unsigned chan = id & 0xF;
                    unsigned adc  = (id >> 4) & 0xF;
                    //                qDebug() << "====>      adc:" << adc << "channel:" << chan;
                }
            }
        }
        bReportOnStart = false;
        numEvents++;

        // !!!
//        break;
        // !!!
    }

    if (output.isEmpty())
    {
        output = "Read failed or bad file format";
    }
    else
        output += "Number of events: " + QString::number(numEvents);

    ref.Disconnect();

    return output;
}

void Trb3dataReader::processTimingSubEvent(hadaq::RawSubevent * subEvent, unsigned subEventSize, std::vector<std::pair<unsigned, double>> * extractedData)
{
    unsigned ix = 13;  // Alberto sais this offset is fixed

    unsigned epoch = 0;
    std::array<bool,NumTimeChannels> seenChannels; seenChannels.fill(false);

    while (ix < subEventSize) // loop over subsubevents
    {
        unsigned hadata = subEvent->Data(ix++);
        //qDebug() << "--> " << QString::number(hadata, 16) << QString::number(hadata, 2);

        if ( (hadata >> 28) == 6)  // epoc records start from 0b011
        {
            epoch = hadata & 0x0fffffff;
            qDebug() << "  Epoch updated:" << QString::number(epoch, 16);
        }
        else if (hadata & 0x80000000) // timedata records start from 0b1
        {
            //hadata = 0x8059bd26;
            const unsigned coarse  = hadata & 0x000007ff;    // 10-0
            const unsigned fine    = (hadata >> 12) & 0x3ff; // 21-12
            const unsigned channel = (hadata >> 22) & 0x7f;  // 28-22
            if (channel >= NumTimeChannels)
            {
                qCritical() << "Bad channel number:" << channel;
                exit(222);
            }
            qDebug() << "  Data->" << "Channel:" << channel << "  Coarse:" << coarse << "  Fine:" << fine;
            if (!seenChannels[channel])
            {
                seenChannels[channel] = true;

                const double timeFromFine  = FineSpan_ns * fine / 0x400;   // 5ps resolution?
                const double timeFromCorse = FineSpan_ns * coarse;
                const double timeFromEpoch = FineSpan_ns * 0x800 * epoch;
                const double time = timeFromEpoch + timeFromCorse + timeFromFine; // ns
                qDebug() << "Time contributions (ns) from fine, corse and epoc:" << timeFromFine << timeFromCorse << timeFromEpoch << " Global:" << time << "ns";

                if (extractedData) extractedData->push_back( {channel,time} );
            }
            //else this channel appears more than once -> ignore
        }
        else if (hadata == 0x15555) break;
    }
}


// num chan = 0 for a block?
void Trb3dataReader::readRawData(const QString &FileName, int enforceNumChannels, int enforceNumSamples)
{
    waveData.clear();
    timeData.clear();

    numChannels = enforceNumChannels;
    numSamples = enforceNumSamples;
    numBadEvents = 0;
    numAllEvents = 0;
    bool bReportOnStart = true;

    hadaq::ReadoutHandle ref = hadaq::ReadoutHandle::Connect(FileName.toLocal8Bit().data());
    hadaq::RawEvent * evnt = nullptr;

    QVector < QVector <float> > thisEventData;  //format: [channel] [sample]
    while ( (evnt = ref.NextEvent(1.0)) )
    {
        bool bBadEvent = false;
        thisEventData.clear();

        // loop over boards
        hadaq::RawSubevent * sub = nullptr;
        std::vector<std::pair<unsigned,double>> timing;
        while ( (sub=evnt->NextSubevent(sub)) )
        {
            const int boardID = sub->GetId();
            qDebug() << "==>BoardId: " + QString::number(boardID, 16);// + "Decoding:" + QString::number(sub->GetDecoding(), 16);

            const unsigned trbSubEvSize = sub->GetSize() / 4 - 4;

            if (Config->isTimerBoard(boardID))
            {
                timing.clear();
                processTimingSubEvent(sub, trbSubEvSize, &timing);
            }
            if (!Config->isADCboard(boardID)) continue;

            const unsigned lastRec = sub->Data(trbSubEvSize-3);
            qDebug() << "--->Last data record" << QString::number(lastRec, 16);
            const unsigned lastId   = (lastRec >> 16) & 0xFFFF;
            const unsigned lastChan = lastId & 0xF;
            const unsigned lastAdc  = (lastId >> 4) & 0xF;

            const unsigned numChan = (lastChan+1) * (lastAdc+1);
            if (numChan == 0)
            {
                qDebug() << "Found event with 0 number of channels for the board: " << boardID;
                ClearData();
                bBadEvent = true;
                break;
            }

            int samples = (trbSubEvSize-2) / numChan;
            if (numSamples != 0)
            {
                if (samples != numSamples)
                {
                    bBadEvent = true;
                    if (numBadEvents<50) qDebug() << "----- Event #" << waveData.size()-1 << " has wrong number of samples ("<< samples <<")\n";
                    break;
                }
            }
            else numSamples = samples;

            // resize the vectors for the waveforms and fill the data
            int oldSize = thisEventData.size();
            thisEventData.resize( oldSize + numChan );

            unsigned ix = 0;
            // loop over data
            for (unsigned iChannel = 0; iChannel < numChan; iChannel++)
            {
                unsigned adcRead = iChannel / 4;
                unsigned adcChan = 3 - iChannel % 4; // inverse order for 4 channels (0, 1, 2 and 3)
                unsigned trueChannel = 4 * adcReadToTrue[adcRead] + adcChan;
                //qDebug() << iChannel << trueChannel;
                //thisEventData[oldSize + iChannel].resize(samples);
                thisEventData[oldSize + trueChannel].resize(samples);
                for (int iSample = 0; iSample < samples; iSample++)
                {
//                    unsigned hadata = sub->Data(ix++);
//                    unsigned id   = (hadata >> 16) & 0xFFFF;
//                    unsigned data = hadata & 0xFFFF;
//                    if (data == 0x5555 && id == 1) break;
//                    unsigned chan = id & 0xF;
//                    unsigned adc  = (id >> 4) & 0xF;
                    //thisEventData[oldSize + iChannel][iSample] = (sub->Data(ix) & 0xFFFF);
                    thisEventData[oldSize + trueChannel][iSample] = (sub->Data(ix) & 0xFFFF);
                    ix++;

                    if (ix >= trbSubEvSize)
                    {
                        qDebug() << "Something is wrong with data extrtaction;  Event:" << waveData.size() << "Board:"<<boardID;
                        bBadEvent = true;
                        break;
                    }
                }
            }
        }

        // Checking this event
        const int foundChannels = thisEventData.size();
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
            timeData.push_back(timing);
            //qDebug() << "New data size: "<<data.size();
        }
        bReportOnStart = false;
        numAllEvents++;
    }

    ref.Disconnect();

    qDebug() << "\n--> Data read completed";
    qDebug() << "--> Events in the file:"<<numAllEvents;
    qDebug() << "--> Events with data: "<< waveData.size();
    qDebug() <<"   Channels: "<<numChannels << "  Samples: "<<numSamples;
    if (numBadEvents > 0) qDebug() << "--> " << numBadEvents << " bad events were disreguarded!";
}


/*
                        if (numSamples != 0)
                        {
                            if (samples != numSamples)
                            {
                                bBadEvent = true;
                                //if (numBadEvents<50) qDebug() << "----- Event #" << waveData.size()-1 << " has wrong number of samples ("<< samples <<")\n";
                            }
                        }
                        else numSamples = samples;
*/


/*
void Trb3dataReader::readRawData(const QString &FileName, int enforceNumChannels, int enforceNumSamples)
{
    waveData.clear();

    numChannels = enforceNumChannels;
    numSamples = enforceNumSamples;
    numBadEvents = 0;
    numAllEvents = 0;
    bool bReportOnStart = true;

    hadaq::ReadoutHandle ref = hadaq::ReadoutHandle::Connect(FileName.toLocal8Bit().data());
    hadaq::RawEvent * evnt = nullptr;

    while ( (evnt = ref.NextEvent(1.0)) )
    {
        bool bBadEvent = false;
        int foundChannels = 0;

        QVector < QVector <float> > thisEventData;  //format: [channel] [sample]

        // loop over sections
        hadaq::RawSubevent * sub = nullptr;
        while ( (sub=evnt->NextSubevent(sub)) )
        {
            unsigned trbSubEvSize = sub->GetSize() / 4 - 4;
            unsigned ix = 0;

            while (ix < trbSubEvSize)
            { // loop over subsubevents

                unsigned hadata = sub->Data(ix++);

                unsigned datalen = (hadata >> 16) & 0xFFFF;
                int datakind = hadata & 0xFFFF;

                if (bReportOnStart) qDebug() << "Data block with datakind: 0x" + QString::number(datakind, 16);

                unsigned ixTmp = ix;  // --->  position before read

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

                        int samples = datalen / channels;
                        if (bReportOnStart) qDebug() << "--> This is an ADC block. Channels: "<<channels<<"   Samples: "<< samples;


                        // resize the vectors for the waveforms and fill the data
                        int oldSize = thisEventData.size();
                        thisEventData.resize( oldSize + channels );
                        for (int iChannel = 0; iChannel < channels; iChannel++)
                        {
               // this is the block:

                            thisEventData[oldSize + iChannel].resize(samples);
                            for (int iSample = 0; iSample < samples; iSample++)
                            {
                                thisEventData[oldSize + iChannel][iSample] = (sub->Data(ix) & 0xFFFF);
                                ix++;
                            }

              // end
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
                    else
                    {
                        //if (bReportOnStart)
                        qDebug() << "==> This is an ADC block. Error: number of channels is 0!";
                    }

                }

                ix = ixTmp + datalen; // <--- position before data read + datalength
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

    qDebug() << "\n--> Data read completed";
    qDebug() << "--> Events in the file:"<<numAllEvents;
    qDebug() << "--> Events with data: "<< waveData.size();
    qDebug() <<"   Channels: "<<numChannels << "  Samples: "<<numSamples;
    if (numBadEvents > 0) qDebug() << "--> " << numBadEvents << " bad events were disreguarded!";
}
*/

QString Trb3dataReader::Read(const QString& FileName)
{
    qDebug() << "--> Reading hld file...";
    readRawData(FileName, Config->HldProcessSettings.NumChannels, Config->HldProcessSettings.NumSamples);

    if ( Config->HldProcessSettings.NumChannels != 0)
    {
        if ( Config->HldProcessSettings.NumChannels != numChannels )
        {
            waveData.clear();
            timeData.clear();
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

#include "TSpectrum.h"
void Trb3dataReader::substractPedestals()
{
    for (int ievent=0; ievent<waveData.size(); ievent++)
        for (int ichannel=0; ichannel<numChannels; ichannel++)
        {
            float pedestal = 0;

            switch (Config->PedestalExtractionMethod)
            {
            case 0:
                for (int isample = Config->PedestalFrom; isample <= Config->PedestalTo; isample++)
                    pedestal += waveData.at(ievent).at(ichannel).at(isample);
                pedestal /= ( Config->PedestalTo + 1 - Config->PedestalFrom );
                break;
            case 1:

/*
            TH1 *hist;

            //const QVector<double> APeakFinder::findPeaks(const double sigma, const double threshold, const int MaxNumberOfPeaks, bool SuppressDraw) const
    TSpectrum *s = new TSpectrum(MaxNumberOfPeaks);

    int numPeaks = s->Search(H, sigma, (SuppressDraw ? "goff nodraw" : ""), threshold);

#if ROOT_VERSION_CODE > ROOT_VERSION(6,0,0)
    double *pos = s->GetPositionX();
#else
    float *pos = s->GetPositionX();
#endif

    QVector<double> peaks;
    for (int i=0; i<numPeaks; i++) peaks << pos[i];
*/

                break;
            default:
                qDebug() << "Invalid pedestal extraction method index: "<< Config->PedestalExtractionMethod;
                throw std::invalid_argument( "invalid pedestal extraction method index" );
                break;
            }

            for (int isample = 0; isample < numSamples; isample++)
                waveData[ievent][ichannel][isample] -= pedestal;
        }
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

            if (Config->bTrapezoidal)
                applyTrapezoidal(waveData[ievent][ichannel], Config->TrapezoidalL, Config->TrapezoidalG);
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

void Trb3dataReader::applyTrapezoidal(QVector<float> & arr, int L, int G) const
{
    QVector<float> arrOriginal = arr;
    const int iSamMax = numSamples - 2*L - G;
    for (int iSam = 0; iSam < iSamMax; iSam++)
    {
        float av1 = 0;
        float av2 = 0;
        for (int i = 0; i < L; i++)
        {
            av1 += arrOriginal[iSam + i];
            av2 += arrOriginal[iSam + i + L + G];
        }
        arr[iSam] = (av2 - av1) / L;
    }

    for (int iSam = iSamMax; iSam < numSamples; iSam++)
        arr[iSam] = arr[iSamMax-1];
}

void Trb3dataReader::ClearData()
{
    waveData.clear();
    timeData.clear();
    numChannels = 0;
    numSamples = 0;
    numBadEvents = 0;
    numAllEvents = 0;
}
