#ifndef ATEXTTOSPEECH_H
#define ATEXTTOSPEECH_H

#include <QObject>
#include <QString>
#include <QList>
#include <QTextToSpeech>

class QVoice;

class ATextToSpeech : public QObject
{
    Q_OBJECT
public:
    ATextToSpeech();
    ~ATextToSpeech();

    QTextToSpeech * Engine = nullptr;

    QStringList    AvailableEngines;
    QList<QLocale> AvailableLanguages;
    QList<QVoice>  AvailableVoices;

    double Volume = 0.5;
    double Pitch  = 0;
    double Rate   = 0;

    void selectEngine(const QString & name);
    void selectLocale(const QLocale & locale);
    void selectVoice(int voiceIndex);

    void setVolume(double value);
    void setPitch(double value);
    void setRate(double value);

    void say(const QString & text);
    void stop();

    bool isBusy() const;

    QString getCurrentEngineName() const;
    QVoice  getCurrentVoice() const;
    QLocale getCurrentLocale() const;

private slots:
    void onEngineReady();
    void onLocaleChanged();

signals:
    void engineChanged();
    void languagesChanged();
    void voicesChanged();

};

#endif // ATEXTTOSPEECH_H
