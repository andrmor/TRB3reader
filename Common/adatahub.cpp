#include "adatahub.h"

#include <limits>
const float NaN = std::numeric_limits<float>::quiet_NaN();

const AOneEvent *ADataHub::GetEvent(int ievent) const
{
    if (ievent<0 || ievent>=_Events.size()) return 0;
    return _Events.at(ievent);
}

AOneEvent *ADataHub::GetEvent(int ievent)
{
    if (ievent<0 || ievent>=_Events.size()) return 0;
    return _Events[ievent];
}

float AOneEvent::GetSignal(int ichannel) const
{
    if (ichannel<0 || ichannel>=_Signals.size()) return NaN;
    return _Signals.at(ichannel);
}

bool AOneEvent::SetSignal(int ichannel, float value)
{
    if (ichannel<0 || ichannel>=_Signals.size()) return false;
    _Signals[ichannel] = value;
    return true;
}





void AOneEvent::ClearWaveforms()
{
    for (QVector<float>* vec : _Waveforms) delete vec;
}

ADataHub::~ADataHub()
{
    Clear();
}

void ADataHub::Clear()
{
    for (AOneEvent* event : _Events)
    {
        event->ClearWaveforms();
        delete event;
    }
    _Events.clear();
}

int ADataHub::CountChannels()
{
    if (_Events.isEmpty()) return 0;
    return _Events.first()->CountChannels();
}

float ADataHub::GetSignal(int ievent, int ichannel)
{
    const AOneEvent* event = GetEvent(ievent);
    if (!event) return NaN;

    return event->GetSignal(ichannel);
}

float ADataHub::GetSignalFast(int ievent, int ichannel)
{
    return _Events.at(ievent)->GetSignalFast(ichannel);
}

const QVector<float> *ADataHub::GetSignals(int ievent) const
{
    if (ievent<0 || ievent>=_Events.size()) return 0;
    return _Events.at(ievent)->GetSignals();
}

const QVector<float> *ADataHub::GetSignalsFast(int ievent) const
{
    return _Events.at(ievent)->GetSignals();
}

bool ADataHub::SetSignal(int ievent, int iLogicalChannel, float value)
{
    if (ievent<0 || ievent>=_Events.size()) return false;
    return _Events[ievent]->SetSignal(iLogicalChannel, value);
}

void ADataHub::SetSignalFast(int ievent, int iLogicalChannel, float value)
{
    _Events[ievent]->SetSignalFast(iLogicalChannel, value);
}

bool ADataHub::SetSignals(int ievent, const QVector<float> *vector)
{
    if (ievent<0 || ievent>=_Events.size()) return false;
    _Events[ievent]->SetSignals(vector);
    return true;
}

void ADataHub::SetSignalsFast(int ievent, const QVector<float> *vector)
{
    _Events[ievent]->SetSignals(vector);
}

bool ADataHub::IsRejected(int ievent) const
{
    if (ievent<0 || ievent>=_Events.size()) return true;
    return _Events.at(ievent)->IsRejected();
}

bool ADataHub::IsRejectedFast(int ievent) const
{
    return _Events.at(ievent)->IsRejected();
}

bool ADataHub::SetRejectedFlag(int ievent, bool flag)
{
    if (ievent<0 || ievent>=_Events.size()) return false;
    _Events[ievent]->SetRejectedFlag(flag);
    return true;
}

void ADataHub::SetRejectedFlagFast(int ievent, bool flag)
{
    _Events[ievent]->SetRejectedFlag(flag);
}

void ADataHub::SetAllRejectedFlag(bool flag)
{
    for (AOneEvent* event : _Events) event->SetRejectedFlag(flag);
}

const float *ADataHub::GetPosition(int ievent) const
{
    if (ievent<0 || ievent>=_Events.size()) return 0;
    return _Events.at(ievent)->GetPosition();
}

const float *ADataHub::GetPositionFast(int ievent) const
{
    return _Events.at(ievent)->GetPosition();
}

bool ADataHub::SetPosition(int ievent, const float *XYZ)
{
    if (ievent<0 || ievent>=_Events.size()) return false;
    _Events[ievent]->SetPosition(XYZ);
    return true;
}

bool ADataHub::SetPosition(int ievent, float x, float y, float z)
{
    if (ievent<0 || ievent>=_Events.size()) return false;
    _Events[ievent]->SetPosition(x, y, z);
    return true;
}

void ADataHub::SetPositionFast(int ievent, const float *XYZ)
{
    _Events[ievent]->SetPosition(XYZ);
}

void ADataHub::SetPositionFast(int ievent, float x, float y, float z)
{
    _Events[ievent]->SetPosition(x, y, z);
}

const QVector<QVector<float> *>* ADataHub::GetWaveforms(int ievent) const
{
    if (ievent<0 || ievent>=_Events.size()) return 0;
    return _Events.at(ievent)->GetWaveforms();
}

void ADataHub::RemoveEvent(int ievent)
{
    if (ievent<0 || ievent>=_Events.size()) return;
    _Events.removeAt(ievent);
}

