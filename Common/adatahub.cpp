#include "adatahub.h"



void ADataHub::Clear()
{
    for (AOneEvent* event : Events)
    {
        for (QVector<float>* vec : event->Waveforms) delete vec;
        delete event;
    }
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

void ADataHub::RemoveEvent(int ievent)
{
    if (ievent<0 || ievent>=Events.size()) return;
    Events.removeAt(ievent);
}
