#ifndef AINTERFACETOWEBSOCKET_H
#define AINTERFACETOWEBSOCKET_H

#include "ascriptinterface.h"

#include <QObject>
#include <QVariant>

class QWebSocketServer;
class QWebSocket;

class AInterfaceToWebSocket: public AScriptInterface
{
  Q_OBJECT

public:
    enum ServerState {Idle=0, Sending, WaitingForAnswer};
    AInterfaceToWebSocket();
    ~AInterfaceToWebSocket();

public slots:
    void setTimeout(int milliseconds) {timeout = milliseconds;}
    QString SendTextMessage(QString Url, QVariant message, bool WaitForAnswer=false);
    int Ping(QString Url);

    bool StartListen(quint16 port);
    void StopListen();

private slots:
    void onClientConnected();
    void onTextMessageReceived(QString message);

    void onNewConnection();
    void processTextMessage(QString message);
    //void processBinaryMessage(QByteArray message);
    void socketDisconnected();

signals:
    void RequestEvaluate(const QString& script, QVariant& result);

private:
    QWebSocketServer *Server;
    QWebSocket *ClientSocket;

    QList<QWebSocket *> ServerListOfClients;
    QWebSocket* ServerReplyClient;

    int timeout;
    int lastExchangeDuration;

    ServerState State;
    QString MessageToSend;
    QString MessageReceived;
    bool fWaitForAnswer;

    QString variantToString(QVariant val);
    void ReplyAndClose(QString message);
};

#endif // AINTERFACETOWEBSOCKET_H
