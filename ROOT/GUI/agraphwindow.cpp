#include "agraphwindow.h"
#include "ui_agraphwindow.h"
#include "arasterwindow.h"

#include <QJsonObject>
#include <QDebug>

#include "TCanvas.h"

AGraphWindow::AGraphWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AGraphWindow)
{
    RasterWindow = 0;
    QWinContainer = 0;
    ColdStart = true;

    ui->setupUi(this);

    RasterWindow = new ARasterWindow(this);

    QWinContainer = QWidget::createWindowContainer(RasterWindow, this);
    QWinContainer->setVisible(true);

    QWinContainer->setGeometry(0, 0, this->width(), this->height());
    RasterWindow->resize(this->width(), this->height());
    RasterWindow->ForceResize();

}

AGraphWindow::~AGraphWindow()
{
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

void AGraphWindow::SaveAs(const QString filename)
{
    RasterWindow->SaveAs(filename);
}

void AGraphWindow::SetTitle(QString title)
{
    setWindowTitle(title);
}

void AGraphWindow::resizeEvent(QResizeEvent * /*event*/)
{
    double width = this->width();
    double height = this->height();

    if (QWinContainer) QWinContainer->setGeometry(0, 0, width, height);
    if (RasterWindow) RasterWindow->ForceResize();
}

void AGraphWindow::hideEvent(QHideEvent *)
{
    emit WasHidden();
}
