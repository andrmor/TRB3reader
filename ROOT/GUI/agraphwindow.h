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
    explicit AGraphWindow(QWidget *parent = 0);
    ~AGraphWindow();

    void ShowAndFocus();
    void SetAsActiveRootWindow();
    void ClearRootCanvas();
    void UpdateRootCanvas();

    void SaveAs(const QString filename);

    void SetTitle(QString title);

protected:
    void resizeEvent(QResizeEvent *event);
    void hideEvent(QHideEvent* event);

private:
    Ui::AGraphWindow *ui;
    ARasterWindow *RasterWindow;
    QWidget *QWinContainer;

    bool ColdStart;

signals:
    void WasHidden();
};

#endif // AGRAPHWINDOW_H
