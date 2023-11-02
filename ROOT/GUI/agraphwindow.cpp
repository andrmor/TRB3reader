#include "agraphwindow.h"
#include "ui_agraphwindow.h"
#include "arasterwindow.h"

#include <QJsonObject>
#include <QDebug>
#include <QHBoxLayout>

#include "TCanvas.h"

AGraphWindow::AGraphWindow(const QString & idStr, QWidget * parent) :
    QMainWindow(parent),
    ui(new Ui::AGraphWindow),
    IdStr(idStr)
{
    ui->setupUi(this);

    RasterWindow = new ARasterWindow(this);

    //RasterWindow->resize(width(), height());
    RasterWindow->resize(800, 500);
    RasterWindow->ForceResize();

    setCentralWidget(RasterWindow);

    restoreGeomStatus();
    hide();
}

AGraphWindow::~AGraphWindow()
{
    storeGeomStatus();
    //qDebug() << "Destructor called for AGraphWindow";
    delete ui;
}

void AGraphWindow::ShowAndFocus()
{
    RasterWindow->fCanvas->cd();
    this->show();
    this->activateWindow();
    this->raise();

    if (ColdStart)
    {
        //first time this window is shown
        ColdStart = false;
        this->resize(width()+1, height());
        this->resize(width()-1, height());
    }
}

void AGraphWindow::SetAsActiveRootWindow()
{
    if (ColdStart)
    {
        //first time this window is shown
        ColdStart = false;
        this->resize(width()+1, height());
        this->resize(width()-1, height());
    }

    RasterWindow->fCanvas->cd();
}

void AGraphWindow::ClearRootCanvas()
{
    RasterWindow->fCanvas->Clear();
}

void AGraphWindow::UpdateRootCanvas()
{
    RasterWindow->fCanvas->Update();
}

void AGraphWindow::SaveAs(const QString & filename)
{
    RasterWindow->SaveAs(filename);
}

void AGraphWindow::SetTitle(const QString & title)
{
    setWindowTitle(title);
}

/*
void AGraphWindow::resizeEvent(QResizeEvent * )
{
    double width = this->width();
    double height = this->height();

    RasterWindow->ForceResize();
}
*/

#include <QTimer>
bool AGraphWindow::event(QEvent * event)
{
    if (event->type() == QEvent::Close)
    {
        event->ignore();
        emit wasHidden(IdStr);
        return false;
    }

    /*
    if (event->type() == QEvent::WindowActivate)
    {
        RasterWindow->UpdateRootCanvas();
    }
    */

    if (event->type() == QEvent::Show)
    {
        if (ColdStart)
        {
            //first time this window is shown
            ColdStart = false;
            resize(width()+1, height());
            resize(width()-1, height());
        }
        else
        {
            qDebug() << "Graph win show event";
            //RasterWindow->UpdateRootCanvas();
            QTimer::singleShot(10, RasterWindow, [this](){RasterWindow->UpdateRootCanvas();}); // without delay canvas is not shown in Qt 5.9.5
        }
    }

    return QMainWindow::event(event);
}

#include <QSettings>
void AGraphWindow::storeGeomStatus()
{
    QSettings settings;
    settings.beginGroup(IdStr);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("visible", isVisible());
    settings.setValue("maximized", isMaximized());
    settings.endGroup();
}

void AGraphWindow::restoreGeomStatus()
{
    QSettings settings;
    settings.beginGroup(IdStr);
    restoreGeometry(settings.value("geometry").toByteArray());
    bool bVisible = settings.value("visible", false).toBool();
    bool bmax = settings.value("maximized", false).toBool();
    if (bVisible)
    {
        if (bmax) showMaximized();
        else      showNormal();
    }
    settings.endGroup();
}

void AGraphWindow::onMainWinButtonClicked(bool show)
{
    if (show)
    {
        restoreGeomStatus();
        showNormal();
        activateWindow();
    }
    else
    {
        storeGeomStatus();
        hide();
    }
}
