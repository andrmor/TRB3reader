#ifndef AINTERFACETOSIGNALS_H
#define AINTERFACETOSIGNALS_H

#include "ascriptinterface.h"

#include <QObject>
#include <QVariant>

class MasterConfig;
class Trb3signalExtractor;

class AInterfaceToSignals : public AScriptInterface
{
    Q_OBJECT

public:
    AInterfaceToSignals(MasterConfig *Config, Trb3signalExtractor *Extractor);

public slots:
    // info
    int countEvents();
    int countHardwareChannels();
    int countLogicalChannels();

    //get signals
    double getSignal_hardware(int ievent, int ichannel);
    QVariant getSignals_hardware(int ievent);
    double getSignal_logical(int ievent, int ichannel);
    QVariant getSignals_logical(int ievent);

    //set signals
    void setSignal_hardware(int ievent, int ichannel, double value);
    void setSignals_hardware(int ievent, QVariant arrayOfValues);
    void setSignal_logical(int ievent, int ichannel, double value);

    //rejection of events
    void setAllRejected(bool flag);
    void setRejected(int ievent, bool flag);

private:
    MasterConfig *Config;
    Trb3signalExtractor* Extractor;

};

#endif // AINTERFACETOSIGNALS_H
