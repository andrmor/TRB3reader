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

    //get signals
    float    getSignal(int ievent, int iLogicalChannel);
    float    getSignalFast(int ievent, int iLogicalChannel);
    QVariant getSignals(int ievent);
    QVariant getSignalsFast(int ievent);

    //set signals
    void     setSignal(int ievent, int iLogicalChannel, float value);
    void     setSignalFast(int ievent, int iLogicalChannel, float value);
    void     setSignals(int ievent, QVariant arrayOfValues);
    void     setSignalsFast(int ievent, QVariant arrayOfValues);

    //rejection of events
    bool     isRejectedEvent(int ievent);
    bool     isRejectedEventFast(int ievent);
    void     setRejected(int ievent, bool flag);
    void     setRejectedFast(int ievent, bool flag);
    void     setAllRejected(bool flag);

    void     Clear();


private:
    ADataHub* DataHub;
};

#endif // AINTERFACETODATA_H
