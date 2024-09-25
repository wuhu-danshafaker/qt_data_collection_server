#include "UdpSocket.h"
#include "mainwindow.h"

UdpSocket::UdpSocket(QObject *parent) : QObject(parent){
    udp_sock = nullptr;
}

UdpSocket::~UdpSocket(){
    udp_sock->close();
    delete udp_sock;
}

void UdpSocket::createSocket() {
    udp_sock = new QUdpSocket;

//    udp_sock->bind(8265, QUdpSocket::ShareAddress);
    connect(udp_sock, SIGNAL(readyRead()), this, SLOT(onReadyReadData()));
    udp_sock->bind(QHostAddress::AnyIPv4, 8265);
}

void UdpSocket::sendData(QString data) {
    udp_sock->writeDatagram(data.toUtf8(), udp_hostAddr, udp_port);
}

void UdpSocket::setTargetInfo(QString ip, quint16 port) {
    udp_hostAddr = QHostAddress(ip);
    udp_port = port;
}

void UdpSocket::onReadyReadData() {
    while(udp_sock->hasPendingDatagrams()){
        QByteArray data;
        data.resize(udp_sock->pendingDatagramSize());
        udp_sock->readDatagram(data.data(), data.size(), &udp_hostAddr, &udp_port);
        emit recvDataSignal(data);
        emit addMsg(QString("UDP [%1] : %3").arg(udp_hostAddr.toString(), data));
    }
}

void UdpSocket::writeIpToESP(QByteArray data) {
    if(data == "Here is esp32s3."){
        QByteArray dataToSend = "IP:"+getIp().toLatin1();
        qDebug() << dataToSend;
        udp_sock->writeDatagram(dataToSend, dataToSend.size(), udp_hostAddr, udp_port);
        emit addMsg(dataToSend);
    }
}

QString UdpSocket::getIp() {
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list){
        if(address.protocol()==QAbstractSocket::IPv4Protocol)
            return address.toString();
    }
    return "";
}

UdpThread::UdpThread(MyServer *server, QObject *parent) : QThread(parent){
    udpSocket = nullptr;
    myServer = server;
}

UdpThread::~UdpThread(){
    this->disconnect();
    udpSocket->disconnect();
    delete udpSocket;
}

void UdpThread::run() {
    udpSocket = new UdpSocket();
    udpSocket->createSocket();
    connect(udpSocket, &UdpSocket::recvDataSignal, udpSocket, &UdpSocket::writeIpToESP);
    connect(this, &UdpThread::broadcastInst, udpSocket, &UdpSocket::writeIpToESP);
    connect(udpSocket, &UdpSocket::addMsg, myServer->mainWindow, &MainWindow::on_addServerMessage);
    exec();
}