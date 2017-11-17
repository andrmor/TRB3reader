#include "ainterfacetosignals.h"
#include "trb3signalextractor.h"
#include "channelmapper.h"

#include <QVariantList>
#include <QJsonArray>

#include <limits>

const size_t NaN = std::numeric_limits<size_t>::quiet_NaN();

AInterfaceToSignals::AInterfaceToSignals(Trb3signalExtractor* Extractor, ChannelMapper *Map) :
    Extractor(Extractor), Map(Map)
{

}

int AInterfaceToSignals::countEvents()
{
    return Extractor->GetNumEvents();
}

int AInterfaceToSignals::countHardwareChannels()
{
    return Extractor->GetNumChannels();
}

int AInterfaceToSignals::countLogicalChannels()
{
    return Map->GetNumLogicalChannels();
}

double AInterfaceToSignals::getSignal_hardware(int ievent, int ichannel)
{
    return Extractor->GetSignal(ievent, ichannel);
}

QVariant AInterfaceToSignals::getSignals_hardware(int ievent)
{
    const std::vector<double>* vec = Extractor->GetSignals(ievent);
    if (!vec) return QVariantList();

    QJsonArray ar;
    for (double val : *vec) ar << val;
    QJsonValue jv = ar;
    return jv.toVariant();
}

double AInterfaceToSignals::getSignal_logical(int ievent, int ichannel)
{
    size_t ihardw = Map->LogicalToHardware(ichannel);
    if (std::isnan(ihardw)) return NaN;

    return Extractor->GetSignal(ievent, ihardw);
}

QVariant AInterfaceToSignals::getSignals_logical(int ievent)
{
    const std::vector<double>* vec = Extractor->GetSignals(ievent);
    if (!vec) return QVariantList();

    const std::vector<std::size_t>& map = Map->GetMapToHardware();

    QJsonArray ar;
    for (int ihardw : map) ar << vec->at(ihardw);
    QJsonValue jv = ar;
    return jv.toVariant();
}

void AInterfaceToSignals::setSignal_hardware(int ievent, int ichannel, double value)
{
    bool bOK = Extractor->SetSignal(ievent, ichannel, value);
    if (!bOK) abort("Failed to set signal value - wrong arguments");
}

void AInterfaceToSignals::setSignals_hardware(int ievent, QVariant arrayOfValues)
{
    QString type = arrayOfValues.typeName();
    if (type != "QVariantList")
    {
        abort("Failed to set signal values - need array");
        return;
    }

    QVariantList vl = arrayOfValues.toList();
    QJsonArray ar = QJsonArray::fromVariantList(vl);
    const int numChannels = Extractor->GetNumChannels();
    if (ar.size() != numChannels)
    {
        abort("Failed to set signal values - array size mismatch");
        return;
    }

    std::vector<double> vec;
    vec.reserve(numChannels);
    for (int i=0; i<ar.size(); ++i)
    {
        if (!ar[i].isDouble())
        {
            abort("Failed to set signal values - array contains non-numerical data");
            return;
        }
        vec.push_back( ar[i].toDouble() );
    }

    bool bOK = Extractor->SetSignals(ievent, vec);
    if (!bOK) abort("Failed to set signal values - wrong arguments");
}

void AInterfaceToSignals::setSignal_logical(int ievent, int ichannel, double value)
{
    int ihardw = Map->LogicalToHardware(ichannel);
    if (std::isnan(ihardw))
    {
        abort("Failed to set signal of a logical channel - wrong channel number");
        return;
    }

    setSignal_hardware(ievent, ihardw, value);
}