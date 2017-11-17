#ifndef AINTERFACETOSIGNALS_H
#define AINTERFACETOSIGNALS_H

#include "ascriptinterface.h"

#include <QObject>
#include <QVariant>

class Trb3signalExtractor;
class ChannelMapper;

class AInterfaceToSignals : public AScriptInterface
{
    Q_OBJECT

public:
    AInterfaceToSignals(Trb3signalExtractor *Extractor, ChannelMapper* Map);

public slots:
    double getSignal_hardware(int ievent, int ichannel);
    QVariant getSignals_hardware(int ievent);

    double getSignal_logical(int ievent, int ichannel);
    QVariant getSignals_logical(int ievent);

private:
    Trb3signalExtractor* Extractor;
    ChannelMapper* Map;

};

#endif // AINTERFACETOSIGNALS_H
