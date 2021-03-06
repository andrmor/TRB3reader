#ifndef ASCRIPTINTERFACEFACTORY_H
#define ASCRIPTINTERFACEFACTORY_H

#include "ascriptinterface.h"
#include "coreinterfaces.h"
#include "ainterfacetoconfig.h"
#include "ainterfacetodata.h"
#include "ainterfacetospeech.h"
#include "ainterfacetohldfileprocessor.h"
#include "ainterfacetowaveforms.h"
#include "ainterfacetoextractor.h"
#include "histgraphinterfaces.h"
#include "ainterfacetowebsocket.h"
#include "ainterfacetomessagewindow.h"

#include <QObject>

class AScriptInterfaceFactory
{
public:    
    static QObject* makeCopy(const QObject* other)
    {
        const AInterfaceToCore* core = dynamic_cast<const AInterfaceToCore*>(other);
        if (core) return new AInterfaceToCore(*core);

        const AInterfaceToMath* math = dynamic_cast<const AInterfaceToMath*>(other);
        if (math) return new AInterfaceToMath(*math);

        const AInterfaceToConfig* config = dynamic_cast<const AInterfaceToConfig*>(other);
        if (config) return new AInterfaceToConfig(*config);

        const AInterfaceToData* events = dynamic_cast<const AInterfaceToData*>(other);
        if (events) return new AInterfaceToData(*events);

        const AInterfaceToHldFileProcessor* hld = dynamic_cast<const AInterfaceToHldFileProcessor*>(other);
        if (hld) return new AInterfaceToHldFileProcessor(*hld);

        const AInterfaceToWaveforms* wave = dynamic_cast<const AInterfaceToWaveforms*>(other);
        if (wave) return new AInterfaceToWaveforms(*wave);

        const AInterfaceToExtractor* extr = dynamic_cast<const AInterfaceToExtractor*>(other);
        if (extr) return new AInterfaceToExtractor(*extr);

        const AInterfaceToHist* hist = dynamic_cast<const AInterfaceToHist*>(other);
        if (hist) return new AInterfaceToHist(*hist);

        const AInterfaceToGraph* graph = dynamic_cast<const AInterfaceToGraph*>(other);
        if (graph) return new AInterfaceToGraph(*graph);

        const AInterfaceToWebSocket* web = dynamic_cast<const AInterfaceToWebSocket*>(other);
        if (web) return new AInterfaceToWebSocket(*web);

        const AInterfaceToMessageWindow* msg = dynamic_cast<const AInterfaceToMessageWindow*>(other);
        if (msg) return new AInterfaceToMessageWindow(*msg);

#ifdef SPEECH
        const AInterfaceToSpeech* speech = dynamic_cast<const AInterfaceToSpeech*>(other);
        if (speech) return new AInterfaceToSpeech(*speech);
#endif

        return 0;
    }
};

#endif // ASCRIPTINTERFACEFACTORY_H
