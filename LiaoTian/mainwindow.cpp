#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QByteArray>
#include <QHostAddress>
#include <QThread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_tcpsock = nullptr;

    ui->textEdit->setEnabled(false);
    ui->textEdit_2->setEnabled(false);
    ui->pushButton_2->setEnabled(false);

    setWindowTitle("客户端");

    m_tcpsock = new QTcpSocket(this);

    connect(m_tcpsock, &QTcpSocket::connected, [=](){
        QMessageBox::information(this, "成功", "连接服务器成功!");
        ui->textEdit->setEnabled(true);
        ui->textEdit_2->setEnabled(true);
        ui->pushButton_2->setEnabled(true);
        ui->pushButton->setEnabled(false);
    });

    //获取对方发送内容
    connect(m_tcpsock, &QTcpSocket::readyRead, [=](){
        QByteArray array = m_tcpsock->readAll();
        ui->textEdit->append(tr("\n服务器:%1").arg(array.data()));
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

//连接
void MainWindow::on_pushButton_clicked()
{
    //获取ip和端口
    QString ip = ui->lineEdit->text();
    qint16 port = ui->lineEdit_2->text().toShort();

    //建立连接
    m_tcpsock->connectToHost(QHostAddress(ip), port);
    //发送用户名定义头协议:*##**#(userName)
    QString user = QString("*##**#%1").arg(ui->lineEdit_3->text());
    m_user = ui->lineEdit_3->text();

    m_tcpsock->write(user.toUtf8().data());
    //等待10ms防止粘包
    QThread::usleep(10);

}
//发送
void MainWindow::on_pushButton_2_clicked()
{
    QString str = ui->textEdit_2->toPlainText();
    ui->textEdit->append(tr("\n%1:%2").arg(m_user).arg(str));

    m_tcpsock->write(str.toUtf8().data());
    //清空
    ui->textEdit_2->clear();
}
//断开连接
void MainWindow::on_pushButton_3_clicked()
{
    if(m_tcpsock != nullptr)
    {
        m_tcpsock->disconnectFromHost();
        m_tcpsock->close();
    }
    ui->pushButton->setEnabled(true);
}
