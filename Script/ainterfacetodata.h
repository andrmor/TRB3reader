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
    AInterfaceToData(const AInterfaceToData* other);

public slots:

    int      countEvents() const;
    int      countChannels() const;

    void     Clear();

    //Add event
    void     addEvent(const QVariant signalArray);

    // Signals
    float    getSignal     (int ievent, int iLogicalChannel) const;
    float    getSignalFast (int ievent, int iLogicalChannel) const;
    const QVariant getSignals    (int ievent) const;
    const QVariant getSignalsFast(int ievent) const;
    void     setSignal     (int ievent, int iLogicalChannel, float value);
    void     setSignalFast (int ievent, int iLogicalChannel, float value);
    void     setSignals    (int ievent, const QVariant arrayOfValues);
    void     setSignalsFast(int ievent, const QVariant arrayOfValues);

    // Rejection of events
    bool     isRejectedEvent    (int ievent) const;
    bool     isRejectedEventFast(int ievent) const;
    void     setRejected        (int ievent, bool flag);
    void     setRejectedFast    (int ievent, bool flag);
    void     setAllRejected     (bool flag);

    // Positions
    const QVariant getPosition    (int ievent) const;
    const QVariant getPositionFast(int ievent) const;
    void     setPosition    (int ievent, float x, float y, float z);
    void     setPositionFast(int ievent, float x, float y, float z);

    // Waveforms
    const QVariant getWaveforms(int ievent);
        //utilities
    float    getWaveformMax(int ievent, int ichannel) const;
    float    getWaveformMin(int ievent, int ichannel) const;
    int      getWaveformMaxSample(int ievent, int ichannel) const;
    int      getWaveformMinSample(int ievent, int ichannel) const;
    int      getWaveformSampleWhereFirstBelow(int ievent, int ichannel, float threshold) const;
    int      getWaveformSampleWhereFirstAbove(int ievent, int ichannel, float threshold) const;

    // Optional
        // Multiplicities
    void     setMultiplicity    (int ievent, QVariant px_py_pz_nx_ny_nz);
    void     setMultiplicityFast(int ievent, QVariant px_py_pz_nx_ny_nz);
    const QVariant getMultiplicity    (int ievent) const;
    const QVariant getMultiplicityFast(int ievent) const;
        // SumSignals
    void     setSumSignals    (int ievent, QVariant px_py_pz_nx_ny_nz);
    void     setSumSignalsFast(int ievent, QVariant px_py_pz_nx_ny_nz);
    const QVariant getSumSignals    (int ievent) const;
    const QVariant getSumSignalsFast(int ievent) const;

    void     save(const QString &FileName, bool bSavePositions, bool bSkipRejected) const;
    void     load(const QString &AppendFromFileName, bool bLoadPositionXYZ);

private:
    ADataHub* DataHub;
};

#endif // AINTERFACETODATA_H
