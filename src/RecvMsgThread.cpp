#include "RecvMsgThread.h"
// 处理byte，转换为需要的数据的进程
RecvMsgThread::RecvMsgThread() {
    qMutex = new QMutex();
    msgQueue = new QQueue<MsgData>();
    csvPath = "../csvData/testData.csv";
    thread_running = false;

}

RecvMsgThread::~RecvMsgThread() {
    qDebug() << "delete rmt...";
    delete msgQueue;
    delete qMutex;
    this->disconnect();
}


void RecvMsgThread::run() {
    MsgData tmpData;
    QStringList dataOnce;
    qDebug() << "subThread msg: " << QThread::currentThreadId();

    while(thread_running){
        qMutex->lock();
        if (!msgQueue->isEmpty()){
            tmpData = msgQueue->dequeue();
            if(tmpData.MsgByteExplain()){
                dataOnce << QString::number(tmpData.timeCounter) << QString::number(tmpData.vcc);
                for (double i : tmpData.fsr){
                    dataOnce << QString::number(i);
                }
                for (double i : tmpData.ntc){
                    dataOnce << QString::number(i);
                }
                for (double i : tmpData.imuAGE){
                    dataOnce << QString::number(i);
                }
                for (int i : tmpData.adc_vol){
                    dataOnce << QString::number(i);
                }
                dataOnce << tmpData.ip_esp32s3;
                writeDataToCsv(csvPath, dataOnce);
                emit resultReady(tmpData);
                dataOnce.clear();
            }
        }
        qMutex->unlock();
    }

    exec();
}

void RecvMsgThread::writeDataToCsv(const QString &filePath, const QStringList &data) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)){
        qDebug() << "Failed to open file for writing." << file.errorString();
        return;
    }
    QByteArray csvData;

    for (int i=0;i<data.size();i++){
        csvData.append(data.at(i).toUtf8());
        if (i<data.size()-1){
            csvData.append(",");
        }
    }
    csvData.append("\n");

    file.write(csvData);
    file.close();
}

void RecvMsgThread::writeHeaderToCsv(const QString &filePath, const QStringList &data) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        qDebug() << "Failed to open file for writing." << file.errorString();
        return;
    }

    QByteArray csvData;
    for (int i=0;i<data.size();i++){
        csvData.append(data.at(i).toUtf8());
        if (i<data.size()-1){
            csvData.append(",");
        }
    }
    csvData.append("\n");

    file.write(csvData);
    file.close();
}

// 创建数据csv，输入表头
void RecvMsgThread::initCsv(const QString& csvDir, const QString& subDir, const QString& csvName) {

//    csvPath = csvDir + "/" + csvName;

    // 如果在这里初始化，会创建多余的子文件夹。
    QDir dir;
    QString targetDir;
    if(subDir=="Test"){
        targetDir = QString("%1/%2").arg(csvDir, subDir);
        if(!dir.exists(targetDir)){
            dir.mkpath(targetDir);
        }
    } else{
        int trailTimes = 1;
        targetDir = QString("%1/%2_%3").arg(csvDir, subDir, QString::number(trailTimes));
//        if(!dir.exists(csvDir)){
//            dir.mkpath(csvDir);
//        }
        while(dir.exists(targetDir)){
            trailTimes++;
            targetDir = QString("%1/%2_%3").arg(csvDir, subDir, QString::number(trailTimes));
        }
        dir.mkpath(targetDir);
    }


    csvPath = targetDir + "/" + csvName;

    QStringList header;
    header << "timeCounter" << "Vcc";
    for (int i=0;i<8;i++){
        header << "FSR"+QString::number(i);
    }
    for (int i=0;i<4;i++){
        header << "NTC"+QString::number(i);
    }
    header << "AccX" << "AccY" << "AccZ" << "GyroX" << "GyroY" << "GyroZ" << "EulerX" << "EulerY" << "EulerZ";
    for (int i=0;i<13;i++){
        header << "ADC"+QString::number(i);
    }
    header << "IP";
    writeHeaderToCsv(csvPath, header);
}

void RecvMsgThread::setLeft(bool flag) {
    isLeft = flag;
}

