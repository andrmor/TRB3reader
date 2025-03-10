#include "ainterfacetohldfileprocessor.h"
#include "ahldfileprocessor.h"
#include "ascripthub.h"

AInterfaceToHldFileProcessor::AInterfaceToHldFileProcessor() :
    HldProcessor(AScriptHub::getInstance().HldProcessor)
{
    Description = "Provides full cycle of load/extraction/save_signals for an hld file.\n"
                  "To modify settings use the global configuration.";

    //connect(this, &AInterfaceToHldFileProcessor::requestProcess, this, &AInterfaceToHldFileProcessor::doProcess, Qt::QueuedConnection);
}

void AInterfaceToHldFileProcessor::ProcessFile_saveSignals(QString hldFileName, bool includeTimingData, bool skipSuppressedChannels, QString outputFileName)
{
    HldProcessor->ProcessFile(hldFileName, 0, includeTimingData, outputFileName, skipSuppressedChannels, false);
}

void AInterfaceToHldFileProcessor::ProcessFile_saveWaveforms(QString hldFileName, bool includeTimingData, bool skipSuppressedChannels, QString outputFileName)
{
    HldProcessor->ProcessFile(hldFileName, 1, includeTimingData, outputFileName, skipSuppressedChannels, false);
}

