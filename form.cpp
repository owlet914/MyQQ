#include "form.h"
#include "ui_form.h"
#include "server.h"
#include "widget.h"
Form::Form(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Form)
{
    ui->setupUi(this);

}

Form::~Form()
{
    delete ui;
}

void Form::on_NewpushButton_clicked()
{
    qint16 port=6666;
    if(ui->PortlineEdit->text().toInt()!=0)
        port=ui->PortlineEdit->text().toInt();
    server=new Server(0,port);
    this->close();
    static Widget w;

    w.Addmsg("服务器已在"+server->IP+":"+QString::number(port)+"上启动");
    socket=new QTcpSocket;
    socketFile=new QTcpSocket;
    // 连接到服务器
    socket->connectToHost(server->IP,port);
    socketFile->connectToHost(server->IP,port);
    qDebug()<<socket;
    w.show();
    w.clt=socket;
    w.cltF=socketFile;
    qDebug()<<w.clt;
    w.init();
}


void Form::on_ConnectpushButton_clicked()
{


    ui->ConnectpushButton->setEnabled(false);
    socket=new QTcpSocket;
    socketFile=new QTcpSocket;
    // 连接到服务器
    QString IP=ui->IPlineEdit->text();
    qint16 port=ui->PortlineEdit->text().toInt();
    socket->connectToHost(IP, port);
    socketFile->connectToHost(IP, port);
    // 等待连接建立或超时
    if(socket->waitForConnected(3000)&&socketFile->waitForConnected(3000)) {
        qDebug() << "Connected to server";

        this->hide();
        static Widget w;
        w.clt=socket;
        w.cltF=socketFile;
        w.show();
        w.init();

    } else {
        qDebug() << "Connection failed:" << socket->errorString();
        QMessageBox::critical(this, "连接失败", "无法连接到服务器:\n" + socket->errorString());
        ui->ConnectpushButton->setEnabled(true);
    }
}

