#ifndef AINTERFACETOHLDFILEPROCESSOR_H
#define AINTERFACETOHLDFILEPROCESSOR_H

#include "ascriptinterface.h"

#include <QObject>

class AHldFileProcessor;

class AInterfaceToHldFileProcessor : public AScriptInterface
{
    Q_OBJECT

public:
    AInterfaceToHldFileProcessor();

    AScriptInterface * cloneBase() const override {return new AInterfaceToHldFileProcessor();}

public slots:
    void ProcessFile_saveSignals(QString hldFileName,   bool includeTimingData, bool skipSuppressedChannels, QString outputFileName);
    void ProcessFile_saveWaveforms(QString hldFileName, bool includeTimingData, bool skipSuppressedChannels, QString outputFileName);

private:
    AHldFileProcessor * HldProcessor = nullptr;

};

#endif // AINTERFACETOHLDFILEPROCESSOR_H
