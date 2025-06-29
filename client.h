#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include "msg.h"
class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QTcpSocket* tClt,QObject *parent = nullptr);
    ~Client();
    void Addmsg(QVector<QByteArray> msg);
    QVector<QByteArray>* Getmsgs();
    QTcpSocket *tClt;
    QTcpSocket *tCltFile;
private:

    QVector<QByteArray> msgs;
};

#endif // CLIENT_H
