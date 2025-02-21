#ifndef QT_PROJ_RECVMSGTHREAD_H
#define QT_PROJ_RECVMSGTHREAD_H
#include <QObject>
#include <QThread>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include "MsgData.h"

class RecvMsgThread: public QThread{
    Q_OBJECT

public:
    RecvMsgThread();
    ~RecvMsgThread() override;
    virtual void run();

    static void writeDataToCsv(const QString &filePath, const QStringList &data);
    static void writeHeaderToCsv(const QString &filePath, const QStringList &data);
    void initCsv(const QString& saveDir, const QString& csvName);
    void pause(){
        thread_running = false;
    }
    // set running flag true
    void resume(){
        thread_running = true;
    }
//    void setLeft(bool flag);
    QQueue<MsgData> *msgQueue;
    QMutex *qMutex;

private:
    bool thread_running;
    QString csvPath;
//    double leftFsrFactor[8] = {1,1,1,1,1,1,1,1};
//    double rightFsrFactor[8] = {1,1,1,1,1,1,1,1};
//    bool isLeft;

signals:
    void resultReady(MsgData result);
};

#endif //QT_PROJ_RECVMSGTHREAD_H
