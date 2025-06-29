#ifndef MSG_H
#define MSG_H
#include<QTcpSocket>

class msg
{
public:
    msg();
    static void sent(QByteArray data,QTcpSocket* socket);
    static QVector<QByteArray> recieve(QTcpSocket* socket);
    static qfloat16 nowpercents;
};

#endif // MSG_H
