#ifndef ABUFFERDELEGATE_H
#define ABUFFERDELEGATE_H

#include <QWidget>

namespace Ui {
class ABufferDelegate;
}

class ABufferDelegate : public QWidget
{
    Q_OBJECT

public:
    explicit ABufferDelegate(QWidget *parent = 0);
    ~ABufferDelegate();

    void setValues(int address, int samples, int delay, int downsampl);
    void getValues(int & address, int & samples, int & delay, int & downsampl);

    QWidget * getWidget();

private slots:
    void onSamplesMaybeChanged();
    void onDelayMaybeChanged();
    void onDownMaybeChanged();

signals:
    void contentChanged(ABufferDelegate *);

private:
    Ui::ABufferDelegate * ui;

    int Address;
    int Samples;
    int Delay;
    int Downsampling;
};

#endif // ABUFFERDELEGATE_H
