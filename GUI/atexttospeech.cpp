#include "atexttospeech.h"

#include <QLoggingCategory>

using namespace Qt::StringLiterals;

ATextToSpeech::ATextToSpeech()
{
    QLoggingCategory::setFilterRules(u"qt.speech.tts=true \n qt.speech.tts.*=true"_s);

    AvailableEngines << "default" << QTextToSpeech::availableEngines();
    selectEngine("default");
}

ATextToSpeech::~ATextToSpeech()
{
    delete Engine; Engine = nullptr;
}

void ATextToSpeech::selectEngine(const QString & name)
{
    //qDebug() << "<<<<<" << name;
    delete Engine;
    Engine = (name == "default" ? new QTextToSpeech() : new QTextToSpeech(name) );

    // some engines initialize asynchronously
    if (Engine->state() == QTextToSpeech::Ready)
        onEngineReady();
    else
        connect(Engine, &QTextToSpeech::stateChanged, this, &ATextToSpeech::onEngineReady, Qt::SingleShotConnection);
}

void ATextToSpeech::onEngineReady()
{
    qDebug() << "<<<<" << Engine->engine() << Engine->state();
    if (Engine->state() != QTextToSpeech::Ready) return;

    emit engineChanged();

    AvailableLanguages = Engine->availableLocales();
    emit languagesChanged();

    setRate(Rate);
    setPitch(Pitch);
    setVolume(Volume);

   // connect(Engine, &QTextToSpeech::stateChanged, this, &ATextToSpeech::onStateChanged);
    connect(Engine, &QTextToSpeech::localeChanged, this, &ATextToSpeech::onLocaleChanged);

    onLocaleChanged();
}

void ATextToSpeech::selectLocale(const QLocale & locale)
{
    Engine->setLocale(locale);
    onLocaleChanged();
}

void ATextToSpeech::onLocaleChanged()
{
    AvailableVoices = Engine->availableVoices();
    emit voicesChanged();
}

void ATextToSpeech::selectVoice(int voiceIndex)
{
    Engine->setVoice(AvailableVoices[voiceIndex]);
}

void ATextToSpeech::setVolume(double value)
{
    Volume = value;
    if (Engine) Engine->setVolume(value);
}
void ATextToSpeech::setPitch(double value)
{
    Pitch = value;
    if (Engine) Engine->setPitch(value);
}
void ATextToSpeech::setRate(double value)
{
    Rate = value;
    if (Engine) Engine->setRate(value);
}

void ATextToSpeech::say(const QString & text)
{
    if (Engine) Engine->say(text);
}

void ATextToSpeech::stop()
{
    if (Engine) Engine->stop();
}

bool ATextToSpeech::isBusy() const
{
    if (!Engine) return false;
    return (Engine->state() != QTextToSpeech::Speaking);
}

QString ATextToSpeech::getCurrentEngineName() const
{
    if (!Engine) return "default";
    return Engine->engine();
}

QVoice ATextToSpeech::getCurrentVoice() const
{
    if (!Engine) return QVoice();
    return Engine->voice();
}

QLocale ATextToSpeech::getCurrentLocale() const
{
    if (!Engine) return QLocale();
    return Engine->locale();
}
