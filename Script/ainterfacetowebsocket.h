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
    AInterfaceToWebSocket(const AInterfaceToWebSocket& other);
    ~AInterfaceToWebSocket();

    virtual bool IsMultithreadCapable() const {return true;}
    virtual void ForceStop();

public slots:    
    const QString  Connect(const QString& Url, bool GetAnswerOnConnection);
    void           Disconnect();

    const QString  SendText(const QString& message);
    const QString  SendObject(const QVariant& object);
    const QString  SendFile(const QString& fileName);

    const QString  ResumeWaitForAnswer();

    const QVariant GetBinaryReplyAsObject();
    bool           SaveBinaryReplyToFile(const QString& fileName);

    void           SetTimeout(int milliseconds);

signals:
    void showTextOnMessageWindow(const QString& text);
    void clearTextOnMessageWindow();

private:
    AWebSocketSession* socket = 0;

    int TimeOut = 3000; //milliseconds

private:
    const QString sendQJsonObject(const QJsonObject &json);
    const QString sendQByteArray(const QByteArray &ba);
};

#endif // AINTERFACETOWEBSOCKET_H
