#ifndef QT_PROJ_MYSOCKET_H
#define QT_PROJ_MYSOCKET_H

#include <QTcpSocket>
#include "MsgData.h"
#include "RecvMsgThread.h"
#include "socketThread.h"
#include "MyServer.h"

class MyServer;
class SocketHelper;

class MySocket : public QTcpSocket{
    Q_OBJECT
public:
    explicit MySocket(QObject *parent = nullptr);
    ~MySocket() override;

    [[maybe_unused]] MyServer* m_tcpServer;
    SocketHelper* socketHelper;

    RecvMsgThread* getRMT(){
        return rmt;
    }

    void setIpAndPort(QString ipInfo, quint16 portInfo);

    void initRMT(const QString& saveDir, const QString& csvName);
    void stopRMT();

    void setCsvPath(bool isLeft, const QString& name, const QString& saveDir);
    void setCsvPath(const QString& name, const QString& saveDir);
    void setLeft(bool flag);
    void setFsrFactor(const QString& size="");
    void setTempOffset(const QString& size="");

private:
    RecvMsgThread *rmt;
    QString ip;
    quint16 port;

    QByteArray msg_last = "";

    bool isLeft;

    QVector<double> leftFsrFactor = {1,1,1,1,1,1,1,1};
    QVector<double> rightFsrFactor = {1,1,1,1,1,1,1,1};
    QVector<double> leftTempOffset = {0,0,0,0};
    QVector<double> rightTempOffset = {0,0,0,0};

public slots:
    void deal_readyRead();
    void deal_disconnect();
    void deal_delete();
    void deal_write(const QByteArray& arr);

signals:
    void addMsg(QString data); // UI显示
    void writeMsg(QByteArray arr); // UI发送的数据
    void deleteSocket();


};
//Q_DECLARE_METATYPE(MySocket)
//Q_DECLARE_OPAQUE_POINTER(MySocket*)
#endif //QT_PROJ_MYSOCKET_H
