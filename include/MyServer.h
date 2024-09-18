#ifndef QT_PROJ_MYSERVER_H
#define QT_PROJ_MYSERVER_H

#include <QTcpServer>
#include "socketThread.h"

class MainWindow;
class MySocket;

class SocketInformation
{
private:
    MySocket* mySocket;//socket指针
    QString ipInfo;//ip端口字符串
    int threadIndex;//所在线程ID
public:
    SocketInformation(MySocket* sock, QString& str, int idx);

    MySocket* getSocket(){
        return mySocket;
    }
    QString getIpAndPort(){
        return ipInfo;
    }
    int getThreadIdx(){
        return threadIndex;
    }
};

class MyServer : public QTcpServer{
    Q_OBJECT
public:
    explicit MyServer(QObject *parent = nullptr);
    ~MyServer() override;
    void SetThread(int num);
    int GetMinLoadThread();

    SocketHelper* socketHelper;
    QList<socketThread*> list_thread;
    QList<SocketInformation> list_information;
    MainWindow *mainWindow;

private:
    void incomingConnection(qintptr socketDescriptor) override;

public slots:
    void addInfo(MySocket* mySocket, int index);
    void removeInfo(MySocket* mySocket);
};

Q_DECLARE_METATYPE(SocketInformation)
#endif //QT_PROJ_MYSERVER_H
