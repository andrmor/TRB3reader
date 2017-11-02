#include "cernrootmodule.h"
#include "trb3datareader.h"
#include "trb3signalextractor.h"
#include "channelmapper.h"
#include "agraphwindow.h"
#include "ajsontools.h"

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

CernRootModule::CernRootModule(Trb3dataReader *Reader, Trb3signalExtractor *Extractor, ChannelMapper *Map, int refreshInterval) :
    Reader(Reader), Extractor(Extractor), Map(Map)
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
        const double val = Reader->GetValue(ievent, iHardwChan, isam);
        gSingle->SetPoint(isam, isam, val);
    }

    if (!autoscale) gSingle->GetYaxis()->SetRangeUser(MinY, MaxY);

    gSingle->SetLineWidth(LineWidth);
    bool bRejected;
    Extractor->extractSignal_SingleChannel(ievent, iHardwChan, &bRejected);
    gSingle->SetLineColor( bRejected ? RejectedColor : NormalColor);

    WOne->SetAsActiveRootWindow();
    gSingle->Draw("AL");
    WOne->UpdateRootCanvas();
    WOne->SetTitle("Event: "+ QString::number(ievent) + "  LogicalChannel: "+QString::number(Map->HardwareToLogical(iHardwChan)));
}

void CernRootModule::DrawOverlay(int ievent, bool bNeg, bool bAutoscale, double Min, double Max, int SortBy_0Logic1Hardw)
{
    int numSamples = Reader->GetNumSamples();
    if (numSamples==0) return;

    if (multiGraph) delete multiGraph;
    multiGraph = new TMultiGraph();

    int numChan = ( SortBy_0Logic1Hardw==0 ? Map->GetNumLogicalChannels() : Reader->GetNumChannels() );

    for (int iCh=0; iCh<numChan; ++iCh)
    {
        int iHardwCh;
        if (SortBy_0Logic1Hardw == 0)
        {
            iHardwCh = Map->LogicalToHardware(iCh);
            if (std::isnan(iHardwCh)) continue;
        }
        else iHardwCh = iCh;

        if (bNeg != Extractor->IsNegative(iHardwCh)) continue;

        TGraph* g = new TGraph();

        for (int isam=0; isam<numSamples; isam++)
        {
            const double val = Reader->GetValue(ievent, iHardwCh, isam);
            g->SetPoint(isam, isam, val);
        }

        g->SetLineWidth(LineWidth);
        bool bRejected;
        Extractor->extractSignal_SingleChannel(ievent, iHardwCh, &bRejected);
        g->SetLineColor( bRejected ? RejectedColor : NormalColor);

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

    int numChannels = ( SortBy_0Logic1Hardw==0 ? Map->GetNumLogicalChannels() : Reader->GetNumChannels() );
    if (bAutoscale)
    {
        Min = 1e20;
        Max = -1e20;
        for (int iCh=0; iCh<numChannels; ++iCh)
        {
            int iHardwCh;
            if (SortBy_0Logic1Hardw == 0)
            {
                iHardwCh = Map->LogicalToHardware(iCh);
                if (std::isnan(iHardwCh)) continue;
            }
            else iHardwCh = iCh;

            if (bNeg != Extractor->IsNegative(iHardwCh)) continue;

            for (int isam=0; isam<numSamples; isam++)
            {
                const double val = Reader->GetValue(ievent, iHardwCh, isam);
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
            iHardwCh = Map->LogicalToHardware(iCh);
            if (std::isnan(iHardwCh)) continue;
        }
        else iHardwCh = iCh;

        if (bNeg != Extractor->IsNegative(iHardwCh)) continue;

        TGraph* g = new TGraph();

        for (int isam=0; isam<numSamples; isam++)
        {
            const double val = Reader->GetValue(ievent, iHardwCh, isam);
            g->SetPoint(isam, isam, val);
        }

        c->cd(iCounter);
        iCounter++;

        g->SetLineWidth(LineWidth);
        bool bRejected;
        Extractor->extractSignal_SingleChannel(ievent, iHardwCh, &bRejected);
        g->SetLineColor( bRejected ? RejectedColor : NormalColor);

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
