#ifndef ADATAHUB_H
#define ADATAHUB_H

#include <QVector>

class AOneEvent
{
public:

    int     CountChannels() const {return _Signals.size();}

    float   GetSignal(int ichannel) const;
    float   GetSignalFast(int ichannel) const {return _Signals.at(ichannel);}
    bool    SetSignal(int ichannel, float value);
    void    SetSignalFast(int ichannel, float value) {_Signals[ichannel] = value;}

    const QVector<float>* GetSignals() const {return &_Signals;}
    void    SetSignals(const QVector<float> *vector) {_Signals = *vector;}

    bool    IsRejected() const {return _Rejected;}
    void    SetRejectedFlag(bool flag) {_Rejected = flag;}

    const float* GetPosition() const {return _Position;}
    void    SetPosition(const float* XYZ)          {_Position[0] = XYZ[0]; _Position[1] = XYZ[1]; _Position[2] = XYZ[2];}
    void    SetPosition(float x, float y, float z) {_Position[0] = x;      _Position[1] = y;      _Position[2] = z;     }

    const QVector< QVector<float>* >* GetWaveforms() const { return &_Waveforms; }
    void    SetWaveforms(const QVector< QVector<float>* >* waveforms) { _Waveforms = *waveforms;}
    void    ClearWaveforms();

private:
    QVector<float>              _Signals;     // [logical_channel]
    QVector< QVector<float>* >  _Waveforms;   // [logical_channel] [sample#] - may contain 0 pointer - if waveworm has no data

    bool                        _Rejected;    // event is rejected
    float                       _Position[3]; // x y z
};

class ADataHub
{
public:
    ~ADataHub();

    // Counts
    int              CountEvents() {return _Events.size();}
    int              CountChannels();

    // General handling
    void             Clear();
    void             AddEvent(AOneEvent* Event) {_Events << Event;}
    void             RemoveEvent(int ievent);
    void             RemoveEventFast(int ievent) {_Events.removeAt(ievent);}
    const AOneEvent* GetEvent(int ievent) const;
    AOneEvent*       GetEvent(int ievent);
    const AOneEvent* GetEventFast(int ievent) const {return _Events.at(ievent);}
    AOneEvent*       GetEventFast(int ievent) {return _Events[ievent];}

    // Signals
    float            GetSignal(int ievent, int ichannel);
    float            GetSignalFast(int ievent, int ichannel);
    const QVector<float>* GetSignals(int ievent) const;
    const QVector<float>* GetSignalsFast(int ievent) const;
    bool             SetSignal(int ievent, int iLogicalChannel, float value);
    void             SetSignalFast(int ievent, int iLogicalChannel, float value);
    bool             SetSignals(int ievent, const QVector<float>* vector);
    void             SetSignalsFast(int ievent, const QVector<float>* vector);

    // Rejection
    bool             IsRejected(int ievent) const;
    bool             IsRejectedFast(int ievent) const;
    bool             SetRejectedFlag(int ievent, bool flag);
    void             SetRejectedFlagFast(int ievent, bool flag);
    void             SetAllRejectedFlag(bool flag);

    // Position
    const float*     GetPosition(int ievent) const;
    const float*     GetPositionFast(int ievent) const;
    bool             SetPosition(int ievent, const float* XYZ);
    bool             SetPosition(int ievent, float x, float y, float z);
    void             SetPositionFast(int ievent, const float* XYZ);
    void             SetPositionFast(int ievent, float x, float y, float z);

    // Waveforms
    const QVector< QVector<float>* >* GetWaveforms(int ievent) const;

private:
    QVector < AOneEvent* > _Events;

};

#endif // ADATAHUB_H
