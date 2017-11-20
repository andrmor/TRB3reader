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

    double getValue_hardware(int ievent, int ichannel, int isample);
    double getValueFast_hardware(int ievent, int ichannel, int isample);

    QVariant getWaveform_hardware(int ievent, int ichannel);
    QVariant getWaveformFast_hardware(int ievent, int ichannel);

    QVariant getWaveform_logical(int ievent, int ichannel);
    QVariant getWaveformFast_logical(int ievent, int ichannel);


private:
    MasterConfig* Config;
    Trb3dataReader* Reader;
};

#endif // AINTERFACETOWAVEFORMS_H
