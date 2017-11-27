#include "ainterfacetodata.h"
#include "adatahub.h"

#include <QJsonArray>

#include <limits>
const double NaN = std::numeric_limits<double>::quiet_NaN();

AInterfaceToData::AInterfaceToData(ADataHub* DataHub) :
    DataHub(DataHub)
{

}

int AInterfaceToData::countEvents()
{
    return DataHub->CountEvents();
}

int AInterfaceToData::countChannels()
{
    return DataHub->CountChannels();
}

float AInterfaceToData::getSignal(int ievent, int iLogicalChannel)
{
    return DataHub->GetSignal(ievent, iLogicalChannel);
}

float AInterfaceToData::getSignalFast(int ievent, int iLogicalChannel)
{
    return DataHub->GetSignalFast(ievent, iLogicalChannel);
}

QVariant AInterfaceToData::getSignals(int ievent)
{
    const QVector<float>* vec = DataHub->GetSignals(ievent);
    if (!vec) return QVariantList();

    QJsonArray ar;
    for (float val : *vec) ar << val;
    QJsonValue jv = ar;
    return jv.toVariant();
}

QVariant AInterfaceToData::getSignalsFast(int ievent)
{
    const QVector<float>* vec = DataHub->GetSignalsFast(ievent);

    QJsonArray ar;
    for (float val : *vec) ar << val;
    QJsonValue jv = ar;
    return jv.toVariant();
}

void AInterfaceToData::setSignal(int ievent, int iLogicalChannel, float value)
{
    bool bOK = DataHub->SetSignal(ievent, iLogicalChannel, value);
    if (!bOK) abort("Failed to set signal value - wrong arguments");
}

void AInterfaceToData::setSignalFast(int ievent, int iLogicalChannel, float value)
{
    DataHub->SetSignalFast(ievent, iLogicalChannel, value);
}

void AInterfaceToData::setSignals(int ievent, QVariant arrayOfValues)
{
    QString type = arrayOfValues.typeName();
    if (type != "QVariantList")
    {
        abort("Failed to set signal values - need array");
        return;
    }

    QVariantList vl = arrayOfValues.toList();
    QJsonArray ar = QJsonArray::fromVariantList(vl);
//    const int numChannels = DataHub->CountChannels();
//    if (ar.size() != numChannels)
//    {
//        abort("Failed to set signal values - array size mismatch");
//        return;
//    }

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

    bool bOK = DataHub->SetSignals(ievent, &vec);
    if (!bOK)
    {
        abort("Failed to set signal values - wrong event number");
    }
}

void AInterfaceToData::setSignalsFast(int ievent, QVariant arrayOfValues)
{
    QVariantList vl = arrayOfValues.toList();
    QJsonArray ar = QJsonArray::fromVariantList(vl);

    QVector<float> vec;
    vec.reserve(ar.size());
    for (int i=0; i<ar.size(); ++i) vec << ar[i].toDouble();

    DataHub->SetSignalsFast(ievent, &vec);
}

bool AInterfaceToData::isRejectedEvent(int ievent)
{
    return DataHub->IsRejected(ievent);
}

bool AInterfaceToData::isRejectedEventFast(int ievent)
{
    return DataHub->IsRejectedFast(ievent);
}

void AInterfaceToData::setRejected(int ievent, bool flag)
{
    bool bOK = DataHub->SetRejectedFlag(ievent, flag);
    if (!bOK)
        abort("Failed to modify rejected status for event #"+QString::number(ievent));
}

void AInterfaceToData::setRejectedFast(int ievent, bool flag)
{
    DataHub->SetRejectedFlagFast(ievent, flag);
}

void AInterfaceToData::setAllRejected(bool flag)
{
    DataHub->SetAllRejectedFlag(flag);
}

void AInterfaceToData::Clear()
{
    DataHub->Clear();
}
