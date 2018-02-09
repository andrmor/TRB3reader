#include "ainterfacetohldfileprocessor.h"
#include "ahldfileprocessor.h"

AInterfaceToHldFileProcessor::AInterfaceToHldFileProcessor(AHldFileProcessor &hldProcessor) :
    hldProcessor(hldProcessor)
{
    Description = "Provides full cycle of load/extraction/save_signals for an hld file.\n"
                  "To modify settings use the global configuration.";
}

AInterfaceToHldFileProcessor::AInterfaceToHldFileProcessor(const AInterfaceToHldFileProcessor *other) :
    AScriptInterface(other), hldProcessor(other->hldProcessor) {}

const QString AInterfaceToHldFileProcessor::ProcessFile(const QString FileName, const QString SaveFileName)
{
    bool bOK = hldProcessor.ProcessFile(FileName, SaveFileName);
    if (bOK) return "";
    else
    {
        abort("HldFileProcessor: " + hldProcessor.GetLastError());
        return "";
    }
}

