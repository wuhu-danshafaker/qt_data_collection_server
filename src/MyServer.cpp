//
// Created by 13372 on 2024/5/21.
//

#include "MyServer.h"
#include "MySocket.h"
#include "mainwindow.h"

MyServer::MyServer(QObject *parent) :  QTcpServer(parent) {
    mainWindow = dynamic_cast<MainWindow*>(parent);
    this->socketHelper = new SocketHelper(this);
    qRegisterMetaType<qintptr>("qintptr");
    qRegisterMetaType<MySocket*>();
    // server所在线程的连接
    connect(socketHelper, &SocketHelper::create, socketHelper, &SocketHelper::createSocket);
    connect(socketHelper, &SocketHelper::addList, this, &MyServer::addInfo);
    connect(socketHelper, &SocketHelper::removeList, this, &MyServer::removeInfo);
}

MyServer::~MyServer(){
    //释放socket
    while(list_information.count()>0){
        emit list_information[0].getSocket()->deleteSocket();
        list_information.removeAt(0);
    }
    while(list_thread.count()>0){
        list_thread[0]->quit();
        list_thread[0]->wait();
        list_thread[0]->deleteLater();
        list_thread.removeAt(0);
    }
    socketHelper->disconnect();
    socketHelper->deleteLater();
}

//获取负载最少的线程索引
//-1:UI线程
int MyServer::GetMinLoadThread()
{
    //只有1个子线程
    if(list_thread.count()==1)
    {
        return 0;
    }
        //多个子线程
    else if(list_thread.count()>1)
    {
        int minLoad = list_thread[0]->ThreadLoad;
        int index=0;
        for(int i=1;i<list_thread.count();i++)
        {
            if(list_thread[i]->ThreadLoad < minLoad)
            {
                index = i;
                minLoad = list_thread[i]->ThreadLoad;
            }
        }
        return index;
    }
    //没有子线程
    return -1;
}

void MyServer::SetThread(int num) {
    /*
     * num:线程数；使用此函数会创建num个线程并启动之
     */
    for(int i=0;i<num;i++){
        list_thread.append(new socketThread(this));
        list_thread[i]->ThreadLoad = 0;
        list_thread[i]->start();
    }
}

void MyServer::incomingConnection(qintptr socketDescriptor) {
    int index = GetMinLoadThread();
    if(index!=-1){
        emit list_thread[index]->socketHelper->create(socketDescriptor, index);
//        qDebug() << "index connection:" << index;
    } else{
        emit socketHelper->create(socketDescriptor, index);
//        qDebug() << "index connection:" << index;
    }
}


//Q_DECLARE_METATYPE(MySocket*)

// edit socket info
void MyServer::addInfo(MySocket *mySocket, int index) {

    qDebug() << QThread::currentThreadId();
    QString ip = mySocket->peerAddress().toString();
    quint16 port = mySocket->peerPort();
    QString str_info = QString("[%1:%2]").arg(ip.replace("::ffff:", "")).arg(port);

    SocketInformation info(mySocket, str_info, index);
    this->list_information.append(info);
    qsizetype idx = list_information.size();
    this->mainWindow->AddClientComboBox(str_info, idx-1);
}

void MyServer::removeInfo(MySocket *mySocket) {
    // 不触发啊！
    // 因为myServer已经被删除，作为槽函数来不及运行
    qDebug() << "remove info ";
    for(int i=0;i<this->list_information.count();i++){
        if(this->list_information[i].getSocket() == mySocket){
            qDebug() << "remove: " << list_information[i].getIpAndPort();
            this->list_information.removeAt(i);
            this->mainWindow->removeClientComboBox(i);
            break;
        }
    }
}

SocketInformation::SocketInformation(MySocket *sock, QString& str, int idx) :
    mySocket(sock), ipInfo(str), threadIndex(idx){
}
