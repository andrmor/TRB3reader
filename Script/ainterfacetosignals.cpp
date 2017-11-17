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

    QJsonArray ar;
    const int numLog = Map->GetNumLogicalChannels();
    for (int ilog=0; ilog<numLog; ++ilog) ar << vec->at(Map->LogicalToHardwareFast(ilog));
    QJsonValue jv = ar;
    return jv.toVariant();
}
