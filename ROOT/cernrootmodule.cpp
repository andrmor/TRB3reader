#include "cernrootmodule.h"
#include "trb3datareader.h"
#include "trb3signalextractor.h"
#include "channelmapper.h"
#include "agraphwindow.h"
#include "ajsontools.h"
#include "tmpobjhubclass.h"
#include "masterconfig.h"

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

CernRootModule::CernRootModule(Trb3dataReader *Reader, Trb3signalExtractor *Extractor, MasterConfig *Config, int refreshInterval) :
    Reader(Reader), Extractor(Extractor), Config(Config)
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
       win->setGeometry(x,y,w,h);
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

    int numSamples = Reader->GetNumSamples();
    TH2D* h = new TH2D("", "",
                       2*numSamples, -numSamples, numSamples,
                       100, -0.05, 1.05);

    int numEvents = Reader->GetNumEvents();
    int numChan = Config->Map->GetNumLogicalChannels();

    for (int ievent=0; ievent<numEvents; ievent++)
        if (!Extractor->IsRejectedEventFast(ievent))
        for (int ilc=0; ilc<numChan; ilc++)
        {
            int iHardwCh = Config->Map->LogicalToHardware(ilc);
            if ( iHardwCh < 0 ) continue;
            if (Config->IgnoreHardwareChannels.contains(iHardwCh)) continue;
            if (bNeg != Config->IsNegative(iHardwCh)) continue;

            bool bRejected;
            int sig = Extractor->extractSignalFromWaveform(ievent, iHardwCh, &bRejected);
            if (bRejected) continue;

            if (bNeg) sig = -sig;

            const QVector<double>* wave = Reader->GetWaveformPtrFast(ievent, iHardwCh);

            //int sum = 0;
            //for (int i=0; i<numSamples; i++) sum += wave->at(i);
            //if (sum == 0) sum = 1.0;
            //else sum = fabs(sum);

            double targetVal = 0.5*sig;
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
                const double val = 1.0*wave->at(i)/sig;// sum;
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
        (*win)->show();
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
}

void CernRootModule::DrawSingle(int ievent, int iHardwChan, bool autoscale, double MinY, double MaxY)
{
    int numSamples = Reader->GetNumSamples();
    if (numSamples==0) return;

    if (gSingle) delete gSingle;
    gSingle = new TGraph();

    for (int isam=0; isam<numSamples; isam++)
    {
        const double val = Reader->GetValueFast(ievent, iHardwChan, isam);
        gSingle->SetPoint(isam, isam, val);
    }

    if (!autoscale) gSingle->GetYaxis()->SetRangeUser(MinY, MaxY);

    gSingle->SetLineWidth(LineWidth);
    bool bRejected;
    Extractor->extractSignalFromWaveform(ievent, iHardwChan, &bRejected);

    bool bIgnoredChannel = Config->IgnoreHardwareChannels.contains(iHardwChan);
    gSingle->SetLineColor( (bIgnoredChannel || Extractor->IsRejectedEvent(ievent) || bRejected) ? RejectedColor : NormalColor);
    if (bIgnoredChannel) gSingle->SetLineStyle(7);

    WOne->SetAsActiveRootWindow();
    gSingle->Draw("AL");
    WOne->UpdateRootCanvas();
    WOne->SetTitle("Event: "+ QString::number(ievent) + "  LogicalChannel: "+QString::number(Config->Map->HardwareToLogical(iHardwChan)));
}

void CernRootModule::DrawOverlay(int ievent, bool bNeg, bool bAutoscale, double Min, double Max, int SortBy_0Logic1Hardw)
{
    int numSamples = Reader->GetNumSamples();
    if (numSamples==0) return;

    if (multiGraph) delete multiGraph;
    multiGraph = new TMultiGraph();

    int numChan = ( SortBy_0Logic1Hardw==0 ? Config->Map->GetNumLogicalChannels() : Reader->GetNumChannels() );

    for (int iCh=0; iCh<numChan; ++iCh)
    {
        int iHardwCh;
        if (SortBy_0Logic1Hardw == 0)
        {
            iHardwCh = Config->Map->LogicalToHardware(iCh);
            if ( iHardwCh < 0 ) continue;
        }
        else iHardwCh = iCh;

        if (bNeg != Config->IsNegative(iHardwCh)) continue;

        TGraph* g = new TGraph();

        for (int isam=0; isam<numSamples; isam++)
        {
            const double val = Reader->GetValueFast(ievent, iHardwCh, isam);
            g->SetPoint(isam, isam, val);
        }

        g->SetLineWidth(LineWidth);
        bool bRejected;
        Extractor->extractSignalFromWaveform(ievent, iHardwCh, &bRejected);

        bool bIgnoredChannel = Config->IgnoreHardwareChannels.contains(iHardwCh);
        g->SetLineColor( (bIgnoredChannel || Extractor->IsRejectedEvent(ievent) || bRejected) ? RejectedColor : NormalColor);
        if (bIgnoredChannel) g->SetLineStyle(7);

        multiGraph->Add(g, "AL");
    }

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
}

void CernRootModule::DrawAll(int ievent, bool bNeg, int padsX, int padsY, bool bAutoscale, double Min, double Max, int SortBy_0Logic1Hardw, bool bShowlabels)
{
    int numSamples = Reader->GetNumSamples();
    if (numSamples==0) return;

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
    c->Divide(padsX, padsY, 0, 0.000001);
    gPad->Modified();

    int numChannels = ( SortBy_0Logic1Hardw==0 ? Config->Map->GetNumLogicalChannels() : Reader->GetNumChannels() );
    if (bAutoscale)
    {
        Min = 1e20;
        Max = -1e20;
        for (int iCh=0; iCh<numChannels; ++iCh)
        {
            int iHardwCh;
            if (SortBy_0Logic1Hardw == 0)
            {
                iHardwCh = Config->Map->LogicalToHardware(iCh);
                if ( iHardwCh < 0 ) continue;
            }
            else iHardwCh = iCh;

            if (bNeg != Config->IsNegative(iHardwCh)) continue;

            for (int isam=0; isam<numSamples; isam++)
            {
                const double val = Reader->GetValueFast(ievent, iHardwCh, isam);
                if (val < Min) Min = val;
                if (val > Max) Max = val;
            }
        }

        //adding margins
        double delta = 0.05* (Max - Min);
        Max += delta;
        Min -= delta;

    }

    int iCounter = 1;
    for (int iCh=0; iCh<numChannels; ++iCh)
    {
        int iHardwCh;
        if (SortBy_0Logic1Hardw == 0)
        {
            iHardwCh = Config->Map->LogicalToHardware(iCh);
            if ( iHardwCh < 0 ) continue;
        }
        else iHardwCh = iCh;

        if (bNeg != Config->IsNegative(iHardwCh)) continue;

        TGraph* g = new TGraph();

        for (int isam=0; isam<numSamples; isam++)
        {
            const double val = Reader->GetValueFast(ievent, iHardwCh, isam);
            g->SetPoint(isam, isam, val);
        }

        c->cd(iCounter);
        iCounter++;

        g->SetLineWidth(LineWidth);
        bool bRejected;
        Extractor->extractSignalFromWaveform(ievent, iHardwCh, &bRejected);

        bool bIgnoredChannel = Config->IgnoreHardwareChannels.contains(iHardwCh);
        g->SetLineColor( (bIgnoredChannel || Extractor->IsRejectedEvent(ievent) || bRejected) ? RejectedColor : NormalColor);
        if (bIgnoredChannel) g->SetLineStyle(7);

        g->Draw("AL");
        g->SetMinimum(Min);
        g->SetMaximum(Max);

        //channel labels
        if (bShowlabels)
        {
            double delta = fabs(0.25*(Max - Min));
            TPaveText* la = new TPaveText(0, Min+delta, numSamples, Max-delta);
            la->SetFillColor(0);
            la->SetFillStyle(0);
            la->SetBorderSize(0);
            la->SetLineColor(1);
            la->SetTextAlign(22);

            la->AddText(QString::number(iCh).toLatin1());
            la->Draw("same");
        }
        //gPad->Update();

        gs->append(g);
    }

    win->UpdateRootCanvas();
    QString title = QString(bNeg ? "Negative" : "Positive") + " polarity waveforms";
    win->SetTitle(title);
}
