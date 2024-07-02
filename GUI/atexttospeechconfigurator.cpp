#include "atexttospeechconfigurator.h"
#include "ui_atexttospeechconfigurator.h"
#include "atexttospeech.h"

ATextToSpeechConfigurator::ATextToSpeechConfigurator(ATextToSpeech & textToSpeechHub, QWidget * parent) :
    QMainWindow(parent),
    TextToSpeechHub(textToSpeechHub),
    ui(new Ui::ATextToSpeechConfigurator)
{
    ui->setupUi(this);

    onEngineChanged();
    onLanguagesChanged();
    onVoicesChanged();

    connect(&textToSpeechHub, &ATextToSpeech::engineChanged,    this, &ATextToSpeechConfigurator::onEngineChanged);
    connect(&textToSpeechHub, &ATextToSpeech::languagesChanged, this, &ATextToSpeechConfigurator::onLanguagesChanged);
    connect(&textToSpeechHub, &ATextToSpeech::voicesChanged,    this, &ATextToSpeechConfigurator::onVoicesChanged);
}

ATextToSpeechConfigurator::~ATextToSpeechConfigurator()
{
    delete ui;
}

void ATextToSpeechConfigurator::on_pSpeak_clicked()
{
    TextToSpeechHub.say(ui->pteTestText->toPlainText());
}

void ATextToSpeechConfigurator::on_pStop_clicked()
{
    TextToSpeechHub.stop();
}

void ATextToSpeechConfigurator::on_slVolume_valueChanged(int value)
{
    TextToSpeechHub.setVolume(value/100.0);
}

void ATextToSpeechConfigurator::on_slRate_valueChanged(int value)
{
    TextToSpeechHub.setRate(value/10.0);
}

void ATextToSpeechConfigurator::on_slPitch_valueChanged(int value)
{
    TextToSpeechHub.setPitch(value/10.0);
}

void ATextToSpeechConfigurator::on_cobEngine_activated(int)
{
    QString engineName = ui->cobEngine->currentText();
    TextToSpeechHub.selectEngine(engineName);
}

void ATextToSpeechConfigurator::on_cobLanguage_activated(int index)
{
    QLocale locale = ui->cobLanguage->itemData(index).toLocale();
    TextToSpeechHub.selectLocale(locale);
}

void ATextToSpeechConfigurator::on_cobVoice_activated(int index)
{
    TextToSpeechHub.selectVoice(index);
}

void ATextToSpeechConfigurator::onEngineChanged()
{
    ui->cobEngine->clear();
    ui->cobEngine->addItems(TextToSpeechHub.AvailableEngines);

    //qDebug() << "looking for:" << TextToSpeechHub.getCurrentEngineName() << TextToSpeechHub.AvailableEngines;
    ui->cobEngine->setCurrentIndex(ui->cobEngine->findText(TextToSpeechHub.getCurrentEngineName()));
}

void ATextToSpeechConfigurator::onLanguagesChanged()
{
    ui->cobLanguage->clear();

    const QLocale current = TextToSpeechHub.getCurrentLocale();

    QStringList langs;
    const size_t num = TextToSpeechHub.AvailableLanguages.size();
    size_t selected = 0;
    for (size_t i = 0; i < num; i++)
    {
        const QLocale & locale = TextToSpeechHub.AvailableLanguages[i];
        if (locale == current) selected = i;

        QString name( QString("%1 (%2)")
                         .arg(QLocale::languageToString(locale.language()))
                         .arg(QLocale::territoryToString(locale.territory())) );
        langs << name;
    }
    ui->cobLanguage->addItems(langs);
    ui->cobLanguage->setCurrentIndex(selected);
}

void ATextToSpeechConfigurator::onVoicesChanged()
{
    ui->cobVoice->clear();

    const QVoice currentVoice = TextToSpeechHub.getCurrentVoice();
    const size_t num = TextToSpeechHub.AvailableVoices.size();
    size_t selected = 0;

    for (size_t i = 0; i < num; i++)
    {
        const QVoice voice = TextToSpeechHub.AvailableVoices[i];
        if (voice == currentVoice) selected = i;

        QString name = voice.name();
        QStringList sl = name.split('+', Qt::SkipEmptyParts);
        if (sl.size() > 1) name = sl[1];
        ui->cobVoice->addItem(name);
    }
    ui->cobVoice->setCurrentIndex(selected);
}
