#include "server.h"
#include "msg.h"
Server::Server(QWidget *parent,qint16 port) :QDialog(parent)
{
    setFixedSize(400,200);
    tport=port;
    tSrv=new QTcpServer(this);

    QAbstractSocket::connect(tSrv, &QTcpServer::newConnection, this, &Server::newconnect);
    initServer();
}

void Server::sendmsg(QByteArray data)
{
    // 遍历所有已连接的客户端并发送消息
    for (auto client : Allclt) {
        msg::sent(data, client);
    }
}
void Server::newconnect()
{
    if (num % 2 == 0) {
        // 处理聊天客户端连接（第 1、3、5... 个连接）
        QTcpSocket *currentClient = tSrv->nextPendingConnection();
        if (!currentClient) return;

        Allclt.append(currentClient);
        clientconnect = currentClient;

        // 连接 readyRead 信号，处理接收数据
        connect(currentClient, &QTcpSocket::readyRead, [this, currentClient]() {
            qDebug() << "Bytes available:" << currentClient->bytesAvailable();
            QVector<QByteArray> data = msg::recieve(currentClient);
            if (!data.isEmpty()) {
                for (auto receivedData : data) {
                    sendmsg(receivedData);
                }
            }
        });

        num++;
        // 记录客户端IP
        QString clientIp = currentClient->peerAddress().toString();
        qDebug() << "New client connected:" << clientIp;
        NameList.push_back(clientIp);

        // 发送欢迎消息
        QString welcomeMsg = "Hello, client!";
        msg::sent(welcomeMsg.toUtf8(), currentClient);

        // 处理客户端断开连接
        connect(currentClient, &QTcpSocket::disconnected, [this, clientIp,currentClient]() {
            // 从列表中移除客户端
            auto itClt = std::remove(Allclt.begin(), Allclt.end(), QPointer<QTcpSocket>(currentClient));
            if (itClt != Allclt.end()) {
                Allclt.erase(itClt);
            }
            // 从 NameList 中移除对应的 IP
            auto itName = std::remove(NameList.begin(), NameList.end(), clientIp);
            if (itName != NameList.end()) {
                NameList.erase(itName);
            }
            num--;
            qDebug() << "Client disconnected, num updated:" << num;
        });
    } else {
        // 处理文件传输客户端连接（第 2、4、6... 个连接）
        QTcpSocket *currentClientFile = tSrv->nextPendingConnection();
        if (!currentClientFile) return;

        AllcltFile.append(currentClientFile);

        // 连接 readyRead 信号，处理文件数据接收与转发
        connect(currentClientFile, &QTcpSocket::readyRead, [this, currentClientFile]() {
            qDebug() << "Bytes available (file):" << currentClientFile->bytesAvailable();
            QVector<QByteArray> data = msg::recieve(currentClientFile);
            if (!data.isEmpty()) {
                for (auto receivedData : data) {
                    for (auto client : AllcltFile) {
                        if (client && client->state() == QTcpSocket::ConnectedState) {
                            msg::sent(receivedData, client);
                        }
                    }
                }
            }
        });

        // 处理文件客户端断开连接
        connect(currentClientFile, &QTcpSocket::disconnected, [this, currentClientFile]() {
            // 从文件连接列表中移除
            auto itFile = std::remove(AllcltFile.begin(), AllcltFile.end(), QPointer<QTcpSocket>(currentClientFile));
            if (itFile != AllcltFile.end()) {
                AllcltFile.erase(itFile);
            }
            num--;
            qDebug() << "File client disconnected, num updated:" << num;

            // 更新在线人数并广播给文件客户端
            QString msg = "当前在线人数：" + QString::number(num / 2) + "用户列表|";
            for (auto ip : NameList) {
                msg += ip + ',';
            }
            QByteArray data = msg.toUtf8();
            for (auto client : AllcltFile) {
                if (client && client->state() == QTcpSocket::ConnectedState) {
                    msg::sent(data, client);
                }
            }
        });

        num++;
        // 首次连接文件客户端时，广播在线人数
        QString onlineMsg = "当前在线人数：" + QString::number(num / 2) + "用户列表|";
        for (auto ip : NameList) {
            onlineMsg += ip + ',';
        }
        QByteArray data = onlineMsg.toUtf8();
        for (auto client : AllcltFile) {
            if (client && client->state() == QTcpSocket::ConnectedState) {
                msg::sent(data, client);
            }
        }
    }
}


void Server::sendFile(QString filename)
{

}

QStringList getLocalIPs() {
    QStringList ips;
    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces()) {
        if (interface.flags() & (QNetworkInterface::IsUp | QNetworkInterface::IsRunning) &&
            !(interface.flags() & QNetworkInterface::IsLoopBack)) {

            foreach (const QNetworkAddressEntry &entry, interface.addressEntries()) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    ips.append(entry.ip().toString());
                }
            }
        }
    }
    return ips;
}

void Server::initServer()
{
    if(!tSrv->listen(QHostAddress::Any, tport)) {
        QMessageBox::critical(this, "Server Error", "无法启动服务器:" + tSrv->errorString());
        close();
        return;
    }


    qDebug() << "可用IP地址:" << getLocalIPs();
    qDebug() << "Server started on port" << tport;
    IP=getLocalIPs()[0];
}
void Server::refused()
{
    // 拒绝新连接
    if (tSrv->isListening()) {
        tSrv->close();
        qDebug() << "Server stopped";
    }
}



