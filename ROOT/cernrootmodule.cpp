#include "cernrootmodule.h"
#include "trb3datareader.h"
#include "trb3signalextractor.h"
#include "channelmapper.h"
#include "agraphwindow.h"
#include "ajsontools.h"
#include "tmpobjhubclass.h"
#include "masterconfig.h"
#include "adatahub.h"

#include <QTimer>
#include <QDebug>

#include "TApplication.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TGraph.h"
#include "TPad.h"
#include "TAxis.h"
#include "TMultiGraph.h"
#include "TList.h"
#include "TPaveText.h"
#include "TH2D.h"
#include "TROOT.h"

CernRootModule::CernRootModule(Trb3dataReader *Reader, Trb3signalExtractor *Extractor, MasterConfig *Config, ADataHub* DataHub, int refreshInterval) :
    Reader(Reader), Extractor(Extractor), Config(Config), DataHub(DataHub)
{
    //create ROOT application
    int rootargc=1;
    char *rootargv[] = {(char*)"qqq"};
    RootApp = new TApplication("My ROOT", &rootargc, rootargv);
    //qDebug() << "->ROOT application activated";
    QTimer* RootUpdateTimer = new QTimer(this); //will be auto-deleted beacuse of the parent
    RootUpdateTimer->setInterval(refreshInterval);
    QObject::connect(RootUpdateTimer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
    RootUpdateTimer->start();
    //qDebug()<<"->Timer to refresh Root events started";

    qDebug() << "Running CERN ROOT version"<<gROOT->GetVersion();

    WOne = WOverNeg = WOverPos = WAllNeg = WAllPos = 0;
    StartGraphWindows();

    gSingle = 0;
    multiGraph = 0;

    NormalColor = 4;
    RejectedColor = 2;
    LineWidth = 2;

    TmpHub = new TmpObjHubClass();
}

const QJsonObject saveWindowProperties(AGraphWindow* w)
{
    return SaveWindowToJson(w->x(), w->y(), w->width(), w->height(), w->isVisible());
}

const QJsonObject CernRootModule::SaveGraphWindows() const
{
    QJsonObject js;
    js["Single"]  = saveWindowProperties(WOne);
    js["OverNeg"] = saveWindowProperties(WOverNeg);
    js["OverPos"] = saveWindowProperties(WOverPos);
    js["AllNeg"]  = saveWindowProperties(WAllNeg);
    js["AllPos"]  = saveWindowProperties(WAllPos);
    return js;
}

void setWinGeometry(AGraphWindow* win, QString name, const QJsonObject& props)
{
    int x = 500, y = 10, w = 1000, h = 800;
    bool bVis;
    QJsonObject js = props[name].toObject();
    if (!js.isEmpty())
    {
       LoadWindowFromJson(js, x, y, w, h, bVis);

       win->move(x, y);
       win->resize(w, h);
       //win->setGeometry(x,y,w,h);  // introduces a shift up on Windows7

       //if (bVis) win->show();
       //else win->hide();
    }
}

void CernRootModule::SetWindowGeometries(const QJsonObject &js)
{
    setWinGeometry(WOne, "Single", js);
    setWinGeometry(WOverNeg, "OverNeg", js);
    setWinGeometry(WOverPos, "OverPos", js);
    setWinGeometry(WAllNeg, "AllNeg", js);
    setWinGeometry(WAllPos, "AllPos", js);
}

void CernRootModule::ResetPositionOfWindows()
{
    WOne->setGeometry(20,20,1000,700);
    WOverNeg->setGeometry(50,50,1000,700);
    WOverPos->setGeometry(80,80,1000,700);
    WAllNeg->setGeometry(110,110,1000,700);
    WAllPos->setGeometry(140,140,1000,700);
}

void CernRootModule::DrawSignature(bool bNeg)
{
    WOne->ShowAndFocus();

    int numSamples = Reader->CountSamples();
    TH2D* h = new TH2D("", "",
                       2*numSamples, -numSamples, numSamples,
                       100, -0.05, 1.05);

    int numEvents = Reader->CountEvents();
    int numChan = Config->CountLogicalChannels();

    for (int ievent=0; ievent<numEvents; ievent++)
        if (!Extractor->IsRejectedEventFast(ievent))
        for (int ilc=0; ilc<numChan; ilc++)
        {
            int iHardwCh = Config->Map->LogicalToHardware(ilc);
            if ( iHardwCh < 0 ) continue;
            if (Config->IsIgnoredHardwareChannel(iHardwCh)) continue;
            if (bNeg != Config->IsNegativeHardwareChannel(iHardwCh)) continue;

            bool bRejected;
            int sig = Extractor->extractSignalFromWaveform(ievent, iHardwCh, &bRejected);
            if (bRejected) continue;

            if (bNeg) sig = -sig;

            const QVector<float>* wave = Reader->GetWaveformPtrFast(ievent, iHardwCh);

            //int sum = 0;
            //for (int i=0; i<numSamples; i++) sum += wave->at(i);
            //if (sum == 0) sum = 1.0;
            //else sum = fabs(sum);

            float targetVal = 0.5*sig;
            int iHalf = 0;
            for (; iHalf<numSamples; iHalf++)
            {
                if (bNeg)
                {
                    if (wave->at(iHalf) < -targetVal) break;
                }
                else
                {
                    if (wave->at(iHalf) > targetVal) break;
                }
            }

            for (int i=0; i<numSamples; i++)
            {
                const float val = 1.0*wave->at(i)/sig;// sum;
                h->Fill(i-iHalf, val);
            }
        }
    h->Draw("colz");
    WOne->UpdateRootCanvas();
}

void CernRootModule::onDrawRequested(TObject *obj, QString opt, bool bDoUpdate)
{
    //WOne->SetAsActiveRootWindow();
    WOne->ShowAndFocus();

    if (!obj)
    {
        WOne->UpdateRootCanvas();
        return;
    }

    obj->Draw(opt.toLatin1().data());
    if (bDoUpdate) WOne->UpdateRootCanvas();
}

void CernRootModule::StartGraphWindows()
{
    QList<AGraphWindow**> ws;
    ws << &WOne << &WOverNeg << &WOverPos << &WAllNeg << &WAllPos;
    for (AGraphWindow** w : ws)
    {
        (*w) = new AGraphWindow();
        (*w)->resize(1001, 601);
    }
    connect(WOne, SIGNAL(WasHidden()), this, SIGNAL(WOneHidden()));
    connect(WOverNeg, SIGNAL(WasHidden()), this, SIGNAL(WOverNegHidden()));
    connect(WOverPos, SIGNAL(WasHidden()), this, SIGNAL(WOverPosHidden()));
    connect(WAllNeg, SIGNAL(WasHidden()), this, SIGNAL(WAllNegHidden()));
    connect(WAllPos, SIGNAL(WasHidden()), this, SIGNAL(WAllPosHidden()));
}

CernRootModule::~CernRootModule()
{
    delete WOne; delete WOverNeg; delete WOverPos; delete WAllNeg; delete WAllPos;
    WOne = WOverNeg = WOverPos = WAllNeg = WAllPos = 0;

    delete gSingle; gSingle = 0;
    delete multiGraph; multiGraph = 0;

    clearNegGraphVectors();
    clearPosGraphVectors();

    delete TmpHub;

    delete RootApp;
}

void CernRootModule::timerTimeout()
{
    gSystem->ProcessEvents();
}

void CernRootModule::showGraphWindow(AGraphWindow** win, bool flag)
{
    if (flag)
    {
        if (!(*win)) (*win) = new AGraphWindow();
        (*win)->showNormal();
        (*win)->activateWindow();
    }
    else
    {
        if (*win) (*win)->hide();
    }
}

void CernRootModule::clearNegGraphVectors()
{
    for (TGraph* g : graphsNeg) delete g;  graphsNeg.clear();
}

void CernRootModule::clearPosGraphVectors()
{
    for (TGraph* g : graphsPos) delete g;  graphsPos.clear();
}

void CernRootModule::ShowSingleWaveWindow(bool flag)
{
    showGraphWindow(&WOne, flag);
}

void CernRootModule::ShowOverNegWaveWindow(bool flag)
{
    showGraphWindow(&WOverNeg, flag);
}

void CernRootModule::ShowOverPosWaveWindow(bool flag)
{
    showGraphWindow(&WOverPos, flag);
}

void CernRootModule::ShowAllNegWaveWindow(bool flag)
{
    showGraphWindow(&WAllNeg, flag);
}

void CernRootModule::ShowAllPosWaveWindow(bool flag)
{
    showGraphWindow(&WAllPos, flag);
}

void CernRootModule::ClearSingleWaveWindow()
{
    if (!WOne) return; //paranoic
    WOne->ClearRootCanvas();
    WOne->UpdateRootCanvas();
    WOne->SetTitle("");
}

void CernRootModule::ClearOverNegWaveWindow()
{
    if (!WOverNeg) return; //paranoic
    WOverNeg->ClearRootCanvas();
    WOverNeg->UpdateRootCanvas();
}

void CernRootModule::ClearOverPosWaveWindow()
{
    if (!WOverPos) return; //paranoic
    WOverPos->ClearRootCanvas();
    WOverPos->UpdateRootCanvas();
}

void CernRootModule::ClearAllNegWaveWindow()
{
    if (!WAllNeg) return; //paranoic
    WAllNeg->ClearRootCanvas();
    WAllNeg->UpdateRootCanvas();
}

void CernRootModule::ClearAllPosWaveWindow()
{
    if (!WAllPos) return; //paranoic
    WAllPos->ClearRootCanvas();
    WAllPos->UpdateRootCanvas();
}

void CernRootModule::SetGraphAttributes(TGraph* g, bool bFromDataHub, int ievent, int ichannel)
{
    bool bRejected;
    bool bIgnoredChannel;
    if (bFromDataHub)
    {
        bIgnoredChannel = Config->IsIgnoredLogicalChannel(ichannel);
        bRejected = DataHub->IsRejected(ievent) || bIgnoredChannel; // zero sig is not copied at all!
    }
    else
    {
        bIgnoredChannel = Config->IsIgnoredHardwareChannel(ichannel);
        Extractor->extractSignalFromWaveform(ievent, ichannel, &bRejected);
        bRejected = bRejected || bIgnoredChannel || Extractor->IsRejectedEvent(ievent);
    }

    g->SetLineColor(bRejected ? RejectedColor : NormalColor);
    if (bIgnoredChannel) g->SetLineStyle(7);
    g->SetLineWidth(LineWidth);
}

bool CernRootModule::DrawSingle(bool bFromDataHub, int ievent, int ichannel, bool autoscale, float MinY, float MaxY)
{
    const QVector<float>* wave = bFromDataHub ? DataHub->GetWaveform(ievent, ichannel) : Reader->GetWaveformPtr(ievent, ichannel);
    if (!wave) return false;

    if (gSingle) delete gSingle;
    gSingle = new TGraph();

    int isam = 0;
    for (const float& val : *wave)
    {
        gSingle->SetPoint(isam, isam, val);
        isam++;
    }

    if (!autoscale) gSingle->GetYaxis()->SetRangeUser(MinY, MaxY);
    SetGraphAttributes(gSingle, bFromDataHub, ievent, ichannel);

    WOne->SetAsActiveRootWindow();
    gSingle->Draw("AL");
    WOne->UpdateRootCanvas();

    int ic = bFromDataHub ? ichannel : Config->Map->HardwareToLogical(ichannel);
    WOne->SetTitle("Event: "+ QString::number(ievent) + "  LogicalChannel: "+QString::number(ic));
    return true;
}

bool CernRootModule::DrawOverlay(bool bFromDataHub, int ievent, bool bNeg, bool bAutoscale, float Min, float Max, int SortBy_0Logic1Hardw)
{
    if (multiGraph) delete multiGraph;
    multiGraph = new TMultiGraph();

    int numChan = ( (bFromDataHub || SortBy_0Logic1Hardw==0) ? Config->CountLogicalChannels() : Reader->CountChannels() );

    for (int iCh=0; iCh<numChan; ++iCh)
    {
        int iChannel;
        if (bFromDataHub) iChannel = iCh;
        else if (SortBy_0Logic1Hardw == 0)
        {
            iChannel = Config->Map->LogicalToHardware(iCh);
            if ( iChannel < 0 ) continue;
        }
        else iChannel = iCh;

        bool bPolarity = bFromDataHub ? Config->IsNegativeLogicalChannel(iChannel) : Config->IsNegativeHardwareChannel(iChannel);
        if (bNeg != bPolarity) continue;

        const QVector<float>* wave = bFromDataHub ? DataHub->GetWaveform(ievent, iChannel) : Reader->GetWaveformPtr(ievent, iChannel);
        if (!wave) continue;
        if (wave->isEmpty()) continue;

        TGraph* g = new TGraph();
        int isam = 0;
        for (const float& val : *wave)
        {
            g->SetPoint(isam, isam, val);
            isam++;
        }

        SetGraphAttributes(g, bFromDataHub, ievent, iChannel);
        multiGraph->Add(g, "AL");
    }

    if (!multiGraph->GetListOfGraphs() || multiGraph->GetListOfGraphs()->GetEntries() == 0) return false;

    AGraphWindow* win = bNeg ? WOverNeg : WOverPos;
    win->SetAsActiveRootWindow();
    multiGraph->Draw("AL");
    gPad->Modified();
    if (!bAutoscale)
    {
        multiGraph->SetMinimum(Min);
        multiGraph->SetMaximum(Max);
    }
    win->UpdateRootCanvas();
    QString title = QString(bNeg ? "Negative" : "Positive") + " polarity waveforms";
    win->SetTitle(title);
    return true;
}

bool CernRootModule::DrawAll(bool bFromDataHub, int ievent, bool bNeg, int padsX, int padsY, bool bAutoscale, float Min, float Max, int SortBy_0Logic1Hardw, bool bShowlabels, int Channels0_Signals1)
{
    AGraphWindow* win;
    QVector<TGraph*>* gs;

    if (bNeg)
    {
        clearNegGraphVectors();
        win = WAllNeg;
        gs = &graphsNeg;
    }
    else
    {
        clearPosGraphVectors();
        win = WAllPos;
        gs = &graphsPos;
    }

    win->SetAsActiveRootWindow();
    TCanvas* c = gPad->GetCanvas();
    c->Clear();
    c->Divide(padsX, padsY, 0, 0.000001f);
    gPad->Modified();

    int numChannels = ( (bFromDataHub || SortBy_0Logic1Hardw==0) ? Config->CountLogicalChannels() : Reader->CountChannels() );
    if (bAutoscale)
    {
        Min = 1e20f;
        Max = -1e20f;
        for (int iCh=0; iCh<numChannels; ++iCh)
        {
            int iChannel;
            if (bFromDataHub) iChannel = iCh;
            else if (SortBy_0Logic1Hardw == 0)
            {
                iChannel = Config->Map->LogicalToHardware(iCh);
                if ( iChannel < 0 ) continue;
            }
            else iChannel = iCh;

            bool bPolarity = bFromDataHub ? Config->IsNegativeLogicalChannel(iChannel) : Config->IsNegativeHardwareChannel(iChannel);
            if (bNeg != bPolarity) continue;

            const QVector<float>* wave = bFromDataHub ? DataHub->GetWaveform(ievent, iChannel) : Reader->GetWaveformPtr(ievent, iChannel);
            if (!wave) continue;
            if (wave->isEmpty()) continue;

            for (const float& val : *wave)
            {
                if (val < Min) Min = val;
                if (val > Max) Max = val;
            }
        }

        //adding margins
        float delta = 0.05 * (Max - Min);
        Max += delta;
        Min -= delta;
    }

    int NonZeroWaves = 0;
    int iCounter = 1;
    for (int iCh=0; iCh<numChannels; ++iCh)
    {
        int iChannel;
        if (bFromDataHub) iChannel = iCh;
        else if (SortBy_0Logic1Hardw == 0)
        {
            iChannel = Config->Map->LogicalToHardware(iCh);
            if ( iChannel < 0 ) continue;
        }
        else iChannel = iCh;

        bool bPolarity = bFromDataHub ? Config->IsNegativeLogicalChannel(iChannel) : Config->IsNegativeHardwareChannel(iChannel);

        if (bNeg != bPolarity) continue;

        c->cd(iCounter);
        iCounter++;

        bool bEmptyOne = false;
        const QVector<float>* wave = bFromDataHub ? DataHub->GetWaveform(ievent, iChannel) : Reader->GetWaveformPtr(ievent, iChannel);
        if (!wave) bEmptyOne = true;
        else if (wave->isEmpty()) bEmptyOne = true;

        if (!bEmptyOne)
        {
            TGraph* g = new TGraph();
            int isam = 0;
            for (const float& val : *wave)
            {
                g->SetPoint(isam, isam, val);
                isam++;
            }

            SetGraphAttributes(g, bFromDataHub, ievent, iChannel);
            g->Draw("AL");
            g->SetMinimum(Min);
            g->SetMaximum(Max);

            //channel labels
            if (bShowlabels)
            {
                float delta = fabs(0.25*(Max - Min));
                TPaveText* la = new TPaveText(0, Min+delta, isam, Max-delta);
                la->SetFillColor(0);
                la->SetFillStyle(0);
                la->SetBorderSize(0);
                la->SetLineColor(1);
                la->SetTextAlign(22);

                QString s;
                if (Channels0_Signals1 == 0)
                    s = QString::number(iCh);
                else
                {
                    double sig = bFromDataHub ? DataHub->GetSignalFast(ievent, iChannel) : Extractor->GetSignalFast(ievent, iChannel);
                    s = QString::number(sig, 'g', 4);
                }

                la->AddText(s.toLocal8Bit().data());
                la->Draw("same");
            }
            //gPad->Update();

            gs->append(g);
            NonZeroWaves++;
        }        
    }

    win->UpdateRootCanvas();
    QString title = QString(bNeg ? "Negative" : "Positive") + " polarity waveforms";
    win->SetTitle(title);

    return NonZeroWaves>0;
}
