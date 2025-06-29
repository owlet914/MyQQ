#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QColor>
#include "server.h"
#include "client.h"
#include <QFontComboBox>
#include<QListWidget>
#include<QStandardItemModel>
#include<QProgressDialog>
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    void GetServer(Server* server);
    void GetClient(Client* client);
    void Showmsg();
    ~Widget();
    void Addmsg(QString msg);
    void init();
    void onFontChanged(const QFont &font);
    QTcpSocket* clt;
    QTcpSocket* cltF;
    void saveData();
    void showFilePreviewDialog();
    void processIncomingData();
    void updateFileListModel();
    void downloadSelectedFile(const QString& fileName);
private slots:
    void on_ColorpushButton_clicked();

    void on_SizecomboBox_currentIndexChanged(int index);

    void on_BoldpushButton_clicked(bool checked);

    void on_ItalicpushButton_clicked(bool checked);

    void on_UnderlinepushButton_clicked(bool checked);

    void on_SendTextpushButton_clicked();

    void on_SendpushButton_clicked();


    void on_ExitTextpushButton_clicked();

private:
    Ui::Widget *ui;
    QColor color;
    Client *client;
    QVector<QByteArray> receivedData;
    QMap<QString, QByteArray> fileDataMap;
    QStandardItemModel fileListModel;
};
#endif // WIDGET_H
