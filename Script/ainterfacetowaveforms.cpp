#include "ainterfacetowaveforms.h"
#include "trb3datareader.h"
#include "channelmapper.h"

#include <QJsonArray>
#include <QJsonValue>

AInterfaceToWaveforms::AInterfaceToWaveforms(Trb3dataReader *Reader, ChannelMapper *Map) :
    Reader(Reader), Map(Map) {}

double AInterfaceToWaveforms::getValue_hardware(int ievent, int ichannel, int isample)
{
    int val = Reader->GetValue(ievent, ichannel, isample);
    if (std::isnan(val)) abort("Faled to get waveform value!");

    return val;
}

double AInterfaceToWaveforms::getValueFast_hardware(int ievent, int ichannel, int isample)
{
    return Reader->GetValueFast(ievent, ichannel, isample);
}

QVariant AInterfaceToWaveforms::getWaveform_hardware(int ievent, int ichannel)
{
    const std::vector<int> *wave = Reader->GetWaveformPtr(ievent, ichannel);
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

QVariant AInterfaceToWaveforms::getWaveformFast_hardware(int ievent, int ichannel)
{
    const std::vector<int> *wave = Reader->GetWaveformPtrFast(ievent, ichannel);

    QJsonArray ar;
    for (int val : *wave) ar << val;
    QJsonValue jv = ar;
    return jv.toVariant();
}

QVariant AInterfaceToWaveforms::getWaveform_logical(int ievent, int ichannel)
{
    int ihardw = Map->LogicalToHardware(ichannel);
    if (std::isnan(ihardw))
    {
        abort("Invalid channel number in get waveform");
        return QVariant();
    }

    return getWaveform_hardware(ievent, ihardw);
}

QVariant AInterfaceToWaveforms::getWaveformFast_logical(int ievent, int ichannel)
{
    return getWaveformFast_hardware(ievent, Map->LogicalToHardware(ichannel));
}

