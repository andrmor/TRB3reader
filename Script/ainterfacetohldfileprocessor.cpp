#include "ainterfacetohldfileprocessor.h"
#include "ahldfileprocessor.h"

AInterfaceToHldFileProcessor::AInterfaceToHldFileProcessor(AHldFileProcessor &hldProcessor) :
    hldProcessor(hldProcessor)
{
    Description = "Provides full cycle of load/extraction/save_signals for an hld file.\n"
                  "To modify settings use the global configuration.";
}

const QString AInterfaceToHldFileProcessor::ProcessFile(const QString FileName, bool bSaveTimeData, const QString SaveFileName)
{
    bool bOK = hldProcessor.ProcessFile(FileName, bSaveTimeData, SaveFileName);
    if (bOK) return "";
    else
    {
        abort("HldFileProcessor: " + hldProcessor.GetLastError());
        return "";
    }
}

