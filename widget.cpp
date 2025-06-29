#include "widget.h"
#include "ui_widget.h"
#include <QColorDialog>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    
    // 初始化字号下拉框
    ui->SizecomboBox->addItems({"12", "14", "16", "18", "20", "22", "24"});
    ui->SizecomboBox->setCurrentIndex(2); // 默认选择16号
    // 设置字体选择器的当前字体为文本编辑框的当前字体
    ui->FontcomboBox->setCurrentFont(ui->MessagetextEdit->font());
    connect(ui->FontcomboBox, &QFontComboBox::currentFontChanged, this, &Widget::onFontChanged);
}

Widget::~Widget()
{

}

void Widget::on_ColorpushButton_clicked()
{
    color=QColorDialog::getColor(color,this);
    if(color.isValid())
    {
        ui->MessagetextEdit->setTextColor(color);
        ui->MessagetextEdit->setFocus();
    }
}


void Widget::on_SizecomboBox_currentIndexChanged(int index)
{
    int size = ui->SizecomboBox->currentText().toInt();
    QTextCharFormat format;
    format.setFontPointSize(size);
    ui->MessagetextEdit->mergeCurrentCharFormat(format);
    ui->MessagetextEdit->setFocus();
}

void Widget::on_BoldpushButton_clicked(bool checked)
{
    if(checked)
    {
        ui->MessagetextEdit->setFontWeight(QFont::Bold);

    }
    else
    {
        ui->MessagetextEdit->setFontWeight(QFont::Normal);
    }
    ui->MessagetextEdit->setFocus();
}


void Widget::on_ItalicpushButton_clicked(bool checked)
{
    ui->MessagetextEdit->setFontItalic(checked);
    ui->MessagetextEdit->setFocus();
}


void Widget::on_UnderlinepushButton_clicked(bool checked)
{
    ui->MessagetextEdit->setFontUnderline(checked);
    ui->MessagetextEdit->setFocus();
}

void Widget::GetClient(Client* client)
{
    this->client=client;
}
void Widget::Addmsg(QString msg)
{
    ui->textBrowser->append(msg);
}
void Widget::Showmsg()
{
    if(client!=NULL&&client->Getmsgs()->length()>=1)
        for(auto i :  *client->Getmsgs())
            ui->textBrowser->append(i);
    client->Getmsgs()->clear();
}


void Widget::on_SendTextpushButton_clicked()
{
    if(client==NULL)
        init();
    QHostAddress ipAddress = clt->peerAddress();
    quint16 port = clt->peerPort();
    QString ipPort = ipAddress.toString() + ":" + QString::number(port);

    // 获取当前时间并格式化
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");

    // 构建带IP和时间的消息前缀
    QString prefix = "<b>[" + ipPort + " " + timeStr + "]</b>";
    QByteArray c=prefix.toUtf8(),d=ui->MessagetextEdit->toHtml().toUtf8();
    msg::sent(c+d,clt);
    ui->MessagetextEdit->clear();
}
void Widget::init()
{
    client=new Client(clt);
    client->tCltFile=cltF;
    ui->Usernumlabel->setText("当前在线人数："+QString::number(0));
    qDebug()<<clt;
    // 连接 readyRead 信号，当有数据可读时调用处理函数
    connect(clt, &QTcpSocket::readyRead,  [this]() {
        qDebug() << "Bytes available:" << clt->bytesAvailable();
        QVector<QByteArray> data = msg::recieve(clt);
        if (!data.isEmpty()) {
            qDebug() << "Received data count:" << data.size();
            client->Addmsg(data);
            this->Showmsg();
        } else {
            qDebug() << "No valid data received.";
        }
    });

    // 连接保存按钮的点击信号
    connect(ui->SavepushButton, &QPushButton::clicked, this, &Widget::saveData);
    this->Showmsg();
    connect(cltF, &QTcpSocket::readyRead, this, [this]() {
        qDebug() << "Bytes available:" << cltF->bytesAvailable();
        processIncomingData();
    });

    // 连接保存按钮的点击信号
    connect(ui->SavepushButton, &QPushButton::clicked, this, &Widget::showFilePreviewDialog);

    // 初始化文件列表视图
    ui->fileListView->setModel(&fileListModel);
    ui->fileListView->setSelectionMode(QAbstractItemView::SingleSelection);

}

void Widget::on_SendpushButton_clicked()
{
    // 打开文件选择对话框（单选模式）
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择要发送的文件",
        QDir::homePath(),
        "所有文件 (*)"
        );

    if (filePath.isEmpty()) {
        return; // 用户取消了选择
    }

    // 检查套接字是否已连接
    if (cltF->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::critical(this, "发送文件", "未连接到服务器");
        return;
    }

    QProgressDialog progress("正在发送文件...", "取消", 0, 1, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    // 打开并读取文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开文件:" << filePath << file.errorString();
        progress.cancel();
        QMessageBox::critical(this, "发送文件", "无法打开文件: " + file.errorString());
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    QString fileName = QFileInfo(filePath).fileName();

    // 构建要发送的数据格式：文件名|**|文件内容
    QByteArray dataToSend;
    dataToSend.append(fileName.toUtf8());
    dataToSend.append("|**|");
    dataToSend.append(fileData);

    // 发送数据
   msg::sent(dataToSend, cltF);

    cltF->waitForBytesWritten();
    progress.setValue(1);
    QMessageBox::information(this, "发送文件", "文件已成功发送");

}

void Widget::saveData() {
//     if (receivedData.isEmpty()) {
//         QMessageBox::information(this, "保存数据", "没有可保存的数据");
//         return;
//     }

//     // 打开文件选择对话框
//     QString filePath = QFileDialog::getSaveFileName(
//         this,
//         "保存文件",
//         QDir::homePath(),
//         "所有文件 (*);;文本文件 (*.txt);;数据文件 (*.dat)"
//         );

//     if (filePath.isEmpty()) {
//         return; // 用户取消了保存操作
//     }

//     // 尝试打开文件进行写入
//     QFile file(filePath);
//     if (!file.open(QIODevice::WriteOnly)) {
//         QMessageBox::critical(this, "保存文件", "无法打开文件: " + file.errorString());
//         return;
//     }

//     // 将接收到的数据写入文件
//     QDataStream out(&file);
//     out.setVersion(QDataStream::Qt_5_15);

//     // 写入所有数据块
//     for (const QByteArray& data : receivedData) {
//         out << data;
//     }

//     file.close();
//     QMessageBox::information(this, "保存文件", "数据已成功保存到: " + filePath);
}

void Widget::showFilePreviewDialog() {
    if (fileDataMap.isEmpty()) {
        QMessageBox::information(this, "文件预览", "没有可预览的文件");
        return;
    }

    // 创建文件预览对话框
    QDialog previewDialog(this);
    previewDialog.setWindowTitle("文件预览");
    previewDialog.resize(600, 400);

    QVBoxLayout* layout = new QVBoxLayout(&previewDialog);

    // 添加文件列表
    QListWidget* fileListWidget = new QListWidget(&previewDialog);
    for (const QString& fileName : fileDataMap.keys()) {
        fileListWidget->addItem(fileName);
    }
    layout->addWidget(new QLabel("可用文件:", &previewDialog));
    layout->addWidget(fileListWidget);

    // 添加预览区域（条件显示）
    QTextEdit* previewTextEdit = new QTextEdit(&previewDialog);
    previewTextEdit->setReadOnly(true);
    QLabel* previewLabel = new QLabel("文件预览:");
    layout->addWidget(previewLabel);
    layout->addWidget(previewTextEdit);

    // 添加文件大小提示标签
    QLabel* fileSizeLabel = new QLabel();
    layout->addWidget(fileSizeLabel);

    // 添加下载按钮
    QPushButton* downloadButton = new QPushButton("下载选中文件", &previewDialog);
    layout->addWidget(downloadButton);

    // 连接文件选择信号（带文件大小检测）
    connect(fileListWidget, &QListWidget::currentItemChanged, [this, previewTextEdit, fileSizeLabel,previewLabel](QListWidgetItem* item) {
        if (!item) return;
        QString fileName = item->text();
        if (!fileDataMap.contains(fileName)) return;

        // 获取文件内容和大小
        QByteArray fileContent = fileDataMap[fileName];
        qint64 fileSize = fileContent.size();

        // 显示文件大小
        fileSizeLabel->setText(QString("文件大小: %1 KB").arg(fileSize / 1024));

        // 超过1MB则不预览
        if (fileSize > 1 * 1024 * 1024) { // 1MB阈值
            previewTextEdit->setPlainText("文件过大（>1MB），无法预览");
            previewTextEdit->setEnabled(false);
            previewLabel->setEnabled(false);
        } else {
            // 尝试文本预览（处理可能的编码问题）
            QString content = QString::fromUtf8(fileContent);
            if (content.isEmpty()) {
                previewTextEdit->setPlainText("无法识别的文件内容（可能为二进制文件）");
            } else {
                previewTextEdit->setPlainText(content);
            }
            previewTextEdit->setEnabled(true);
            previewLabel->setEnabled(true);
        }
    });

    // 下载按钮逻辑（保持不变）
    connect(downloadButton, &QPushButton::clicked, [&previewDialog, fileListWidget, this]() {
        QListWidgetItem* item = fileListWidget->currentItem();
        if (!item) return;
        QString selectedFile = item->text();
        if (fileDataMap.contains(selectedFile)) {
            downloadSelectedFile(selectedFile);
            previewDialog.accept();
        }
    });

    previewDialog.exec();
}
void Widget::processIncomingData() {
    receivedData = msg::recieve(cltF);

    QString fullData = QString::fromUtf8(receivedData.join());
    if (fullData.startsWith("当前在线人数：")) {
        QString countStr = fullData.mid(7);
        ui->Usernumlabel->setText("当前在线人数："+countStr.mid(0,1));
        QString userListStr = countStr.mid(6);
        QStringList users = userListStr.split(",");

        qDebug() << "解析后的用户数量:" << users.size();
        qDebug() << "解析后的用户列表:" << users;

        // 清空表格（保留列数，仅删除行数据）
        ui->UsertableWidget->setRowCount(0);

        // 确保表格有两列（用户名和状态）
        if (ui->UsertableWidget->columnCount() != 2) {
            ui->UsertableWidget->setColumnCount(2);
            ui->UsertableWidget->setHorizontalHeaderLabels({"用户名", "状态"});
        }

        // 添加用户到表格
        for (const QString& user : users) {
            // 跳过空用户
            if (user.trimmed().isEmpty())
                continue;

            // 获取当前行数并插入新行
            int row = ui->UsertableWidget->rowCount();
            ui->UsertableWidget->insertRow(row);

            // 创建用户名项并设置文本
            QTableWidgetItem* nameItem = new QTableWidgetItem(user);
            ui->UsertableWidget->setItem(row, 0, nameItem);

            // 创建状态项并设置图标
            QTableWidgetItem* statusItem = new QTableWidgetItem;

            if (QFile::exists(":/new/prefix1/Images/OnLine.png")) {
                statusItem->setIcon(QIcon(":/new/prefix1/Images/OnLine.png"));
            }

            ui->UsertableWidget->setItem(row, 1, statusItem);
        }

        // 调整列宽以适应内容
        ui->UsertableWidget->resizeColumnsToContents();

        // 自动滚动到底部
        if (ui->UsertableWidget->rowCount() > 0) {
            ui->UsertableWidget->scrollToBottom();
        }
    }
    else
    {
        QStringList parts = fullData.split("|**|");

        for (int i = 0; i + 1 < parts.size(); i += 2) {
            QString fileName = parts[i];
            QByteArray fileContent = parts[i + 1].toUtf8();
            fileDataMap[fileName] = fileContent;
        }

        // 更新文件列表模型
        updateFileListModel();
    }
}

// 更新文件列表模型
void Widget::updateFileListModel() {
    fileListModel.clear();
    for (const QString& fileName : fileDataMap.keys()) {
        fileListModel.appendRow(new QStandardItem(fileName));
    }
}
void Widget::onFontChanged(const QFont &font)
{
    QTextCursor cursor = ui->MessagetextEdit->textCursor();

    if (cursor.hasSelection()) {
        QTextCharFormat format;
        format.setFont(font);
        cursor.mergeCharFormat(format);
        ui->MessagetextEdit->setTextCursor(cursor);
    } else {
        QTextCharFormat format;
        format.setFont(font);
        ui->MessagetextEdit->setCurrentCharFormat(format);
    }
}
void Widget::downloadSelectedFile(const QString& fileName) {
    // 打开文件选择对话框
    QString defaultPath = QDir::homePath() + "/" + fileName;
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "保存文件",
        defaultPath,
        "所有文件 (*)"
        );

    if (filePath.isEmpty()) {
        return; // 用户取消了保存操作
    }

    // 保存文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "保存文件", "无法打开文件: " + file.errorString());
        return;
    }

    file.write(fileDataMap[fileName]);
    file.close();

    QMessageBox::information(this, "保存文件", "文件已成功保存到: " + filePath);
}

void Widget::on_ExitTextpushButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "退出程序", "确定要退出聊天程序吗？",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 执行退出前的清理操作


        if (clt && clt->state() == QAbstractSocket::ConnectedState) {
            clt->close();
        }
        if (cltF && cltF->state() == QAbstractSocket::ConnectedState) {
            cltF->close();
        }


        if (client) {
            delete client;
            client = nullptr;
        }


        QApplication::quit();
    }
}

