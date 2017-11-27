#ifndef ADATAHUB_H
#define ADATAHUB_H

#include <QVector>

struct AOneEvent
{
    QVector<float>              Signals;     // [logical_channel]
    QVector< QVector<float>* >  Waveforms;   // [logical_channel] [sample#] - may contain 0 pointer - if waveworm has no data

    bool                        Rejected;    // event is rejected
    float                       Position[3]; // x y z
};

class ADataHub
{
public:
    void             Clear();

    void             AddEvent(AOneEvent* Event) {Events << Event;}

    const AOneEvent* GetEvent(int ievent) const;
    AOneEvent*       GetEvent(int ievent);
    const AOneEvent* GetEventFast(int ievent) const {return Events.at(ievent);}
    AOneEvent*       GetEventFast(int ievent) {return Events[ievent];}

    void             RemoveEvent(int ievent);
    void             RemoveEventFast(int ievent) {Events.removeAt(ievent);}

private:
    QVector < AOneEvent* > Events;

};

#endif // ADATAHUB_H
