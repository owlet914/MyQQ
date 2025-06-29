#include "client.h"

Client::Client(QTcpSocket* tClt,QObject *parent)
    : QObject{parent},tClt(tClt)
{

}
void Client::Addmsg(QVector<QByteArray> msgs)
{
    for(auto msg : msgs)
        this->msgs.append(msg);
}
QVector<QByteArray>* Client::Getmsgs()
{
    return &msgs;
}
Client::~Client()
{
    delete tClt;
}
