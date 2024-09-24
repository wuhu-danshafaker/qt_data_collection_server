//
// Created by 13372 on 2024/5/21.
//

#include "socketThread.h"
#include "MyServer.h"
#include "MySocket.h"
#include "mainwindow.h"
#include <QDebug>

socketThread::socketThread(QObject *parent){
    this->myServer = dynamic_cast<MyServer*>(parent);
    this->socketHelper = nullptr;
    this->ThreadLoad = 0;
}

socketThread::~socketThread() {
    if(this->socketHelper != nullptr){
        qDebug() << "socketThread disconnecting";
        socketHelper->disconnect();
        socketHelper->deleteLater();
    }
}

void socketThread::run() {
    //在线程内创建对象，槽函数在这个线程中执行
    this->socketHelper = new SocketHelper(this->myServer);
    qRegisterMetaType<MySocket*>();  // ?
    connect(socketHelper,&SocketHelper::create,socketHelper,&SocketHelper::createSocket);
    connect(socketHelper,&SocketHelper::addList,myServer,&MyServer::addInfo);
    connect(socketHelper,&SocketHelper::removeList,myServer,&MyServer::removeInfo);

    exec();
}

SocketHelper::SocketHelper(QObject *parent){
    this->myServer = dynamic_cast<MyServer*>(parent);
}

void SocketHelper::createSocket(qintptr socketDescriptor, int index) {
    qDebug() << "subThread:" << QThread::currentThreadId();

    // socket 在 myServer 的 list information 中存储
    auto* tcpSocket = new MySocket(this->myServer);
    tcpSocket->socketHelper = this;
    tcpSocket->setSocketDescriptor(socketDescriptor);
    emit addList(tcpSocket, index);

    if(index!=-1){
        // 子线程
        myServer->list_thread[index]->ThreadLoad += 1;
        connect(tcpSocket, &MySocket::deleteSocket, tcpSocket, &MySocket::deal_delete, Qt::ConnectionType::BlockingQueuedConnection);
    } else{
        connect(tcpSocket, &MySocket::deleteSocket, tcpSocket, &MySocket::deal_delete, Qt::ConnectionType::AutoConnection);
    }

    connect(tcpSocket, &MySocket::addMsg, myServer->mainWindow, &MainWindow::on_addServerMessage);
    connect(tcpSocket, &MySocket::writeMsg, tcpSocket, &MySocket::deal_write);
    connect(tcpSocket, &MySocket::readyRead, tcpSocket, &MySocket::deal_readyRead);
    connect(tcpSocket, &MySocket::disconnected, tcpSocket, &MySocket::deal_disconnect);
    QString ip = tcpSocket->peerAddress().toString();
    quint16 port = tcpSocket->peerPort();
    tcpSocket->setIpAndPort(ip, port);

    QString message = QString("[%1:%2] 已连接").arg(ip.replace("::ffff:", "")).arg(port);
    qDebug() << message;

    // 发送到UI线程显示
    emit tcpSocket->addMsg(message);

}
