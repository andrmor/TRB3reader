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

int AInterfaceToWaveforms::countSamples() const
{
    if (!Reader->isValid()) return 0;

    return Reader->CountSamples();
}

float AInterfaceToWaveforms::getValue(int ievent, int iHardwChannel, int isample) const
{
    float val = Reader->GetValue(ievent, iHardwChannel, isample);
    if ( std::isnan(val) ) abort("Failed to get waveform value!");

    return val;
}

float AInterfaceToWaveforms::getValueFast(int ievent, int iHardwChannel, int isample) const
{
    return Reader->GetValueFast(ievent, iHardwChannel, isample);
}

void AInterfaceToWaveforms::setValue(int ievent, int iHardwChannel, int isample, float value)
{
    bool bOK = Reader->SetValue(ievent, iHardwChannel, isample, value);
    if (!bOK)
        abort("Failed to set waveform value");
}

void AInterfaceToWaveforms::setValueFast(int ievent, int iHardwChannel, int isample, float value)
{
    Reader->SetValueFast(ievent, iHardwChannel, isample, value);
}

const QVariant AInterfaceToWaveforms::getWaveform(int ievent, int iHardwChannel) const
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

const QVariant AInterfaceToWaveforms::getWaveformFast(int ievent, int iHardwChannel) const
{
    const QVector<float> *wave = Reader->GetWaveformPtrFast(ievent, iHardwChannel);

    QJsonArray ar;
    for (int val : *wave) ar << val;
    QJsonValue jv = ar;
    return jv.toVariant();
}

void AInterfaceToWaveforms::setWaveform(int ievent, int ichannel, const QVariant array)
{
    QString type = array.typeName();
    if (type != "QVariantList")
    {
        abort("Failed to set waveform - need array as argument");
        return;
    }

    QVariantList vl = array.toList();
    QJsonArray ar = QJsonArray::fromVariantList(vl);

    QVector<float> vec;
    vec.reserve(ar.size());
    for (int i=0; i<ar.size(); ++i)
    {
        if (!ar[i].isDouble())
        {
            abort("Failed to set signal values - array contains non-numerical data");
            return;
        }
        vec << ar[i].toDouble();
    }

    bool bOK = Reader->SetWaveform(ievent, ichannel, vec);
    if (!bOK)
        abort("Failed to set waveform - check event/channel and array size are valid");
}

void AInterfaceToWaveforms::setWaveformFast(int ievent, int ichannel, const QVariant array)
{
    QVariantList vl = array.toList();
    QJsonArray ar = QJsonArray::fromVariantList(vl);

    QVector<float> vec;
    vec.reserve(ar.size());
    for (int i=0; i<ar.size(); ++i) vec << ar[i].toDouble();

    Reader->SetWaveformFast(ievent, ichannel, vec);
}

float AInterfaceToWaveforms::getMax(int ievent, int iHardwChannel) const
{
    float val = Reader->GetMax(ievent, iHardwChannel);
    if ( std::isnan(val) ) abort("Failed to get waveform value!");

    return val;
}

float AInterfaceToWaveforms::getMaxFast(int ievent, int iHardwChannel) const
{
    return Reader->GetMaxFast(ievent, iHardwChannel);
}

float AInterfaceToWaveforms::getMin(int ievent, int iHardwChannel) const
{
    float val = Reader->GetMin(ievent, iHardwChannel);
    if ( std::isnan(val) ) abort("Failed to get waveform value!");

    return val;
}

float AInterfaceToWaveforms::getMinFast(int ievent, int iHardwChannel) const
{
    return Reader->GetMinFast(ievent, iHardwChannel);
}

int AInterfaceToWaveforms::getMaxSample(int ievent, int iHardwChannel) const
{
    int val = Reader->GetMaxSample(ievent, iHardwChannel);
    if (val < 0) abort("Failed to get sample number!");

    return val;
}

int AInterfaceToWaveforms::getMaxSampleFast(int ievent, int iHardwChannel) const
{
    return Reader->GetMaxSampleFast(ievent, iHardwChannel);
}

int AInterfaceToWaveforms::getMinSample(int ievent, int iHardwChannel) const
{
    int val = Reader->GetMinSample(ievent, iHardwChannel);
    if (val < 0) abort("Failed to get sample number!");

    return val;
}

int AInterfaceToWaveforms::getMinSampleFast(int ievent, int iHardwChannel) const
{
    return Reader->GetMinSampleFast(ievent, iHardwChannel);
}

int AInterfaceToWaveforms::getSampleWhereFirstAbove(int ievent, int iHardwChannel, int threshold) const
{
    int isample = Reader->GetSampleWhereFirstAbove(ievent, iHardwChannel, threshold);
    if (isample < 0) abort("Failed to get sample number!");

    return isample;
}

int AInterfaceToWaveforms::getSampleWhereFirstAboveFast(int ievent, int iHardwChannel, int threshold) const
{
    return Reader->GetSampleWhereFirstAboveFast(ievent, iHardwChannel, threshold);
}

int AInterfaceToWaveforms::getSampleWhereFirstBelow(int ievent, int iHardwChannel, int threshold) const
{
    int isample = Reader->GetSampleWhereFirstBelow(ievent, iHardwChannel, threshold);
    if (isample < 0) abort("Failed to get sample number!");

    return isample;
}

int AInterfaceToWaveforms::getSampleWhereFirstBelowFast(int ievent, int iHardwChannel, int threshold) const
{
    return Reader->GetSampleWhereFirstBelowFast(ievent, iHardwChannel, threshold);
}

