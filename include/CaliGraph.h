//
// Created by 13372 on 2024/11/5.
//

#ifndef QT_PROJ_CALIGRAPH_H
#define QT_PROJ_CALIGRAPH_H
#include <QWidget>
#include "MsgData.h"
#include "../src_qCustomPlot/qcustomplot.h"
#include "MySocket.h"

class RecvMsgThread;

class CaliGraph  : public QWidget{
    Q_OBJECT
public:
    explicit CaliGraph(QWidget *parent = nullptr) : QWidget(parent){
        magXZ = nullptr;
        magYZ = nullptr;
        magXY = nullptr;

        Acc = nullptr;
        Gyro = nullptr;
        Euler = nullptr;

        sock = nullptr;
        isConnected = false;
    };
    CaliGraph(QCustomPlot *xz, QCustomPlot *yz, QCustomPlot *xy,
              QCustomPlot *acc, QCustomPlot *gyro, QCustomPlot *euler) :
            magXZ(xz), magYZ(yz), magXY(xy), Acc(acc), Gyro(gyro), Euler(euler){
        setupPlot();
        sock = nullptr;
        isConnected = false;
    };
    void setupPlot();
    void resetPlot(bool onlyMag=false);
    void setSocket(MySocket *mySocket);
    void startRecord();
    void pauseRecord();
    bool status();

    void startAccCali();
    void sendCaliCmd(QByteArray& cmd);

public slots:
    void updatePlot(const MsgData& msg);

private:
    QCustomPlot *magXZ{};
    QCustomPlot *magYZ{};
    QCustomPlot *magXY{};

    QCustomPlot *Acc{};
    QCustomPlot *Gyro{};
    QCustomPlot *Euler{};

    MySocket *sock;
    bool isConnected;
};


#endif //QT_PROJ_CALIGRAPH_H
