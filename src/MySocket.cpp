//
// Created by 13372 on 2024/5/21.
//

#include "MySocket.h"

MySocket::MySocket(QObject *parent) : m_tcpServer(dynamic_cast<MyServer*>(parent)){
    rmt = nullptr;
    msg_last = "";
    setFsrFactor();
}

MySocket::~MySocket(){
    this->write("CMD: stop record");
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
    QByteArray header = "\x5A\x55";

    msg = msg_last.append(tcpSocket->readAll());
    int msg_length = msg.size();
    int idx_header = 0;
    msg_last.clear();
    // header 2byte; time stamp 4byte; adc 2+24byte; imu 72yte, ip 4byte; total 108byte
    while(true){
        idx_header = msg.indexOf(header, idx_header);
        if(idx_header == -1){
            if(msg.back() == 0x5A){
                // 找不到包头的情况：正好结束或者包头被截断
                msg_last = "\x5A";
            }
            break;
        }
        if(idx_header+BYTE_LENGTH > msg_length){
            // 找到包头但最后一个数据不完整
            msg_last = msg.mid(idx_header);
            break;
        }
        // 正常处理数据
        QByteArray msg_slice = msg.mid(idx_header, BYTE_LENGTH);
        MsgData tmp_data(isLeft, msg_slice, leftFsrFactor, rightFsrFactor, leftTempOffset, rightTempOffset);
        if(ip==tmp_data.ipByte2Str()){
            rmt->qMutex->lock();
            rmt->msgQueue->enqueue(tmp_data);  // 在这里修改成一个函数
            rmt->qMutex->unlock();
        } else{
            qDebug() << "Wrong ip!";  // 如今其实没有串包的现象，类中需慎用静态变量。
        }

        idx_header++;
    }
    if (msg.size()<BYTE_LENGTH){
        qDebug() << "what msg? " << msg;
        emit addMsg(QString::fromLocal8Bit(msg));
    }
}

void MySocket::deal_write(const QByteArray& arr){
    this->write(arr);
}

void MySocket::deal_disconnect(){
    auto *tcpSocket = dynamic_cast<MySocket *>(sender());
    qDebug() << "deal disconnect:";
    if (tcpSocket==this){
        qDebug() << "socket disconnected:";
//        emit tcpSocket->writeMsg("CMD: stop record");
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

        //释放 手动删除会导致程序崩溃
//        tcpSocket->deleteLater();
    }
}

void MySocket::deal_delete(){
    auto* tcpSocket = dynamic_cast<MySocket*>(sender());
    //断开socket
    tcpSocket->abort();
    //断开所有信号连接
    tcpSocket->disconnect();
    //释放
//    tcpSocket->deleteLater();
}

void MySocket::initRMT(const QString& saveDir, const QString& csvName) {
    rmt = new RecvMsgThread();
    rmt->initCsv(saveDir, csvName);
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

void MySocket::setCsvPath(bool is_Left, const QString& name, const QString& saveDir) {
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString timeStr = currentDateTime.toString("yyMMdd-hh-mm-ss");
    QString foot = (is_Left) ? "left" : "right";
    QString csvName = QString("%1_%2_%3_%4.csv").arg(name, foot, timeStr, ip);
    initRMT(saveDir, csvName);
}

void MySocket::setCsvPath(const QString& name, const QString& saveDir) {
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString timeStr = currentDateTime.toString("MMdd-hh-mm-ss");
    QString dateStr = currentDateTime.toString("yyMMdd");
    QString type = "cali";
    QString csvDir = saveDir + "/" + dateStr;
    QString csvName = QString("%1_%2_%3_%4.csv").arg(name, type, timeStr, ip);
    initRMT(csvDir, csvName);
}

void MySocket::setLeft(bool flag) {
    isLeft = flag;
//    rmt->setLeft(flag);
}

void MySocket::setFsrFactor(const QString& size) {
    QString filePath = QString("../scripts/fsrFactor%1.json").arg(size);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open FSR JSON file.";
        return;
    }

    QByteArray jsonData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull()) {
        qDebug() << "Failed to parse JSON data.";
        return;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(rootObj.contains("Left FSR Factor") && rootObj.contains("Right FSR Factor")){
        QJsonObject leftObj = rootObj["Left FSR Factor"].toObject();
        QJsonObject rightObj = rootObj["Right FSR Factor"].toObject();
        for(int i=0;i<8;i++){
            leftFsrFactor[i] = leftObj[QString("fsr%1").arg(i+1)].toDouble();
            rightFsrFactor[i] = rightObj[QString("fsr%1").arg(i+1)].toDouble();
        }
        qDebug() << "Set factor: " << leftFsrFactor << "to " << filePath;
        qDebug() << "Set factor: " << rightFsrFactor << "to " << filePath;
    }
}

void MySocket::setTempOffset(const QString& size) {
    QString filePath = QString("../scripts/tempOffset%1.json").arg(size);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open FSR JSON file.";
        return;
    }

    QByteArray jsonData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull()) {
        qDebug() << "Failed to parse JSON data.";
        return;
    }
    QJsonObject rootObj = jsonDoc.object();
    if(rootObj.contains("Left NTC Offset") && rootObj.contains("Right NTC Offset")){
        QJsonObject leftObj = rootObj["Left NTC Offset"].toObject();
        QJsonObject rightObj = rootObj["Right NTC Offset"].toObject();
        for(int i=0;i<4;i++){
            leftTempOffset[i] = leftObj[QString("ntc%1").arg(i+1)].toDouble();
            rightTempOffset[i] = rightObj[QString("ntc%1").arg(i+1)].toDouble();
        }
        qDebug() << "Set offset: " << leftTempOffset << "to " << filePath;
        qDebug() << "Set offset: " << rightTempOffset << "to " << filePath;
    }
}


