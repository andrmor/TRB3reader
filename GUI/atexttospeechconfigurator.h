#ifndef ATEXTTOSPEECHCONFIGURATOR_H
#define ATEXTTOSPEECHCONFIGURATOR_H

#include <QMainWindow>
#include <QTextToSpeech>

namespace Ui {
class ATextToSpeechConfigurator;
}

class ATextToSpeech;

class ATextToSpeechConfigurator : public QMainWindow
{
    Q_OBJECT

public:
    explicit ATextToSpeechConfigurator(ATextToSpeech & textToSpeechHub, QWidget *parent = nullptr);
    ~ATextToSpeechConfigurator();

private slots:
    void on_pSpeak_clicked();
    void on_pStop_clicked();

    void on_slVolume_valueChanged(int value);
    void on_slRate_valueChanged(int value);
    void on_slPitch_valueChanged(int value);

    void on_cobEngine_activated(int index);
    void on_cobLanguage_activated(int index);
    void on_cobVoice_activated(int index);

    void onEngineChanged();
    void onLanguagesChanged();
    void onVoicesChanged();

private:
    ATextToSpeech & TextToSpeechHub;
    Ui::ATextToSpeechConfigurator * ui = nullptr;

};

#endif // ATEXTTOSPEECHCONFIGURATOR_H
