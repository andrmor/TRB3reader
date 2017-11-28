#include "adatahub.h"

#include <limits>
const float NaN = std::numeric_limits<float>::quiet_NaN();

void ADataHub::RemoveEvent(int ievent)
{
    if (ievent<0 || ievent>=Events.size()) return;
    Events.removeAt(ievent);
}

const AOneEvent *ADataHub::GetEvent(int ievent) const
{
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent);
}

AOneEvent *ADataHub::GetEvent(int ievent)
{
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events[ievent];
}

float AOneEvent::GetSignal(int ichannel) const
{
    if (ichannel<0 || ichannel>=Signals.size()) return NaN;
    return Signals.at(ichannel);
}

bool AOneEvent::SetSignal(int ichannel, float value)
{
    if (ichannel<0 || ichannel>=Signals.size()) return false;
    Signals[ichannel] = value;
    return true;
}

void AOneEvent::ClearWaveforms()
{
    for (QVector<float>* vec : Waveforms) delete vec;
}

ADataHub::~ADataHub()
{
    Clear();
}

void ADataHub::Clear()
{
    for (AOneEvent* event : Events)
    {
        event->ClearWaveforms();
        delete event;
    }
    Events.clear();
}

int ADataHub::CountChannels()
{
    if (Events.isEmpty()) return 0;
    return Events.first()->CountChannels();
}

float ADataHub::GetSignal(int ievent, int ichannel)
{
    const AOneEvent* event = GetEvent(ievent);
    if (!event) return NaN;

    return event->GetSignal(ichannel);
}

float ADataHub::GetSignalFast(int ievent, int ichannel)
{
    return Events.at(ievent)->GetSignalFast(ichannel);
}

const QVector<float> *ADataHub::GetSignals(int ievent) const
{
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent)->GetSignals();
}

const QVector<float> *ADataHub::GetSignalsFast(int ievent) const
{
    return Events.at(ievent)->GetSignals();
}

bool ADataHub::SetSignal(int ievent, int iLogicalChannel, float value)
{
    if (ievent<0 || ievent>=Events.size()) return false;
    return Events[ievent]->SetSignal(iLogicalChannel, value);
}

void ADataHub::SetSignalFast(int ievent, int iLogicalChannel, float value)
{
    Events[ievent]->SetSignalFast(iLogicalChannel, value);
}

bool ADataHub::SetSignals(int ievent, const QVector<float> *vector)
{
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetSignals(vector);
    return true;
}

void ADataHub::SetSignalsFast(int ievent, const QVector<float> *vector)
{
    Events[ievent]->SetSignals(vector);
}

bool ADataHub::IsRejected(int ievent) const
{
    if (ievent<0 || ievent>=Events.size()) return true;
    return Events.at(ievent)->IsRejected();
}

bool ADataHub::IsRejectedFast(int ievent) const
{
    return Events.at(ievent)->IsRejected();
}

bool ADataHub::SetRejectedFlag(int ievent, bool flag)
{
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetRejectedFlag(flag);
    return true;
}

void ADataHub::SetRejectedFlagFast(int ievent, bool flag)
{
    Events[ievent]->SetRejectedFlag(flag);
}

void ADataHub::SetAllRejectedFlag(bool flag)
{
    for (AOneEvent* event : Events) event->SetRejectedFlag(flag);
}

const float *ADataHub::GetPosition(int ievent) const
{
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent)->GetPosition();
}

const float *ADataHub::GetPositionFast(int ievent) const
{
    return Events.at(ievent)->GetPosition();
}

bool ADataHub::SetPosition(int ievent, const float *XYZ)
{
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetPosition(XYZ);
    return true;
}

bool ADataHub::SetPosition(int ievent, float x, float y, float z)
{
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetPosition(x, y, z);
    return true;
}

void ADataHub::SetPositionFast(int ievent, const float *XYZ)
{
    Events[ievent]->SetPosition(XYZ);
}

void ADataHub::SetPositionFast(int ievent, float x, float y, float z)
{
    Events[ievent]->SetPosition(x, y, z);
}

const QVector<QVector<float> *>* ADataHub::GetWaveforms(int ievent) const
{
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent)->GetWaveforms();
}

const int *ADataHub::GetMultiplicityPositive(int ievent) const
{
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent)->GetMultiplicitiesPositive();
}

const int *ADataHub::GetMultiplicityPositiveFast(int ievent) const
{
    return Events.at(ievent)->GetMultiplicitiesPositive();
}

const int *ADataHub::GetMultiplicityNegative(int ievent) const
{
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent)->GetMultiplicitiesNegative();
}

const int *ADataHub::GetMultiplicityNegativeFast(int ievent) const
{
    return Events.at(ievent)->GetMultiplicitiesNegative();
}

bool ADataHub::SetMultiplicityPositive(int ievent, const int *multi)
{
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetMultiplicitiesPositive(multi);
    return true;
}

void ADataHub::SetMultiplicityPositiveFast(int ievent, const int *multi)
{
    Events[ievent]->SetMultiplicitiesPositive(multi);
}

bool ADataHub::SetMultiplicityNegative(int ievent, const int *multi)
{
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetMultiplicitiesNegative(multi);
    return true;
}

void ADataHub::SetMultiplicityNegativeFast(int ievent, const int *multi)
{
    Events[ievent]->SetMultiplicitiesNegative(multi);
}

const float *ADataHub::GetSumSignalPositive(int ievent) const
{
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent)->GetSumSigPositive();
}

const float *ADataHub::GetSumSignalPositiveFast(int ievent) const
{
    return Events.at(ievent)->GetSumSigPositive();
}

const float *ADataHub::GetSumSignalNegative(int ievent) const
{
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent)->GetSumSigNegative();
}

const float *ADataHub::GetSumSignalNegativeFast(int ievent) const
{
    return Events.at(ievent)->GetSumSigNegative();
}

bool ADataHub::SetSumSignalPositive(int ievent, const float *sums)
{
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetSumSigPositive(sums);
    return true;
}

void ADataHub::SetSumSignalPositiveFast(int ievent, const float *sums)
{
    Events[ievent]->SetSumSigPositive(sums);
}

bool ADataHub::SetSumSignalNegative(int ievent, const float *sums)
{
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetSumSigNegative(sums);
    return true;
}

void ADataHub::SetSumSignalNegativeFast(int ievent, const float *sums)
{
    Events[ievent]->SetSumSigNegative(sums);
}
