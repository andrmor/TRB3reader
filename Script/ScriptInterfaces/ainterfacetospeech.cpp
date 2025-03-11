#include "ainterfacetospeech.h"

#include <QVector>
#include <QTextToSpeech>
#include <QDebug>

AInterfaceToSpeech::AInterfaceToSpeech() : m_speech(0)
{
    Description = "Interface to native text-to-speech engine.";

    if (QTextToSpeech::availableEngines().isEmpty())
    {
        qDebug() << "Speech unit: there are no speech engines available!";
    }
    else
    {
        SelectEngine(0);
    }
}

void AInterfaceToSpeech::Say(QString text)
{
    if (!m_speech) return;
    m_speech->say(text);
}

void AInterfaceToSpeech::Stop()
{
    if (!m_speech) return;
    m_speech->stop();
}

const QStringList AInterfaceToSpeech::GetAvailableEngines() const
{
    QStringList list = QTextToSpeech::availableEngines();
    for (int i=0; i<list.size(); i++)
        list[i] = QString::number(i)+"="+list.at(i);
    return list;
}

bool AInterfaceToSpeech::SelectEngine(int EngineIndex)
{
    int numEn = QTextToSpeech::availableEngines().size();
    if (EngineIndex<0 || EngineIndex>=numEn) return false;

    delete m_speech; m_speech = 0;
    const QString engineName = QTextToSpeech::availableEngines().at(EngineIndex);
    m_speech = new QTextToSpeech(engineName, this);

    return true;
}

const QStringList AInterfaceToSpeech::GetAvailableLocales() const
{
    if (!m_speech) return QStringList();

    QVector<QLocale> locales = m_speech->availableLocales();
    //QLocale current = m_speech->locale();

    QStringList list;
    int icounter = 0;
    foreach (const QLocale &locale, locales)
    {
        QString name(QString("%1 (%2)")
                     .arg(QLocale::languageToString(locale.language()))
                     .arg(QLocale::countryToString(locale.country())));
        list << QString::number(icounter++)+"="+name;
    }
    return list;
}

bool AInterfaceToSpeech::SelectLocale(int LocaleIndex)
{
    if (!m_speech) return false;

    QVector<QLocale> locales = m_speech->availableLocales();
    if (LocaleIndex<0 || LocaleIndex>=locales.size()) return false;

    m_speech->setLocale(locales.at(LocaleIndex));
    return true;
}

const QStringList AInterfaceToSpeech::GetAvailableVoices() const
{
    if (!m_speech) return QStringList();

    QVector<QVoice> m_voices = m_speech->availableVoices();
    //QVoice currentVoice = m_speech->voice();

    QStringList list;
    int icounter = 0;
    foreach (const QVoice &voice, m_voices)
    {
        QString tv = QString("%1 (%2 %3)")
                          .arg(voice.name())
                          .arg(QVoice::genderName(voice.gender()))
                          .arg(QVoice::ageName(voice.age()));
        list << QString::number(icounter++)+"="+tv;
    }
    return list;
}

bool AInterfaceToSpeech::SetVoice(int VoiceIndex)
{
    if (!m_speech) return false;

    QVector<QVoice> m_voices = m_speech->availableVoices();
    if (VoiceIndex<0 || VoiceIndex>=m_voices.size()) return false;

    m_speech->setVoice(m_voices.at(VoiceIndex));

    return true;
}

void AInterfaceToSpeech::SetVolume(double Volume_0to1)
{
    if (!m_speech) return;
    m_speech->setVolume(Volume_0to1);
}

void AInterfaceToSpeech::SetPitch(double Pitch_minus1to1)
{
    if (!m_speech) return;
    m_speech->setPitch(Pitch_minus1to1);
}

void AInterfaceToSpeech::SetRate(double Rate_minus1to1)
{
    if (!m_speech) return;
    m_speech->setRate(Rate_minus1to1);
}
