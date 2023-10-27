#ifndef AINTERFACETOWAVEFORMS_H
#define AINTERFACETOWAVEFORMS_H

#include "ascriptinterface.h"

#include <QObject>
#include <QVariant>

class MasterConfig;
class Trb3dataReader;

class AInterfaceToWaveforms : public AScriptInterface
{
    Q_OBJECT

public:
    AInterfaceToWaveforms(MasterConfig* Config, Trb3dataReader* Reader);

public slots:

    int      countSamples() const;

    float    getValue(int ievent, int iHardwChannel, int isample) const;
    float    getValueFast(int ievent, int iHardwChannel, int isample) const; // not safe - no argument check

    void     setValue(int ievent, int iHardwChannel, int isample, float value);
    void     setValueFast(int ievent, int iHardwChannel, int isample, float value);

    const QVariant getWaveform(int ievent, int iHardwChannel) const;
    const QVariant getWaveformFast(int ievent, int iHardwChannel) const; // not safe - no argument check

    void     setWaveform(int ievent, int ichannel, const QVariant array);
    void     setWaveformFast(int ievent, int ichannel, const QVariant array);

    //max/min value over waveform
    float    getMax(int ievent, int iHardwChannel) const;
    float    getMaxFast(int ievent, int iHardwChannel) const; // not safe - no argument check
    float    getMin(int ievent, int iHardwChannel) const;
    float    getMinFast(int ievent, int iHardwChannel) const; // not safe - no argument check

    //sample # where max/min is reached over waveform
    int      getMaxSample(int ievent, int iHardwChannel) const;
    int      getMaxSampleFast(int ievent, int iHardwChannel) const; // not safe - no argument check
    int      getMinSample(int ievent, int iHardwChannel) const;
    int      getMinSampleFast(int ievent, int iHardwChannel) const; // not safe - no argument check

    int      getSampleWhereFirstAbove(int ievent, int iHardwChannel, int threshold) const;
    int      getSampleWhereFirstAboveFast(int ievent, int iHardwChannel, int threshold) const;
    int      getSampleWhereFirstBelow(int ievent, int iHardwChannel, int threshold) const;
    int      getSampleWhereFirstBelowFast(int ievent, int iHardwChannel, int threshold) const;

private:
    MasterConfig*   Config;
    Trb3dataReader* Reader;
};

#endif // AINTERFACETOWAVEFORMS_H
