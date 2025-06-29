#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include "server.h"
#include "client.h"
namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = nullptr);
    ~Form();

private slots:
    void on_NewpushButton_clicked();

    void on_ConnectpushButton_clicked();

private:
    Ui::Form *ui;
    Server* server;
    QTcpSocket* socket;
    QTcpSocket* socketFile;
    Client* client;
};

#endif // FORM_H
