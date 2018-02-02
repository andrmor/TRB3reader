#include "ainterfacetodata.h"
#include "adatahub.h"
#include "masterconfig.h"

#include <QJsonArray>

#include <limits>
const double NaN = std::numeric_limits<double>::quiet_NaN();

AInterfaceToData::AInterfaceToData(ADataHub* DataHub) :
    DataHub(DataHub)
{

}

int AInterfaceToData::countEvents() const
{
    return DataHub->CountEvents();
}

int AInterfaceToData::countChannels() const
{
    return DataHub->CountChannels();
}

void AInterfaceToData::Clear()
{
    DataHub->Clear();
}

void AInterfaceToData::addEvent(const QVariant signalArray)
{
    QString type = signalArray.typeName();
    if (type != "QVariantList")
    {
        abort("Failed to add event - argument should be an array with signal values");
        return;
    }

    QVariantList vl = signalArray.toList();
    QJsonArray ar = QJsonArray::fromVariantList(vl);
    const int numChannels = DataHub->getConfig().CountLogicalChannels();
    if (ar.size() != numChannels)
        {
            abort("Failed to add event: signal array size is not equal to the number of logical channels!");
            return;
        }

    AOneEvent* ev = new AOneEvent();
    ev->SetRejectedFlag(false);
    DataHub->AddEventFast(ev);
    setSignals(DataHub->CountEvents()-1, signalArray); // size is checked there, abort if wrong
}

float AInterfaceToData::getSignal(int ievent, int iLogicalChannel) const
{
    return DataHub->GetSignal(ievent, iLogicalChannel);
}

float AInterfaceToData::getSignalFast(int ievent, int iLogicalChannel) const
{
    return DataHub->GetSignalFast(ievent, iLogicalChannel);
}

const QVariant AInterfaceToData::getSignals(int ievent) const
{
    const QVector<float>* vec = DataHub->GetSignals(ievent);
    if (!vec) return QVariantList();

    QJsonArray ar;
    for (float val : *vec) ar << val;
    QJsonValue jv = ar;
    return jv.toVariant();
}

const QVariant AInterfaceToData::getSignalsFast(int ievent) const
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

void AInterfaceToData::setSignals(int ievent, const QVariant arrayOfValues)
{
    QString type = arrayOfValues.typeName();
    if (type != "QVariantList")
    {
        abort("Failed to set signal values - need array");
        return;
    }

    QVariantList vl = arrayOfValues.toList();
    QJsonArray ar = QJsonArray::fromVariantList(vl);
    //const int numChannels = DataHub->CountChannels();
    const int numChannels = DataHub->getConfig().CountLogicalChannels();
    if (ar.size() != numChannels)       
        {
            abort("Failed to set signal values - array size mismatch");
            return;
        }

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

void AInterfaceToData::setSignalsFast(int ievent, const QVariant arrayOfValues)
{
    QVariantList vl = arrayOfValues.toList();
    QJsonArray ar = QJsonArray::fromVariantList(vl);

    QVector<float> vec;
    vec.reserve(ar.size());
    for (int i=0; i<ar.size(); ++i) vec << ar[i].toDouble();

    DataHub->SetSignalsFast(ievent, &vec);
}

bool AInterfaceToData::isRejectedEvent(int ievent) const
{
    return DataHub->IsRejected(ievent);
}

bool AInterfaceToData::isRejectedEventFast(int ievent) const
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

const QVariant AInterfaceToData::getPosition(int ievent) const
{
    const float* pos = DataHub->GetPosition(ievent);
    if (!pos)
    {
        abort("Get position: bad event number "+QString::number(ievent));
        return QVariantList();
    }

    QJsonArray ar;
    for (int i=0; i<3; i++) ar << pos[i];
    QJsonValue jv = ar;
    return jv.toVariant();
}

const QVariant AInterfaceToData::getPositionFast(int ievent) const
{
    const float* pos = DataHub->GetPosition(ievent);

    QJsonArray ar;
    for (int i=0; i<3; i++) ar << pos[i];
    QJsonValue jv = ar;
    return jv.toVariant();
}

void AInterfaceToData::setPosition(int ievent, float x, float y, float z)
{
    bool bOK = DataHub->SetPosition(ievent, x, y, z);
    if (!bOK)
    {
        abort("Bad event number in position set");
    }
}

void AInterfaceToData::setPositionFast(int ievent, float x, float y, float z)
{
    DataHub->SetPositionFast(ievent, x, y, z);
}

const QVariant AInterfaceToData::getWaveforms(int ievent)
{
    const QVector< QVector<float>* >* vec = DataHub->GetWaveforms(ievent);
    if (!vec)
    {
        abort("Cannot get waveforms for event: "+QString::number(ievent));
        return QVariantList();
    }

    QJsonArray ar;
    for (int ich=0; ich<vec->size(); ich++)
    {
        QJsonArray arEl;
        const QVector<float>* wave = vec->at(ich);
        if (wave)
            for (int isam=0; isam<wave->size(); isam++) arEl << wave->at(isam);
        ar.append(arEl);
    }
    QJsonValue jv = ar;
    return jv.toVariant();
}

float AInterfaceToData::getWaveformMax(int ievent, int ichannel) const
{
    float val = DataHub->GetWaveformMax(ievent, ichannel);
    if (std::isnan(val))
        abort("Error in getWaveformMax");
    return val;
}

float AInterfaceToData::getWaveformMin(int ievent, int ichannel) const
{
    float val = DataHub->GetWaveformMin(ievent, ichannel);
    if (std::isnan(val))
        abort("Error in getWaveformMin");
    return val;
}

int AInterfaceToData::getWaveformMaxSample(int ievent, int ichannel) const
{
    int val = DataHub->GetWaveformMaxSample(ievent, ichannel);
    if (val < 0)
        abort("Error in getWaveformMaxSample");
    return val;
}

int AInterfaceToData::getWaveformMinSample(int ievent, int ichannel) const
{
    int val = DataHub->GetWaveformMinSample(ievent, ichannel);
    if (val < 0)
        abort("Error in getWaveformMinSample");
    return val;
}

int AInterfaceToData::getWaveformSampleWhereFirstBelow(int ievent, int ichannel, float threshold) const
{
    int val = DataHub->GetWaveformSampleWhereFirstBelow(ievent, ichannel, threshold);
    if (val < 0)
        abort("Error in getWaveformSampleWhereFirstBelow");
    return val;
}

int AInterfaceToData::getWaveformSampleWhereFirstAbove(int ievent, int ichannel, float threshold) const
{
    int val = DataHub->GetWaveformSampleWhereFirstAbove(ievent, ichannel, threshold);
    if (val < 0)
        abort("Error in getWaveformSampleWhereFirstAbove");
    return val;
}

void AInterfaceToData::setMultiplicity(int ievent, QVariant px_py_pz_nx_ny_nz)
{
    QString type = px_py_pz_nx_ny_nz.typeName();
    if (type != "QVariantList")
    {
        abort("Failed to set multiplicities - need array of 6 integers");
        return;
    }

    QVariantList vl = px_py_pz_nx_ny_nz.toList();
    QJsonArray ja = QJsonArray::fromVariantList(vl);
    if (ja.size() != 6)
    {
        abort("Set multiplicities - require array of multPosX, multPosY, multPosZ, multNegX, multNegY, multNegZ");
        return;
    }
    int arrPos[3];
    int arrNeg[3];
    for (int i=0; i<3; i++)
    {
        if (!ja[i].isDouble())  // ***!!! need isInt, but not available in this Qt version!
        {
            abort("Set multiplicities - require array of six integers");
            return;
        }
        arrPos[i] = ja[i].toInt();
    }
    for (int i=3; i<6; i++)
    {
        if (!ja[i].isDouble())  // ***!!! need isInt, but not available in this Qt version!
        {
            abort("Set multiplicities - require array of six integers");
            return;
        }
        arrNeg[i-3] = ja[i].toInt();
    }

    bool bOK = DataHub->SetMultiplicityPositive(ievent, arrPos);
    if (!bOK)
    {
        abort("failed to set multiplicity for event " + QString::number(ievent));
        return;
    }
    bOK = DataHub->SetMultiplicityNegative(ievent, arrNeg);
    if (!bOK)
    {
        abort("failed to set multiplicity for event " + QString::number(ievent));
        return;
    }
}

void AInterfaceToData::setMultiplicityFast(int ievent, QVariant px_py_pz_nx_ny_nz)
{
    QVariantList vl = px_py_pz_nx_ny_nz.toList();
    QJsonArray ja = QJsonArray::fromVariantList(vl);
    int arrPos[3];
    int arrNeg[3];
    for (int i=0; i<3; i++) arrPos[i]   = ja[i].toInt();
    for (int i=3; i<6; i++) arrNeg[i-3] = ja[i].toInt();

    DataHub->SetMultiplicityPositiveFast(ievent, arrPos);
    DataHub->SetMultiplicityNegativeFast(ievent, arrNeg);
}

const QVariant AInterfaceToData::getMultiplicity(int ievent) const
{
    const int* multPos = DataHub->GetMultiplicityPositive(ievent);
    if (!multPos)
    {
        abort("Get position: bad event number "+QString::number(ievent));
        return QVariantList();
    }
    const int* multNeg = DataHub->GetMultiplicityNegativeFast(ievent);

    QJsonArray ar;
    for (int i=0; i<3; i++) ar << multPos[i];
    for (int i=0; i<3; i++) ar << multNeg[i];
    QJsonValue jv = ar;
    return jv.toVariant();
}

const QVariant AInterfaceToData::getMultiplicityFast(int ievent) const
{
    const int* multPos = DataHub->GetMultiplicityPositiveFast(ievent);
    const int* multNeg = DataHub->GetMultiplicityNegativeFast(ievent);

    QJsonArray ar;
    for (int i=0; i<3; i++) ar << multPos[i];
    for (int i=0; i<3; i++) ar << multNeg[i];
    QJsonValue jv = ar;
    return jv.toVariant();
}

void AInterfaceToData::setSumSignals(int ievent, QVariant px_py_pz_nx_ny_nz)
{
    QString type = px_py_pz_nx_ny_nz.typeName();
    if (type != "QVariantList")
    {
        abort("Failed to set sum signals - need array of 6 real numbers");
        return;
    }

    QVariantList vl = px_py_pz_nx_ny_nz.toList();
    QJsonArray ja = QJsonArray::fromVariantList(vl);
    if (ja.size() != 6)
    {
        abort("Set sum signals - require array of sumsigPosX, sumsigPosY, sumsigPosZ, sumsigNegX, sumsigNegY, sumsigNegZ");
        return;
    }
    float arrPos[3];
    float arrNeg[3];
    for (int i=0; i<3; i++)
    {
        if (!ja[i].isDouble())
        {
            abort("Set sumsignals - require array of six real numbers");
            return;
        }
        arrPos[i] = ja[i].toDouble();
    }
    for (int i=3; i<6; i++)
    {
        if (!ja[i].isDouble())
        {
            abort("Set sumsignals - require array of six real numbers");
            return;
        }
        arrNeg[i-3] = ja[i].toDouble();
    }

    bool bOK = DataHub->SetSumSignalPositive(ievent, arrPos);
    if (!bOK)
    {
        abort("failed to set sumsignals for event " + QString::number(ievent));
        return;
    }
    bOK = DataHub->SetSumSignalNegative(ievent, arrNeg);
    if (!bOK)
    {
        abort("failed to set sumsignals for event " + QString::number(ievent));
        return;
    }
}

void AInterfaceToData::setSumSignalsFast(int ievent, QVariant px_py_pz_nx_ny_nz)
{
    QVariantList vl = px_py_pz_nx_ny_nz.toList();
    QJsonArray ja = QJsonArray::fromVariantList(vl);
    float arrPos[3];
    float arrNeg[3];
    for (int i=0; i<3; i++) arrPos[i]   = ja[i].toDouble();
    for (int i=3; i<6; i++) arrNeg[i-3] = ja[i].toDouble();

    DataHub->SetSumSignalPositiveFast(ievent, arrPos);
    DataHub->SetSumSignalNegativeFast(ievent, arrNeg);
}

const QVariant AInterfaceToData::getSumSignals(int ievent) const
{
    const float* sumPos = DataHub->GetSumSignalPositive(ievent);
    if (!sumPos)
    {
        abort("Get sum signals: bad event number "+QString::number(ievent));
        return QVariantList();
    }
    const float* sumNeg = DataHub->GetSumSignalNegativeFast(ievent);

    QJsonArray ar;
    for (int i=0; i<3; i++) ar << sumPos[i];
    for (int i=0; i<3; i++) ar << sumNeg[i];
    QJsonValue jv = ar;
    return jv.toVariant();
}

const QVariant AInterfaceToData::getSumSignalsFast(int ievent) const
{
    const float* sumPos = DataHub->GetSumSignalPositiveFast(ievent);
    const float* sumNeg = DataHub->GetSumSignalNegativeFast(ievent);

    QJsonArray ar;
    for (int i=0; i<3; i++) ar << sumPos[i];
    for (int i=0; i<3; i++) ar << sumNeg[i];
    QJsonValue jv = ar;
    return jv.toVariant();
}

void AInterfaceToData::save(const QString& FileName, bool bSavePositions, bool bSkipRejected) const
{
    const QString ErrStr = DataHub->Save(FileName, bSavePositions, bSkipRejected);

    if (!ErrStr.isEmpty()) abort(ErrStr);
}

void AInterfaceToData::load(const QString &AppendFromFileName, bool bLoadPositionXYZ)
{
    const QString ErrStr = DataHub->Load(AppendFromFileName, bLoadPositionXYZ);

    if (!ErrStr.isEmpty()) abort(ErrStr);
}

