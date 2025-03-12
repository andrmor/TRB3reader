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
    connect(RasterWindow, &ARasterWindow::cursorPositionChanged, this, &AGraphWindow::onCursorPositionChanged);

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
    Title = title;
    setWindowTitle(title);
}

#include <QTimer>
void AGraphWindow::resizeEvent(QResizeEvent *)
{
    //storeGeomStatus();
    //QTimer::singleShot(100, this, &AGraphWindow::storeGeomStatus);
}

void AGraphWindow::moveEvent(QMoveEvent *)
{
    //storeGeomStatus();
    //QTimer::singleShot(100, this, &AGraphWindow::storeGeomStatus);
}

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

void AGraphWindow::onDrawRequest(TObject *obj, QString options, bool )
{
    //qDebug() << "Here!!!!" << IdStr;
    showNormal();
    activateWindow();
    SetAsActiveRootWindow();
    obj->Draw(options.toLatin1().data());
    UpdateRootCanvas();
}

void AGraphWindow::onCursorPositionChanged(double x, double y)
{
    QString s = QString("%0  (%1, %2)").arg(Title).arg(x).arg(y);
    setWindowTitle(s);
    //qDebug() << x << y;
}

#include <QSettings>
void AGraphWindow::storeGeomStatus()
{
    //qDebug() << "store" << IdStr;
    QSettings settings;
    settings.beginGroup(IdStr);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("visible", isVisible());
    settings.setValue("maximized", isMaximized());
    settings.endGroup();
}

void AGraphWindow::restoreGeomStatus()
{
    //qDebug() << "restore" << IdStr;
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
        showNormal();
        restoreGeomStatus();
        activateWindow();
    }
    else
    {
        storeGeomStatus();
        hide();
    }
}
