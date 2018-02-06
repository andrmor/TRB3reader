#ifndef ASCRIPTINTERFACEFACTORY_H
#define ASCRIPTINTERFACEFACTORY_H

#include "ascriptinterface.h"
#include "coreinterfaces.h"
#include "ainterfacetospeech.h"

#include <QObject>

class AScriptInterfaceFactory
{
public:    
    static QObject* makeCopy(const QObject* other)
    {
        const AInterfaceToCore* core = dynamic_cast<const AInterfaceToCore*>(other);
        if (core) return new AInterfaceToCore(core);

        const AInterfaceToMath* math = dynamic_cast<const AInterfaceToMath*>(other);
        if (math) return new AInterfaceToMath(math);

        const AInterfaceToSpeech* speech = dynamic_cast<const AInterfaceToSpeech*>(other);
        if (speech) return new AInterfaceToSpeech(speech);

        return 0;
    }
};

#endif // ASCRIPTINTERFACEFACTORY_H