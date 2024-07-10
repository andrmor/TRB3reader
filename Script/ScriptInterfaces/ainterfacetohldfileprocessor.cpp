#include "ainterfacetohldfileprocessor.h"
#include "ahldfileprocessor.h"
#include "ascripthub.h"

AInterfaceToHldFileProcessor::AInterfaceToHldFileProcessor() :
    HldProcessor(AScriptHub::getInstance().HldProcessor)
{
    Description = "Provides full cycle of load/extraction/save_signals for an hld file.\n"
                  "To modify settings use the global configuration.";
}

const QString AInterfaceToHldFileProcessor::ProcessFile(QString FileName, int What_0signals1waves, bool bIncludeTimeData, QString SaveFileName, bool doNotSaveSuppressedChannels)
{
    bool bOK = HldProcessor->ProcessFile(FileName, What_0signals1waves, bIncludeTimeData, SaveFileName, doNotSaveSuppressedChannels);
    if (bOK) return "";
    else
    {
        abort("HldFileProcessor: " + HldProcessor->GetLastError());
        return "";
    }
}

