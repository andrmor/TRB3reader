#ifndef AINTERFACETODATA_H
#define AINTERFACETODATA_H

#include "ascriptinterface.h"

#include <QObject>
#include <QVariant>

class ADataHub;

class AInterfaceToData : public AScriptInterface
{
    Q_OBJECT

public:
    AInterfaceToData(ADataHub* DataHub);

public slots:

    int      countEvents();
    int      countChannels();

    void     Clear();

    // Signals
    float    getSignal     (int ievent, int iLogicalChannel);
    float    getSignalFast (int ievent, int iLogicalChannel);
    QVariant getSignals    (int ievent);
    QVariant getSignalsFast(int ievent);
    void     setSignal     (int ievent, int iLogicalChannel, float value);
    void     setSignalFast (int ievent, int iLogicalChannel, float value);
    void     setSignals    (int ievent, QVariant arrayOfValues);
    void     setSignalsFast(int ievent, QVariant arrayOfValues);

    // Rejection of events
    bool     isRejectedEvent    (int ievent);
    bool     isRejectedEventFast(int ievent);
    void     setRejected        (int ievent, bool flag);
    void     setRejectedFast    (int ievent, bool flag);
    void     setAllRejected     (bool flag);

    // Positions
    QVariant getPosition    (int ievent);
    QVariant getPositionFast(int ievent);
    void     setPosition    (int ievent, float x, float y, float z);
    void     setPositionFast(int ievent, float x, float y, float z);

    // Waveforms
    QVariant getWaveforms(int ievent);
        //utilities
    float    getWaveformMax(int ievent, int ichannel);
    float    getWaveformMin(int ievent, int ichannel);
    int      getWaveformMaxSample(int ievent, int ichannel);
    int      getWaveformMinSample(int ievent, int ichannel);
    int      getWaveformSampleWhereFirstBelow(int ievent, int ichannel, float threshold);
    int      getWaveformSampleWhereFirstAbove(int ievent, int ichannel, float threshold);

    // Optional
        // Multiplicities
    void     setMultiplicity    (int ievent, QVariant px_py_pz_nx_ny_nz);
    void     setMultiplicityFast(int ievent, QVariant px_py_pz_nx_ny_nz);
    QVariant getMultiplicity    (int ievent);
    QVariant getMultiplicityFast(int ievent);
        // SumSignals
    void     setSumSignals    (int ievent, QVariant px_py_pz_nx_ny_nz);
    void     setSumSignalsFast(int ievent, QVariant px_py_pz_nx_ny_nz);
    QVariant getSumSignals    (int ievent);
    QVariant getSumSignalsFast(int ievent);


private:
    ADataHub* DataHub;
};

#endif // AINTERFACETODATA_H
