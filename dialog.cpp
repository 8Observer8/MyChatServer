#include "dialog.h"
#include "ui_dialog.h"

#include <QMessageBox>
#include <QtNetwork>
#include <QNetworkInterface>
#include <QDebug>
//#include <QTime>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    m_nNextBlockSize(0)
{
    ui->setupUi(this);

    m_ptcpServer = new QTcpServer(this);
    int nPort = 1234;
    if (!m_ptcpServer->listen(QHostAddress::Any, nPort)) {
        QMessageBox::critical(0,
                              "Server Error",
                              "Unable to start the server:"
                              + m_ptcpServer->errorString()
                              );
        m_ptcpServer->close();
        return;
    }
    connect(m_ptcpServer, SIGNAL(newConnection()),
            this,         SLOT(slotNewConnection())
            );
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::sendToClient(QTcpSocket *pSocket, const QString &str)
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << quint16(0) << QTime::currentTime() << str;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    pSocket->write(arrBlock);
}

void Dialog::slotNewConnection()
{
    //QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();
    m_pTcpClientSocket = m_ptcpServer->nextPendingConnection();
    connect(m_pTcpClientSocket, SIGNAL(disconnected()),
            m_pTcpClientSocket, SLOT(deleteLater())
            );
    connect(m_pTcpClientSocket, SIGNAL(readyRead()),
            this,          SLOT(slotReadClient())
            );

    sendToClient(m_pTcpClientSocket, "Успешный отклик от сервера!");
    ui->teOutput->append(QTime::currentTime().toString()+" Друг подсоединился!");
}

void Dialog::slotReadClient()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QDataStream in(pClientSocket);
    in.setVersion(QDataStream::Qt_4_7);
    for (;;) {
        if (!m_nNextBlockSize) {
            if (pClientSocket->bytesAvailable() < (int)sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if (pClientSocket->bytesAvailable() < m_nNextBlockSize) {
            break;
        }
        QTime   time;
        QString str;
        in >> time >> str;

        QString strMessage =
                time.toString() + " " + "Друг: " + str;
        ui->teOutput->append(strMessage);

        m_nNextBlockSize = 0;
    }
}

void Dialog::on_btnSend_clicked()
{
    if (m_pTcpClientSocket != NULL) {
        sendToClient(m_pTcpClientSocket,
                     ui->leInput->text()
                     );
        ui->teOutput->append(QTime::currentTime().toString()+" Я: "+ui->leInput->text());
    } else {
        ui->teOutput->append(QTime::currentTime().toString()+" Я: "+"<i>неотправлено</i>"+" "+ui->leInput->text());
    }
    ui->leInput->setText("");
}
