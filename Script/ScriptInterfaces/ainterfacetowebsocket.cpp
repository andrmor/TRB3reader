#include "ainterfacetowebsocket.h"
#include "awebsocketsession.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

QJsonObject strToObject(const QString & s)
{
    QJsonDocument doc = QJsonDocument::fromJson(s.toUtf8());
    return doc.object();
}

AInterfaceToWebSocket::AInterfaceToWebSocket() : AScriptInterface() {}

AInterfaceToWebSocket::~AInterfaceToWebSocket()
{
    if (socket) socket->deleteLater();
}

void AInterfaceToWebSocket::abortRun()
{
    if (socket) socket->ExternalAbort();
}

void AInterfaceToWebSocket::SetTimeout(int milliseconds)
{
    TimeOut = milliseconds;

    if (socket) socket->SetTimeout(milliseconds);
}

QString AInterfaceToWebSocket::Connect(const QString &Url, bool GetAnswerOnConnection)
{
    if (!socket)
    {
        socket = new AWebSocketSession();
        socket->SetTimeout(TimeOut);
    }

    bool bOK = socket->Connect(Url, GetAnswerOnConnection);
    if (bOK)
    {
        return socket->GetTextReply();
    }
    else
    {
        abort(socket->GetError());
        return "";
    }
}

void AInterfaceToWebSocket::Disconnect()
{
    if (socket) socket->Disconnect();
}

QString AInterfaceToWebSocket::SendText(const QString &message)
{
    if (!socket)
    {
        abort("Web socket was not connected");
        return "";
    }

    bool bOK = socket->SendText(message);
    if (bOK)
        return socket->GetTextReply();
    else
    {
        abort(socket->GetError());
        return "";
    }
}

QString AInterfaceToWebSocket::SendObject(const QVariantMap & object)
{
    QJsonObject js = QJsonObject::fromVariantMap(object);

    return sendQJsonObject(js);
}

QString AInterfaceToWebSocket::sendQJsonObject(const QJsonObject& json)
{
    if (!socket)
    {
        abort("Web socket was not connected");
        return "";
    }

    bool bOK = socket->SendJson(json);
    if (bOK)
        return socket->GetTextReply();
    else
    {
        abort(socket->GetError());
        return "";
    }
}

QString AInterfaceToWebSocket::sendQByteArray(const QByteArray &ba)
{
    if (!socket)
    {
        abort("Web socket was not connected");
        return "";
    }

    bool bOK = socket->SendQByteArray(ba);
    if (bOK)
        return socket->GetTextReply();
    else
    {
        abort(socket->GetError());
        return "";
    }
}

QString AInterfaceToWebSocket::SendFile(const QString &fileName)
{
    if (!socket)
    {
        abort("Web socket was not connected");
        return "";
    }

    bool bOK = socket->SendFile(fileName);
    if (bOK)
        return socket->GetTextReply();
    else
    {
        abort(socket->GetError());
        return "";
    }
}

QString AInterfaceToWebSocket::ResumeWaitForAnswer()
{
    if (!socket)
    {
        abort("Web socket was not connected");
        return "";
    }

    bool bOK = socket->ResumeWaitForAnswer();
    if (bOK)
        return socket->GetTextReply();
    else
    {
        abort(socket->GetError());
        return "";
    }
}

QVariant AInterfaceToWebSocket::GetBinaryReplyAsObject()
{
    if (!socket)
    {
        abort("Web socket was not connected");
        return "";
    }
    const QByteArray& ba = socket->GetBinaryReply();
    QJsonDocument doc = QJsonDocument::fromJson(ba);
    QJsonObject json = doc.object();

    QVariantMap vm = json.toVariantMap();
    return vm;
}

bool AInterfaceToWebSocket::SaveBinaryReplyToFile(const QString &fileName)
{
    if (!socket)
    {
        abort("Web socket was not connected");
        return "";
    }
    const QByteArray& ba = socket->GetBinaryReply();
    qDebug() << "ByteArray to save size:"<<ba.size();

    QFile saveFile(fileName);
    if ( !saveFile.open(QIODevice::WriteOnly) )
    {
        abort( QString("Server: Cannot save binary to file: ") + fileName );
        return false;
    }
    saveFile.write(ba);
    saveFile.close();
    return true;
}
