#include "abufferdelegate.h"
#include "ui_abufferdelegate.h"

#include <QSpinBox>

ABufferDelegate::ABufferDelegate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ABufferDelegate)
{
    ui->setupUi(this);

    QObject::connect(ui->sbSamples, &QSpinBox::editingFinished, this, &ABufferDelegate::onSamplesMaybeChanged);
    QObject::connect(ui->sbDelay, &QSpinBox::editingFinished, this, &ABufferDelegate::onDelayMaybeChanged);
    QObject::connect(ui->sbDownsampling, &QSpinBox::editingFinished, this, &ABufferDelegate::onDownMaybeChanged);
}

ABufferDelegate::~ABufferDelegate()
{
    delete ui;
}

void ABufferDelegate::setValues(int address, int samples, int delay, int downsampl)
{
    Address = address;
    Samples = samples;
    Delay = delay;
    Downsampling = downsampl;

    ui->labAddress->setText( QString::number(address, 16) );
    ui->sbSamples->setValue(samples);
    ui->sbDelay->setValue(delay);
    ui->sbDownsampling->setValue(downsampl + 1);
}

void ABufferDelegate::getValues(int & address, int & samples, int & delay, int & downsampl)
{
    Address = address = ui->labAddress->text().toInt(nullptr, 16);
    Samples = samples = ui->sbSamples->value();
    Delay = delay = ui->sbDelay->value();
    Downsampling = downsampl = ui->sbDownsampling->value() - 1;
}

QWidget * ABufferDelegate::getWidget()
{
    return this;
}

void ABufferDelegate::onSamplesMaybeChanged()
{
    int newSamples = ui->sbSamples->value();
    if (newSamples != Samples)
    {
        Samples = newSamples;
        emit contentChanged(this);
    }
}

void ABufferDelegate::onDelayMaybeChanged()
{
    int newDelay = ui->sbDelay->value();
    if (newDelay != Delay)
    {
        Delay = newDelay;
        emit contentChanged(this);
    }
}

void ABufferDelegate::onDownMaybeChanged()
{
    int newDownsampling = ui->sbDownsampling->value() - 1;
    if (newDownsampling != Downsampling)
    {
        Downsampling = newDownsampling;
        emit contentChanged(this);
    }
}
