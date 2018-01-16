#include "ainterfacetowebsocket.h"

#include <QWebSocketServer>
#include <QWebSocket>

#include <QApplication>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QElapsedTimer>

AInterfaceToWebSocket::AInterfaceToWebSocket()
{
    timeout = 3000;
    ClientSocket = new QWebSocket();
    connect(ClientSocket, &QWebSocket::connected, this, &AInterfaceToWebSocket::onClientConnected);
    connect(ClientSocket, &QWebSocket::textMessageReceived, this, &AInterfaceToWebSocket::onTextMessageReceived);
    State = Idle;

    Server = new QWebSocketServer(QStringLiteral("Echo Server"), QWebSocketServer::NonSecureMode, this);

    //qDebug() << QMetaObject::normalizedSignature(SIGNAL(RequestEvaluate(const QString&, QVariant&)));
}

AInterfaceToWebSocket::~AInterfaceToWebSocket()
{
    ClientSocket->deleteLater();

    Server->close();
    qDeleteAll(ServerListOfClients.begin(), ServerListOfClients.end());
}

QString AInterfaceToWebSocket::SendTextMessage(QString Url, QVariant message, bool WaitForAnswer)
{
   if (State != Idle)
   {
       qDebug() << "Not ready for this yet....";
       exit(678);
   }

   qDebug() << "-->Attempting to send to Url:\n" << Url
            << "\n->Message:\n" << message
            << "\n<-";

   State = Sending;
   MessageToSend = variantToString(message);
   MessageReceived = "";
   fWaitForAnswer = WaitForAnswer;

   QElapsedTimer timer;
   timer.start();

   ClientSocket->open(QUrl(Url));   
   do
   {
       qApp->processEvents();
       if (timer.elapsed() > timeout)
       {
           ClientSocket->abort();
           State = Idle;
           abort("Timeout!");
           return "";
       }
   }
   while (State != Idle);

   ClientSocket->close();
   lastExchangeDuration = timer.elapsed();
   return MessageReceived;
}

int AInterfaceToWebSocket::Ping(QString Url)
{
  SendTextMessage(Url, "", true);
  return lastExchangeDuration;
}

void AInterfaceToWebSocket::onClientConnected()
{
    qDebug() << "ClientSocket connected";
    ClientSocket->sendTextMessage(MessageToSend);

    if (!fWaitForAnswer) State = Idle;
    else State = WaitingForAnswer;
}

void AInterfaceToWebSocket::onTextMessageReceived(QString message)
{
    qDebug() << "Message received:" << message;
    MessageReceived = message;
    State = Idle;
}

QString AInterfaceToWebSocket::variantToString(QVariant val)
{
    QString type = val.typeName();

    QJsonValue jv;
    QString rep;
    if (type == "int")
      {
        jv = QJsonValue(val.toInt());
        rep = QString::number(val.toInt());
      }
    else if (type == "double")
      {
        jv = QJsonValue(val.toDouble());
        rep = QString::number(val.toDouble());
      }
    else if (type == "bool")
      {
        jv = QJsonValue(val.toBool());
        rep = val.toBool() ? "true" : "false";
      }
    else if (type == "QString")
      {
        jv = QJsonValue(val.toString());
        rep = val.toString();
      }
//    else if (type == "QVariantList")
//      {
//        QVariantList vl = val.toList();
//        QJsonArray ar = QJsonArray::fromVariantList(vl);
//        jv = ar;
//        rep = "-Array-";
//      }
    else if (type == "QVariantMap")
        {
          QVariantMap mp = val.toMap();
          QJsonObject json = QJsonObject::fromVariantMap(mp);
          QJsonDocument doc(json);
          rep = QString(doc.toJson(QJsonDocument::Compact));
        }

    return rep;
}

//---------------------

bool AInterfaceToWebSocket::StartListen(quint16 port)
{
    if (Server->listen(QHostAddress::Any, port))
    {
            qDebug() << "ANTS2 web socket server is now listening";
            qDebug() << "--Address:"<< Server->serverAddress();
            qDebug() << "--Port:"<< Server->serverPort();
            qDebug() << "--URL:" << Server->serverUrl().toString();

            connect(Server, &QWebSocketServer::newConnection, this, &AInterfaceToWebSocket::onNewConnection);

            return true;
    }
    else    return false;
}

void AInterfaceToWebSocket::StopListen()
{
    Server->close();
}

void AInterfaceToWebSocket::onNewConnection()
{
    qDebug() << "New connection";
    QWebSocket *pSocket = Server->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &AInterfaceToWebSocket::processTextMessage);
    //connect(pSocket, &QWebSocket::binaryMessageReceived, this, &AInterfaceToWebSocket::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &AInterfaceToWebSocket::socketDisconnected);

    ServerListOfClients << pSocket;
}

void AInterfaceToWebSocket::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "Server - message received:" << message << "client:"<<pClient;
    ServerReplyClient = pClient;

    QVariant res;
    emit RequestEvaluate(message, res);

    ReplyAndClose(res.toString());
}

void AInterfaceToWebSocket::ReplyAndClose(QString message)
{
    qDebug() << "Reply and close request";
    qDebug() << "Message:"<<message<<"client:"<<ServerReplyClient;

    if (ServerReplyClient)
    {
        ServerReplyClient->sendTextMessage(message);
        ServerReplyClient->close();
        ServerReplyClient = 0;
    }
}


/*
void AInterfaceToWebSocket::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "Binary Message received:" << message;
    //if (pClient) pClient->sendBinaryMessage(message);
}
*/

void AInterfaceToWebSocket::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "socketDisconnected:" << pClient;

    if (pClient)
    {
        ServerListOfClients.removeAll(pClient);
        pClient->deleteLater();
    }

    ServerReplyClient = 0;
}
