#ifndef AINTERFACETOEXTRACTOR_H
#define AINTERFACETOEXTRACTOR_H

#include "ascriptinterface.h"

#include <QObject>
#include <QVariant>

class MasterConfig;
class Trb3signalExtractor;

class AInterfaceToExtractor : public AScriptInterface
{
    Q_OBJECT

public:
    AInterfaceToExtractor(MasterConfig *Config, Trb3signalExtractor *Extractor);

public slots:

    int      countEvents() const;
    int      countChannels() const;

    //get signals
    float    getSignal(int ievent, int iHardwChannel) const;
    const QVariant getSignals(int ievent) const;
    const QVariant getSignals_logical(int ievent) const; // array of signal values ordered according to the map of logical channels

    //set signals
    void     setSignal(int ievent, int iHardwChannel, float value);
    void     setSignals(int ievent, const QVariant arrayOfValues);

    //rejection of events
    void     setAllRejected(bool flag);
    bool     isRejectedEvent(int ievent) const;
    bool     isRejectedEventFast(int ievent) const;
    void     setRejected(int ievent, bool flag);

private:
    MasterConfig *Config;
    Trb3signalExtractor* Extractor;

};

#endif // AINTERFACETOEXTRACTOR_H
