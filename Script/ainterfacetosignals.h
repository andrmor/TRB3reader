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

    int      countEvents();
    int      countChannels();

    //get signals
    double   getSignal(int ievent, int iHardwChannel);
    QVariant getSignals(int ievent);

    //set signals
    void     setSignal(int ievent, int iHardwChannel, double value);
    void     setSignals(int ievent, QVariant arrayOfValues);

    //rejection of events
    void     setAllRejected(bool flag);
    void     setRejected(int ievent, bool flag);

private:
    MasterConfig *Config;
    Trb3signalExtractor* Extractor;

};

#endif // AINTERFACETOSIGNALS_H
