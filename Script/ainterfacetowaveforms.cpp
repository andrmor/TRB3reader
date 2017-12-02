#include "ainterfacetowaveforms.h"
#include "trb3datareader.h"
#include "masterconfig.h"
#include "channelmapper.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>

#include <limits>

AInterfaceToWaveforms::AInterfaceToWaveforms(MasterConfig* Config, Trb3dataReader* Reader) :
    Config(Config), Reader(Reader) {}

int AInterfaceToWaveforms::countSamples()
{
    if (!Reader->isValid()) return 0;

    return Reader->CountSamples();
}

float AInterfaceToWaveforms::getValue(int ievent, int iHardwChannel, int isample)
{
    float val = Reader->GetValue(ievent, iHardwChannel, isample);
    if ( std::isnan(val) ) abort("Failed to get waveform value!");

    return val;
}

float AInterfaceToWaveforms::getValueFast(int ievent, int iHardwChannel, int isample)
{
    return Reader->GetValueFast(ievent, iHardwChannel, isample);
}

QVariant AInterfaceToWaveforms::getWaveform(int ievent, int iHardwChannel)
{
    const QVector<float> *wave = Reader->GetWaveformPtr(ievent, iHardwChannel);
    if (!wave)
    {
        abort("Failed to get waveform");
        return QVariantList();
    }

    QJsonArray ar;
    for (int val : *wave) ar << val;
    QJsonValue jv = ar;
    return jv.toVariant();
}

QVariant AInterfaceToWaveforms::getWaveformFast(int ievent, int iHardwChannel)
{
    const QVector<float> *wave = Reader->GetWaveformPtrFast(ievent, iHardwChannel);

    QJsonArray ar;
    for (int val : *wave) ar << val;
    QJsonValue jv = ar;
    return jv.toVariant();
}

float AInterfaceToWaveforms::getMax(int ievent, int iHardwChannel)
{
    float val = Reader->GetMax(ievent, iHardwChannel);
    if ( std::isnan(val) ) abort("Failed to get waveform value!");

    return val;
}

float AInterfaceToWaveforms::getMaxFast(int ievent, int iHardwChannel)
{
    return Reader->GetMaxFast(ievent, iHardwChannel);
}

float AInterfaceToWaveforms::getMin(int ievent, int iHardwChannel)
{
    float val = Reader->GetMin(ievent, iHardwChannel);
    if ( std::isnan(val) ) abort("Failed to get waveform value!");

    return val;
}

float AInterfaceToWaveforms::getMinFast(int ievent, int iHardwChannel)
{
    return Reader->GetMinFast(ievent, iHardwChannel);
}

int AInterfaceToWaveforms::getMaxSample(int ievent, int iHardwChannel)
{
    int val = Reader->GetMaxSample(ievent, iHardwChannel);
    if (val < 0) abort("Failed to get sample number!");

    return val;
}

int AInterfaceToWaveforms::getMaxSampleFast(int ievent, int iHardwChannel)
{
    return Reader->GetMaxSampleFast(ievent, iHardwChannel);
}

int AInterfaceToWaveforms::getMinSample(int ievent, int iHardwChannel)
{
    int val = Reader->GetMinSample(ievent, iHardwChannel);
    if (val < 0) abort("Failed to get sample number!");

    return val;
}

int AInterfaceToWaveforms::getMinSampleFast(int ievent, int iHardwChannel)
{
    return Reader->GetMinSampleFast(ievent, iHardwChannel);
}

int AInterfaceToWaveforms::getSampleWhereFirstAbove(int ievent, int iHardwChannel, int threshold)
{
    int isample = Reader->GetSampleWhereFirstAbove(ievent, iHardwChannel, threshold);
    if (isample < 0) abort("Failed to get sample number!");

    return isample;
}

int AInterfaceToWaveforms::getSampleWhereFirstAboveFast(int ievent, int iHardwChannel, int threshold)
{
    return Reader->GetSampleWhereFirstAboveFast(ievent, iHardwChannel, threshold);
}

int AInterfaceToWaveforms::getSampleWhereFirstBelow(int ievent, int iHardwChannel, int threshold)
{
    int isample = Reader->GetSampleWhereFirstBelow(ievent, iHardwChannel, threshold);
    if (isample < 0) abort("Failed to get sample number!");

    return isample;
}

int AInterfaceToWaveforms::getSampleWhereFirstBelowFast(int ievent, int iHardwChannel, int threshold)
{
    return Reader->GetSampleWhereFirstBelowFast(ievent, iHardwChannel, threshold);
}

