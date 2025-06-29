#include "msg.h"

msg::msg() {}

void msg::sent(QByteArray data,QTcpSocket* socket)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << quint32(data.size());
    out << data;
    socket->write(block);
    socket->waitForBytesWritten(5000);
}
QVector<QByteArray> msg::recieve(QTcpSocket* socket)
{
    QVector<QByteArray> tmp;
    QByteArray buffer;
    quint32 dataSize = 0;
    bool hasSize = false;
    buffer=socket->readAll();
    while (socket->state() == QAbstractSocket::ConnectedState) {

        // 先读取长度头部
        if (!hasSize && buffer.size() >= sizeof(quint32)) {
            QDataStream in(&buffer, QIODevice::ReadOnly);
            in.setVersion(QDataStream::Qt_5_15);
            in >> dataSize;
            hasSize = true;
            // 移除已读取的长度头部
            buffer = buffer.mid(sizeof(quint32));
        }

        // 再读取完整数据
        if (hasSize && buffer.size() >= dataSize) {
            QByteArray completeData = buffer.right(dataSize);
            buffer = buffer.mid(dataSize); // 移除已处理的数据
            tmp.append(completeData); // 处理完整数据
            hasSize = false; // 重置状态，准备接收下一条消息
            dataSize = 0;
        }

        // 当没有完整数据且没有更多数据可读取时退出循环
        if (!hasSize && dataSize == 0 && socket->bytesAvailable() == 0) {
            break;
        }

        // 等待更多数据
        if (socket->bytesAvailable() == 0) {
            if (!socket->waitForReadyRead(5000)) {
                qDebug() << "Timeout or error";
                break;
            }
            buffer.append(socket->readAll());
        }

    }
    return tmp;
}
