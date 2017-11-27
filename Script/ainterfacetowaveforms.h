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

    int      countSamples();

    double   getValue(int ievent, int iHardwChannel, int isample);
    double   getValueFast(int ievent, int iHardwChannel, int isample); // not safe - no argument check

    QVariant getWaveform(int ievent, int iHardwChannel);
    QVariant getWaveformFast(int ievent, int iHardwChannel); // not safe - no argument check

    //max/min value over waveform
    double   getMax(int ievent, int iHardwChannel);
    double   getMaxFast(int ievent, int iHardwChannel); // not safe - no argument check
    double   getMin(int ievent, int iHardwChannel);
    double   getMinFast(int ievent, int iHardwChannel); // not safe - no argument check

    //sample # where max/min is reached over waveform
    int      getMaxSample(int ievent, int iHardwChannel);
    int      getMaxSampleFast(int ievent, int iHardwChannel); // not safe - no argument check
    int      getMinSample(int ievent, int iHardwChannel);
    int      getMinSampleFast(int ievent, int iHardwChannel); // not safe - no argument check

    int      getSampleWhereFirstAbove(int ievent, int iHardwChannel, int threshold);
    int      getSampleWhereFirstAboveFast(int ievent, int iHardwChannel, int threshold);
    int      getSampleWhereFirstBelow(int ievent, int iHardwChannel, int threshold);
    int      getSampleWhereFirstBelowFast(int ievent, int iHardwChannel, int threshold);

private:
    MasterConfig* Config;
    Trb3dataReader* Reader;
};

#endif // AINTERFACETOWAVEFORMS_H
