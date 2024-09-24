#ifndef QT_PROJ_UDPSOCKET_H
#define QT_PROJ_UDPSOCKET_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QThread>
#include <QtNetwork>
#include "MyServer.h"

class UdpSocket : public QObject{
    Q_OBJECT
public:
    explicit UdpSocket(QObject *parent = nullptr);
    ~UdpSocket();

    void createSocket();

    void sendData(QString data);
    void setTargetInfo(QString ip, quint16 port);
    QString getIp();

signals:
    void recvDataSignal(QByteArray data);
    void addMsg(QString msg);

public slots:
    void onReadyReadData();
    void writeIpToESP(QByteArray data);

private:
    QUdpSocket *udp_sock;
    QHostAddress udp_hostAddr;
    quint16 udp_port;
};

class UdpThread : public QThread{
    Q_OBJECT
public:
    explicit UdpThread(MyServer *server, QObject *parent = nullptr);
    ~UdpThread() override;
    void run() override;
signals:
    void broadcastInst(QByteArray data);

private:
    MyServer *myServer;
    UdpSocket *udpSocket;
};

#endif //QT_PROJ_UDPSOCKET_H