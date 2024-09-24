//
// Created by 13372 on 2024/5/21.
//

#include "MySocket.h"

MySocket::MySocket(QObject *parent) : m_tcpServer(dynamic_cast<MyServer*>(parent)){
    rmt = nullptr;
}

MySocket::~MySocket(){
    rmt->pause();
    rmt->quit();
    rmt->wait();
    rmt->deleteLater();
    qDebug()<<"释放Socket,所在线程："<<QThread::currentThreadId();
}

void MySocket::deal_readyRead(){
    auto* tcpSocket = dynamic_cast<MySocket*>(sender());

    //获取客户端发来的数据
    QByteArray msg;
    static QByteArray msg_last = "";
    QByteArray header = "\x5A\x55";

    msg = msg_last.append(tcpSocket->readAll());
    int msg_length = msg.size();
    int idx_header = 0;
    msg_last.clear();
    // header 2byte; time stamp 4byte; adc 24byte; imu 72yte; total 102byte
    while(true){
        idx_header = msg.indexOf(header, idx_header);
        if(idx_header == -1){
            if(msg.back() == 0x5A){
                // 找不到包头的情况：正好结束或者包头被截断
                msg_last = "\x5A";
            }
            break;
        }
        if(idx_header+102 > msg_length){
            // 找到包头但最后一个数据不完整
            msg_last = msg.mid(idx_header);
            break;
        }
        // 正常处理数据
        QByteArray msg_slice = msg.mid(idx_header, 102);
        emit addMsg(QString("[%1：%2] \n%3").arg(ip).arg(port).arg(msg_slice.toHex().toUpper()));
        MsgData tmp_data;
        tmp_data.byteInput(msg_slice);
        rmt->qMutex->lock();
        rmt->msgQueue->enqueue(tmp_data);  // 在这里修改成一个函数
        rmt->qMutex->unlock();
        idx_header++;
    }
    if (msg.size()<102){
        qDebug() << "initial message";
        emit addMsg(QString::fromLocal8Bit(msg));
    }
}

void MySocket::deal_write(const QByteArray& arr){
    this->write(arr);
}

void MySocket::deal_disconnect(){

    qDebug() << "socket disconnected:";
    auto *tcpSocket = dynamic_cast<MySocket *>(sender());
    //断开socket
    tcpSocket->abort();
    //消息提示断开

    QString message = QString("[%1:%2] 已断开").arg(ip).arg(port);
    qDebug() << message;
    //发送到UI线程显示
    emit addMsg(message);
    //断开所有信号连接
    //tcpSocket->disconnect();
    //发送到UI线程移除信息
    qDebug() << "socket remove:" << tcpSocket;
    emit socketHelper->removeList(tcpSocket);
    //释放
    tcpSocket->deleteLater();

}

void MySocket::deal_delete(){
    auto* tcpSocket = dynamic_cast<MySocket*>(sender());
    //断开socket
    tcpSocket->abort();
    //断开所有信号连接
    tcpSocket->disconnect();
    //释放
    tcpSocket->deleteLater();
}

void MySocket::initRMT(const QString& csvDir, const QString& csvName) {
    rmt = new RecvMsgThread();
    rmt->initCsv(csvDir, csvName);
    rmt->resume();
    rmt->start();
}

void MySocket::stopRMT() {
    rmt->pause();
}

void MySocket::setIpAndPort(QString ipInfo, quint16 portInfo) {
    ip = ipInfo.replace("::ffff:", "");
    port = portInfo;
}

void MySocket::setCsvPath(bool isLeft, const QString& name) {
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString timeStr = currentDateTime.toString("MM_dd_hh_mm");
    QString foot = (isLeft) ? "left" : "right";
    QString csvDir = QString("../csvData/%1").arg(name);
    QString csvName = QString("%1_%2_%3.csv").arg(ip, foot, timeStr);
    initRMT(csvDir, csvName);
}

