#include "amessage.h"

#include <QMessageBox>

void message(QString text, QWidget* parent)
{
  QMessageBox mb(parent);
  mb.setWindowFlags(mb.windowFlags() | Qt::WindowStaysOnTopHint);
  mb.setText(text);
  if (!parent) mb.move(200,200);
  mb.exec();
}

bool areYouSure(const QString &text, QWidget *parent)
{
    QMessageBox::StandardButton reply = QMessageBox::question(parent, "Confirm please", text,
                                    QMessageBox::Yes|QMessageBox::Cancel);
    if (reply == QMessageBox::Yes) return true;
    return false;
}
