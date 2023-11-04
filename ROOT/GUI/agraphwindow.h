#ifndef AGRAPHWINDOW_H
#define AGRAPHWINDOW_H

#include <QMainWindow>

namespace Ui {
class AGraphWindow;
}

class ARasterWindow;

class AGraphWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AGraphWindow(const QString & idStr, QWidget * parent = nullptr);
    ~AGraphWindow();

    void ShowAndFocus();
    void SetAsActiveRootWindow();
    void ClearRootCanvas();
    void UpdateRootCanvas();

    void onMainWinButtonClicked(bool show);

    void SaveAs(const QString & filename);
    void SetTitle(const QString & title);

    void storeGeomStatus();
    void restoreGeomStatus();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    //void hideEvent(QHideEvent* event);  do not use! it is triggered AFTER the woindow is closed, we need to intercept BEFORE --> see event() method
    bool event(QEvent *event);

private:
    Ui::AGraphWindow * ui = nullptr;

    ARasterWindow * RasterWindow = nullptr;

    QString IdStr;
    bool ColdStart = true;

signals:
    void wasHidden(QString idStr);
};

#endif // AGRAPHWINDOW_H
