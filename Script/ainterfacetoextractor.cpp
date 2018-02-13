#include "ainterfacetoextractor.h"
#include "trb3signalextractor.h"
#include "channelmapper.h"
#include "masterconfig.h"

#include <QVariantList>
#include <QJsonArray>

AInterfaceToExtractor::AInterfaceToExtractor(MasterConfig* Config, Trb3signalExtractor* Extractor) :
    Config(Config), Extractor(Extractor)
{
    Description = "Low-elevel unit for signal extraction. Takes waveworm data from \"wav\" unit.";
}

int AInterfaceToExtractor::countEvents() const
{
    return Extractor->CountEvents();
}

int AInterfaceToExtractor::countChannels() const
{
    return Extractor->CountChannels();
}

float AInterfaceToExtractor::getSignal(int ievent, int iHardwChannel) const
{
    return Extractor->GetSignal(ievent, iHardwChannel);
}

const QVariant AInterfaceToExtractor::getSignals(int ievent) const
{
    const QVector<float>* vec = Extractor->GetSignals(ievent);
    if (!vec) return QVariantList();

    QJsonArray ar;
    for (float val : *vec) ar << val;
    QJsonValue jv = ar;
    return jv.toVariant();
}

const QVariant AInterfaceToExtractor::getSignals_logical(int ievent) const
{
    const QVector<float>* vec = Extractor->GetSignals(ievent);
    if (!vec) return QVariantList();

    const QVector<int>& map = Config->Map->GetMapToHardware();

    QJsonArray ar;
    for (int ihardw : map) ar << vec->at(ihardw);
    QJsonValue jv = ar;
    return jv.toVariant();
}

void AInterfaceToExtractor::setSignal(int ievent, int iHardwChannel, float value)
{
    bool bOK = Extractor->SetSignal(ievent, iHardwChannel, value);
    if (!bOK) abort("Failed to set signal value - wrong arguments");
}

void AInterfaceToExtractor::setSignals(int ievent, const QVariant arrayOfValues)
{
    QString type = arrayOfValues.typeName();
    if (type != "QVariantList")
    {
        abort("Failed to set signal values - need array");
        return;
    }

    QVariantList vl = arrayOfValues.toList();
    QJsonArray ar = QJsonArray::fromVariantList(vl);
    const int numChannels = Extractor->CountChannels();
    if (ar.size() != numChannels)
    {
        abort("Failed to set signal values - array size mismatch");
        return;
    }

    QVector<float> vec;
    vec.reserve(numChannels);
    for (int i=0; i<ar.size(); ++i)
    {
        if (!ar[i].isDouble())
        {
            abort("Failed to set signal values - array contains non-numerical data");
            return;
        }
        vec << ar[i].toDouble();
    }

    bool bOK = Extractor->SetSignals(ievent, vec);
    if (!bOK) abort("Failed to set signal values - wrong arguments");
}

void AInterfaceToExtractor::setAllRejected(bool flag)
{
    Extractor->SetAllRejected(flag);
}

bool AInterfaceToExtractor::isRejectedEvent(int ievent) const
{
    return Extractor->IsRejectedEvent(ievent);
}

bool AInterfaceToExtractor::isRejectedEventFast(int ievent) const
{
    return Extractor->IsRejectedEventFast(ievent);
}

void AInterfaceToExtractor::setRejected(int ievent, bool flag)
{
    bool bOK = Extractor->SetRejected(ievent, flag);
    if (!bOK)
        abort("Failed to modify rejected status for event #"+QString::number(ievent));
}
