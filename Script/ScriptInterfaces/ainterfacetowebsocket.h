#ifndef AINTERFACETOWEBSOCKET_H
#define AINTERFACETOWEBSOCKET_H

#include "ascriptinterface.h"

#include <QObject>
#include <QVariant>

class QWebSocketServer;
class QWebSocket;
class AWebSocketSession;

class AInterfaceToWebSocket: public AScriptInterface
{
  Q_OBJECT

public:
    AInterfaceToWebSocket();
    ~AInterfaceToWebSocket();

    AScriptInterface * cloneBase() const override {return new AInterfaceToWebSocket();}

    void abortRun() override;

public slots:    
    QString  Connect(const QString & Url, bool GetAnswerOnConnection);
    void     Disconnect();

    QString  SendText(const QString & message);
    QString  SendObject(const QVariant & object);
    QString  SendFile(const QString & fileName);

    QString  ResumeWaitForAnswer();

    QVariant GetBinaryReplyAsObject();
    bool     SaveBinaryReplyToFile(const QString & fileName);

    void     SetTimeout(int milliseconds);

signals:
    void showTextOnMessageWindow(const QString & text);
    void clearTextOnMessageWindow();

private:
    AWebSocketSession * socket = nullptr;

    int TimeOut = 3000; //milliseconds

private:
    QString sendQJsonObject(const QJsonObject & json);
    QString sendQByteArray(const QByteArray & ba);
};

#endif // AINTERFACETOWEBSOCKET_H
