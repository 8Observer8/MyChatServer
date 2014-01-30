#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

class QTcpServer;
class QTcpSocket;

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    Ui::Dialog *ui;
    QTcpServer* m_ptcpServer;
    QTcpSocket* m_pTcpClientSocket;
    quint16     m_nNextBlockSize;
    void sendToClient(QTcpSocket* pSocket, const QString& str);

public slots:
    virtual void slotNewConnection();
    void slotReadClient();
private slots:
    void on_btnSend_clicked();
};

#endif // DIALOG_H
