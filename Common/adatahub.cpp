#include "adatahub.h"
#include "masterconfig.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QApplication>

#include <limits>
const float NaN = std::numeric_limits<float>::quiet_NaN();

void ADataHub::RemoveEvent(int ievent)
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return;
    Events.removeAt(ievent);
}

const AOneEvent *ADataHub::GetEvent(int ievent) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent);
}

AOneEvent *ADataHub::GetEvent(int ievent)
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events[ievent];
}

float AOneEvent::GetSignal(int ichannel) const
{
    if (ichannel<0 || ichannel>=Signals.size()) return NaN;
    return Signals.at(ichannel);
}

bool AOneEvent::SetSignal(int ichannel, float value)
{
    if (ichannel<0 || ichannel>=Signals.size()) return false;
    Signals[ichannel] = value;
    return true;
}

void AOneEvent::ClearWaveforms()
{
    for (QVector<float>* vec : Waveforms) delete vec;
}

const QVector<float> *AOneEvent::GetWaveform(int ichannel) const
{
    if (ichannel<0 || ichannel>=Waveforms.size()) return 0;
    return Waveforms.at(ichannel);
}

float AOneEvent::GetWaveformMax(int ichannel) const
{
    if (ichannel<0 || ichannel>=Waveforms.size()) return NaN;

    const QVector <float>* vec = Waveforms.at(ichannel);
    if (!vec || vec->isEmpty()) return NaN;

    float max = vec->at(0);
    for (int i=1; i<vec->size(); i++)
        if ( vec->at(i) > max ) max = vec->at(i);
    return max;
}

float AOneEvent::GetWaveformMin(int ichannel) const
{
    if (ichannel<0 || ichannel>=Waveforms.size()) return NaN;

    const QVector <float>* vec = Waveforms.at(ichannel);
    if (!vec || vec->isEmpty()) return NaN;

    float min = vec->at(0);
    for (int i=1; i<vec->size(); i++)
        if ( vec->at(i) < min ) min = vec->at(i);
    return min;
}

int AOneEvent::GetWaveformMaxSample(int ichannel) const
{
    if (ichannel<0 || ichannel>=Waveforms.size()) return -1;

    const QVector <float>* vec = Waveforms.at(ichannel);
    if (!vec || vec->isEmpty()) return -1;

    int   imax = 0;
    float max = vec->at(0);
    for (int i=1; i<vec->size(); i++)
        if (vec->at(i) > max)
        {
            imax = i;
            max  = vec->at(i);
        }
    return imax;
}

int AOneEvent::GetWaveformMinSample(int ichannel) const
{
    if (ichannel<0 || ichannel>=Waveforms.size()) return -1;

    const QVector <float>* vec = Waveforms.at(ichannel);
    if (!vec || vec->isEmpty()) return -1;

    int   imin = 0;
    float min = vec->at(0);
    for (int i=1; i<vec->size(); i++)
        if (vec->at(i) < min)
        {
            imin = i;
            min  = vec->at(i);
        }
    return imin;
}

int AOneEvent::GetWaveformSampleWhereFirstBelow(int ichannel, float threshold) const
{
    if (ichannel<0 || ichannel>=Waveforms.size()) return -1;

    const QVector <float>* vec = Waveforms.at(ichannel);
    if (!vec || vec->isEmpty()) return -1;

    for (int i=0; i<vec->size(); i++)
        if (vec->at(i) < threshold) return i;
    return -1;
}

int AOneEvent::GetWaveformSampleWhereFirstAbove(int ichannel, float threshold) const
{
    if (ichannel<0 || ichannel>=Waveforms.size()) return -1;

    const QVector <float>* vec = Waveforms.at(ichannel);
    if (!vec || vec->isEmpty()) return -1;

    for (int i=0; i<vec->size(); i++)
        if (vec->at(i) > threshold) return i;
    return -1;
}

ADataHub::ADataHub(const MasterConfig &Config) : Config(Config) {}

ADataHub::~ADataHub()
{
    Clear();
}

void ADataHub::Clear()
{
    QMutexLocker ml(&Mutex);
    for (AOneEvent* event : Events)
    {
        event->ClearWaveforms();
        delete event;
    }
    Events.clear();
}

void ADataHub::AddEvent(AOneEvent *Event)
{
    QMutexLocker ml(&Mutex);
    Events << Event;
}

int ADataHub::CountChannels()
{
    QMutexLocker ml(&Mutex);
    if (Events.isEmpty()) return 0;
    return Events.at(0)->CountChannels();
}

float ADataHub::GetSignal(int ievent, int ichannel)
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return NaN;
    const AOneEvent* event = Events.at(ievent);
    return event->GetSignal(ichannel);
}

float ADataHub::GetSignalFast(int ievent, int ichannel)
{
    return Events.at(ievent)->GetSignalFast(ichannel);
}

const QVector<float> *ADataHub::GetSignals(int ievent) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent)->GetSignals();
}

const QVector<float> *ADataHub::GetSignalsFast(int ievent) const
{
    return Events.at(ievent)->GetSignals();
}

bool ADataHub::SetSignal(int ievent, int iLogicalChannel, float value)
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return false;
    return Events[ievent]->SetSignal(iLogicalChannel, value);
}

void ADataHub::SetSignalFast(int ievent, int iLogicalChannel, float value)
{
    Events[ievent]->SetSignalFast(iLogicalChannel, value);
}

bool ADataHub::SetSignals(int ievent, const QVector<float> *vector)
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetSignals(vector);
    return true;
}

void ADataHub::SetSignalsFast(int ievent, const QVector<float> *vector)
{
    Events[ievent]->SetSignals(vector);
}

bool ADataHub::IsRejected(int ievent) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return true;
    return Events.at(ievent)->IsRejected();
}

bool ADataHub::IsRejectedFast(int ievent) const
{
    return Events.at(ievent)->IsRejected();
}

bool ADataHub::SetRejectedFlag(int ievent, bool flag)
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetRejectedFlag(flag);
    return true;
}

void ADataHub::SetRejectedFlagFast(int ievent, bool flag)
{
    Events[ievent]->SetRejectedFlag(flag);
}

void ADataHub::SetAllRejectedFlag(bool flag)
{
    QMutexLocker ml(&Mutex);
    for (AOneEvent* event : Events) event->SetRejectedFlag(flag);
}

const float *ADataHub::GetPosition(int ievent) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent)->GetPosition();
}

const float *ADataHub::GetPositionFast(int ievent) const
{
    return Events.at(ievent)->GetPosition();
}

bool ADataHub::SetPosition(int ievent, const float *XYZ)
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetPosition(XYZ);
    return true;
}

bool ADataHub::SetPosition(int ievent, float x, float y, float z)
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetPosition(x, y, z);
    return true;
}

void ADataHub::SetPositionFast(int ievent, const float *XYZ)
{
    Events[ievent]->SetPosition(XYZ);
}

void ADataHub::SetPositionFast(int ievent, float x, float y, float z)
{
    Events[ievent]->SetPosition(x, y, z);
}

const QVector<QVector<float> *>* ADataHub::GetWaveforms(int ievent) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent)->GetWaveforms();
}

const QVector<float>* ADataHub::GetWaveform(int ievent, int ichannel) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent)->GetWaveform(ichannel);
}

const QVector<float> *ADataHub::GetWaveformFast(int ievent, int ichannel) const
{
    return Events.at(ievent)->GetWaveformFast(ichannel);
}

float ADataHub::GetWaveformMax(int ievent, int ichannel) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return NaN;
    return Events.at(ievent)->GetWaveformMax(ichannel);
}

float ADataHub::GetWaveformMin(int ievent, int ichannel) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return NaN;
    return Events.at(ievent)->GetWaveformMin(ichannel);
}

int ADataHub::GetWaveformMaxSample(int ievent, int ichannel) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return -1;
    return Events.at(ievent)->GetWaveformMaxSample(ichannel);
}

int ADataHub::GetWaveformMinSample(int ievent, int ichannel) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return -1;
    return Events.at(ievent)->GetWaveformMinSample(ichannel);
}

int ADataHub::GetWaveformSampleWhereFirstBelow(int ievent, int ichannel, float threshold) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return -1;
    return Events.at(ievent)->GetWaveformSampleWhereFirstBelow(ichannel, threshold);
}

int ADataHub::GetWaveformSampleWhereFirstAbove(int ievent, int ichannel, float threshold) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return -1;
    return Events.at(ievent)->GetWaveformSampleWhereFirstAbove(ichannel, threshold);
}

const int *ADataHub::GetMultiplicityPositive(int ievent) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent)->GetMultiplicitiesPositive();
}

const int *ADataHub::GetMultiplicityPositiveFast(int ievent) const
{
    return Events.at(ievent)->GetMultiplicitiesPositive();
}

const int *ADataHub::GetMultiplicityNegative(int ievent) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent)->GetMultiplicitiesNegative();
}

const int *ADataHub::GetMultiplicityNegativeFast(int ievent) const
{
    return Events.at(ievent)->GetMultiplicitiesNegative();
}

bool ADataHub::SetMultiplicityPositive(int ievent, const int *multi)
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetMultiplicitiesPositive(multi);
    return true;
}

void ADataHub::SetMultiplicityPositiveFast(int ievent, const int *multi)
{
    Events[ievent]->SetMultiplicitiesPositive(multi);
}

bool ADataHub::SetMultiplicityNegative(int ievent, const int *multi)
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetMultiplicitiesNegative(multi);
    return true;
}

void ADataHub::SetMultiplicityNegativeFast(int ievent, const int *multi)
{
    Events[ievent]->SetMultiplicitiesNegative(multi);
}

const float *ADataHub::GetSumSignalPositive(int ievent) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent)->GetSumSigPositive();
}

const float *ADataHub::GetSumSignalPositiveFast(int ievent) const
{
    return Events.at(ievent)->GetSumSigPositive();
}

const float *ADataHub::GetSumSignalNegative(int ievent) const
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return 0;
    return Events.at(ievent)->GetSumSigNegative();
}

const float *ADataHub::GetSumSignalNegativeFast(int ievent) const
{
    return Events.at(ievent)->GetSumSigNegative();
}

bool ADataHub::SetSumSignalPositive(int ievent, const float *sums)
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetSumSigPositive(sums);
    return true;
}

void ADataHub::SetSumSignalPositiveFast(int ievent, const float *sums)
{
    Events[ievent]->SetSumSigPositive(sums);
}

bool ADataHub::SetSumSignalNegative(int ievent, const float *sums)
{
    QMutexLocker ml(&Mutex);
    if (ievent<0 || ievent>=Events.size()) return false;
    Events[ievent]->SetSumSigNegative(sums);
    return true;
}

void ADataHub::SetSumSignalNegativeFast(int ievent, const float *sums)
{
    Events[ievent]->SetSumSigNegative(sums);
}

const QString ADataHub::Save(const QString& FileName, bool bSavePositions, bool bSkipRejected)
{
    QMutexLocker ml(&Mutex);
    if ( Events.isEmpty())   return "There are no events in the DataHub!";
    if ( FileName.isEmpty()) return "File name is empty!";

    QFile outFile( FileName ); outFile.open(QIODevice::WriteOnly);
    if(!outFile.isOpen())    return "Unable to open file " +FileName+ " for writing!";

    QTextStream outStream(&outFile);
    for (const AOneEvent* e : Events)
    {
        if (bSkipRejected && e->IsRejected()) continue;

        const QVector<float>* vec = e->GetSignals();
        for (float val : *vec) outStream << QString::number(val) << " ";
        if (bSavePositions)
        {
            const float* R = e->GetPosition();
            outStream << "     " << R[0] << " " << R[1] << " " << R[2];
        }
        outStream << "\r\n";
    }

    return "";
}

const QString ADataHub::Load(const QString &AppendFromFileName, bool bLoadPositionXYZ)
{
    QMutexLocker ml(&Mutex);
    QFile inFile( AppendFromFileName );
    inFile.open(QIODevice::ReadOnly);
    if(!inFile.isOpen()) return "Unable to open file " +AppendFromFileName+ " for reading!";

    QTextStream inStream(&inFile);

    int numChannels = Config.CountLogicalChannels();
    int upperLim = numChannels;
    if (bLoadPositionXYZ) upperLim += 3;
    int numEvents = 0;
    qint64 totSize = QFileInfo(AppendFromFileName).size();
    while (!inStream.atEnd())  // optimized assuming expected format of the file
    {
        const QString s = inStream.readLine();

        if (numEvents % 200 == 0)
        {
            emit reportProgress(100.0 * inStream.pos() / totSize);
            qApp->processEvents();
        }

        QRegExp rx("(\\ |\\,|\\:|\\t)");
        QStringList fields = s.split(rx, QString::SkipEmptyParts);
        if (fields.size() < upperLim) continue;

        QVector<float>* vec = new QVector<float>(numChannels);
        for (int i=0; i<numChannels; i++)
        {
            bool bOK;
            const QString f = fields.at(i);
            float val = f.toFloat(&bOK);
            if (!bOK)
            {
                delete vec;
                continue;
            }
            (*vec)[i] = val;
        }

        float xyz[3];
        if (bLoadPositionXYZ)
        {

            bool bOK;
            for (int i=0; i<3; i++)
            {
                const QString ss = fields.at(numChannels+i);
                xyz[i] = ss.toFloat(&bOK);
                if (!bOK)
                {
                    delete vec;
                    continue;
                }
            }
        }

        AOneEvent* ev = new AOneEvent();
        ev->SetSignals(vec);  // transfer ownership!
        if (bLoadPositionXYZ) ev->SetPosition(xyz);

        AddEventFast(ev);
        numEvents++;
    }

    emit requestGuiUpdate();

    if (numEvents == 0) return "No events were added from file " + AppendFromFileName + "!";
    else                return "";
}
