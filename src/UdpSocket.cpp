#include "UdpSocket.h"
#include "mainwindow.h"

UdpSocket::UdpSocket(QObject *parent) : QObject(parent){
    udp_sock = nullptr;
}

UdpSocket::~UdpSocket(){
    udp_sock->close();
//    udp_sock->deleteLater();
//    delete udp_sock; 手动删除会导致报错
}

void UdpSocket::createSocket() {
    udp_sock = new QUdpSocket;
    connect(udp_sock, SIGNAL(readyRead()), this, SLOT(onReadyReadData()));
    udp_sock->bind(getIp(), 8265);
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

void UdpSocket::writeIpToESP(const QByteArray& data) {
    if(data == "Here is esp32s3."){
        QByteArray dataToSend = "IP:"+getIp().toString().toLatin1();
        qDebug() << dataToSend;
        udp_sock->writeDatagram(dataToSend, dataToSend.size(), udp_hostAddr, udp_port);
        emit addMsg(dataToSend);
    }
}

QHostAddress UdpSocket::getIp() {
    QList<QNetworkInterface> iFaceList = QNetworkInterface::allInterfaces();
    qDebug() << iFaceList.count();
    for(int i=0;i<iFaceList.count();i++){
        const auto& var = iFaceList.at(i);

        if (!(var.flags() & QNetworkInterface::IsUp) || !(var.flags() & QNetworkInterface::IsRunning)) {
            continue;
        }
        if (var.humanReadableName().contains("WLAN")) {
            QList<QNetworkAddressEntry> entryList = var.addressEntries();
            for (int j = 0; j < entryList.count(); j++) {
                const QNetworkAddressEntry &entry = entryList.at(j);
                if (entry.ip().protocol()==QAbstractSocket::IPv4Protocol) {
                    qDebug() << " IP地址：" << entry.ip().toString();
                    return entry.ip();
                }
            }
        }
    }
    return QHostAddress::AnyIPv4;
}

UdpThread::UdpThread(MyServer *server, QObject *parent) : QThread(parent){
    udpSocket = nullptr;
    myServer = server;
}

UdpThread::~UdpThread(){
    this->disconnect();
//    udpSocket->disconnect();
    udpSocket->deleteLater();
}

void UdpThread::run() {
    udpSocket = new UdpSocket();
    udpSocket->createSocket();
    connect(udpSocket, &UdpSocket::recvDataSignal, udpSocket, &UdpSocket::writeIpToESP);
    connect(this, &UdpThread::broadcastInst, udpSocket, &UdpSocket::writeIpToESP);
    connect(udpSocket, &UdpSocket::addMsg, myServer->mainWindow, &MainWindow::on_addServerMessage);
    exec();
}