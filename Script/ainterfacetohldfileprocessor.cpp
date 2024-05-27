#include "ainterfacetohldfileprocessor.h"
#include "ahldfileprocessor.h"

AInterfaceToHldFileProcessor::AInterfaceToHldFileProcessor(AHldFileProcessor &hldProcessor) :
    hldProcessor(hldProcessor)
{
    Description = "Provides full cycle of load/extraction/save_signals for an hld file.\n"
                  "To modify settings use the global configuration.";
}

const QString AInterfaceToHldFileProcessor::ProcessFile(QString FileName, int What_0signals1waves, bool bIncludeTimeData, QString SaveFileName, bool doNotSaveSuppressedChannels)
{
    bool bOK = hldProcessor.ProcessFile(FileName, What_0signals1waves, bIncludeTimeData, SaveFileName, doNotSaveSuppressedChannels);
    if (bOK) return "";
    else
    {
        abort("HldFileProcessor: " + hldProcessor.GetLastError());
        return "";
    }
}

