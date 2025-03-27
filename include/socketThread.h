#ifndef QT_PROJ_SOCKETTHREAD_H
#define QT_PROJ_SOCKETTHREAD_H
#include <QThread>

class MyServer;
class MySocket;

class SocketHelper:public QObject{
    Q_OBJECT
public:
    explicit SocketHelper(QObject *parent = nullptr);
    MyServer* myServer;
public slots:
    void createSocket(qintptr socketDescriptor, int index);
signals:
    void create(qintptr socketDescriptor, int index);
    void addList(MySocket* tcpsocket, int index);
    void removeList(MySocket* tcpsocket);
};

class socketThread : public QThread{
    Q_OBJECT
public:
    explicit socketThread(QObject *parent);
    ~socketThread() override;
    MyServer* myServer;
    SocketHelper* socketHelper;
    int ThreadLoad;
    void run() override;
};

#endif //QT_PROJ_SOCKETTHREAD_H
