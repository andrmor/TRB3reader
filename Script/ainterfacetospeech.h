#ifndef AINTERFACETOSPEECH_H
#define AINTERFACETOSPEECH_H

#include "ascriptinterface.h"

#include <QObject>
class QTextToSpeech;

class AInterfaceToSpeech : public AScriptInterface
{
        Q_OBJECT
public:
    AInterfaceToSpeech();
    AInterfaceToSpeech(const AInterfaceToSpeech* other);

public slots:
    void              Say(QString text);
    void              Stop();

    const QStringList GetAvailableEngines() const;
    bool              SelectEngine(int EngineIndex);

    const QStringList GetAvailableLocales() const;
    bool              SelectLocale(int LocaleIndex);

    const QStringList GetAvailableVoices() const;
    bool              SetVoice(int VoiceIndex);

    void              SetVolume(double Volume_0to1);
    void              SetPitch(double Pitch_minus1to1);
    void              SetRate(double Rate_minus1to1);

public:
    QTextToSpeech* m_speech;

};

#endif // AINTERFACETOSPEECH_H
