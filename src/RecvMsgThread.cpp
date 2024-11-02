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
    qDebug() << filePath;
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
void RecvMsgThread::initCsv(const QString& saveDir, const QString& csvName) {

    csvPath = saveDir + "/" + csvName;

    // 如果在这里初始化，会创建多余的子文件夹。
    QDir dir;
    if(!dir.exists(saveDir)){
        dir.mkpath(saveDir);
    }

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
