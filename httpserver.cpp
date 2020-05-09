#include "httpserver.h"
#include <QTextStream>

HttpServer::HttpServer(QObject * parent)
    : QObject(parent)
{
    QObject::connect(&tcp_server, &QTcpServer::acceptError, this, &HttpServer::onError);
    QObject::connect(&tcp_server, &QTcpServer::newConnection, this, &HttpServer::onNewConnection);
}

bool HttpServer::start(int port)
{
    return tcp_server.listen(QHostAddress::Any, port);
}

void HttpServer::onError(QAbstractSocket::SocketError socketError)
{
   qDebug() << "tcp server error: " << socketError;
}

void HttpServer::onNewConnection()
{
   QObject * sender = this->sender();  // to get sender object pointer
   QTcpServer * server = static_cast<QTcpServer *>(sender);  // downcast
   //
   qDebug() << "got client new pending connection";
   QTcpSocket * new_client = server->nextPendingConnection();
   // connect client socket signals
   connect(new_client, &QTcpSocket::readyRead, this, &HttpServer::onClientReadyRead);
   connect(new_client, &QTcpSocket::bytesWritten, this, &HttpServer::onClientDataSent);
   connect(new_client, &QTcpSocket::disconnected, this, &HttpServer::onClientDisconnected);
}

HttpResponse HttpServer::handleRequest(HttpRequest & req)
{
    HttpResponse res;
    res.http_version = "HTTP/1.1";
    HttpHandler handler = nullptr;
    if (handlers_.contains(req.method + " " + req.uri))
        handler = handlers_[req.method + " " + req.uri];
    else if (handler_ != nullptr)
        handler = handler_;
    if (handler == nullptr)
    {
        res.status_code = 404;
        res.status_description = "Not Found";
    }
    else
    {
        res.status_code = 200;
        res.status_description = "OK";
        res.headers["Content-type"] = "text/html";
        handler(req, res);
    }
    int content_length = res.body.length();
    if (content_length > 0)
        res.headers["Content-length"] = QString::number(content_length);
    return res;
}

HttpRequest parseHttpRequest(const QString & str);

QString formatHttpResponse(const HttpResponse & res);

void HttpServer::onClientReadyRead()
{
   QObject * sender = this->sender();  // to get sender object pointer
   QTcpSocket * client = static_cast<QTcpSocket *>(sender);  // downcast
   //
   QString received_string = QString::fromUtf8(client->readAll());
   HttpRequest req = parseHttpRequest(received_string);
   HttpResponse res = handleRequest(req);
   client->write(formatHttpResponse(res).toUtf8());
   client->flush();
}

void HttpServer::onClientDataSent()
{
    qDebug() << "data sent to client.";
    QObject * sender = this->sender();  // to get sender object pointer
    QTcpSocket * client = static_cast<QTcpSocket *>(sender);  // downcast
    // close the connection
    client->close();
}

void HttpServer::onClientDisconnected()
{
   QObject * sender = this->sender();  // to get sender object pointer
   QTcpSocket * client = static_cast<QTcpSocket *>(sender);  // downcast
   //
   qDebug() << "client disconnected";
   client->deleteLater();  // use this instead of delete
}

HttpRequest parseHttpRequest(const QString & str)
{
    HttpRequest res;
    QString copy = str;
    QTextStream ts{&copy};
    // GET /uri HTTP/1.1
    QString request_line = ts.readLine();
    QStringList request_line_parts = request_line.split(" ");
    res.method = request_line_parts[0];
    res.uri = request_line_parts[1];
    res.http_version = request_line_parts[2];
    // Content-type: text/html
    QString header_line;
    while ((header_line = ts.readLine()).length() > 0)
    {
        QStringList parts = header_line.split(": ");
        res.headers[parts[0]] = parts[1];
    }
    //
    res.body = ts.readAll();
    return res;
}

QString formatHttpResponse(const HttpResponse & res)
{
    QString str;
    QTextStream ts{&str};
    // HTTP/1.1 200 OK
    ts << res.http_version << " " << res.status_code << " " << res.status_description << "\r\n";
    // Content-type: text/html
    QMap<QString, QString>::const_iterator it;
    for (it = res.headers.cbegin(); it != res.headers.cend(); ++it)
        ts << it.key() + ": " + it.value() << "\r\n";
    ts << "\r\n";
    //
    ts << res.body;
    return str;
}
