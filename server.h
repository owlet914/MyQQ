#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include <QObject>
#include <QTime>
#include<QTcpServer>
#include<QTcpSocket>
#include<QFile>
#include<QFileDialog>
#include<QDebug>
#include<QMessageBox>
#include <QNetworkInterface>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMutex>
#include<QPointer>
// class QFile;
// class QTcpServer;
// class QTcpSocket;

#include"client.h"

class Server : public QDialog
{
public:
    Server(QWidget *parent=0,qint16 port=6666);
    qint16 tport=6666;
    QTcpServer *tSrv;
    QString filename;
    QFile *localFile;

    qint64 totalbytes;

    QByteArray outblock;

    qint16 num=0;
    QTcpSocket *clientconnect;
    QTcpSocket *clientconnectFile;
    QTime time;
    void initServer();
    void refused();
    QString IP;
public slots:
    void sendmsg(QByteArray data);
    void sendFile(QString filename);
    void newconnect();
private:
    QVector<QTcpSocket*> Allclt;
    QVector<QTcpSocket*> AllcltFile;
    QVector<QString> NameList;
};

#endif // SERVER_H
