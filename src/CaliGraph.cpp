//
// Created by 13372 on 2024/11/5.
//

#include "CaliGraph.h"

void CaliGraph::setupPlot() {
    if (magXZ && magYZ && magXY) {
        magXZ->plotLayout()->insertRow(0);
        magXZ->plotLayout()->addElement(0, 0, new QCPTextElement(magXZ, "mag XZ"));
        magXZ->xAxis->setRange(-75, 75);
        magXZ->yAxis->setRange(-75, 75);
        magXZ->addGraph(magXZ->xAxis, magXZ->yAxis)
                ->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 1));

        magYZ->plotLayout()->insertRow(0);
        magYZ->plotLayout()->addElement(0, 0, new QCPTextElement(magYZ, "mag YZ"));
        magYZ->xAxis->setRange(-75, 75);
        magYZ->yAxis->setRange(-75, 75);
        magYZ->addGraph(magYZ->xAxis, magYZ->yAxis)
                ->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 1));

        magXY->plotLayout()->insertRow(0);
        magXY->plotLayout()->addElement(0, 0, new QCPTextElement(magXY, "mag XY"));
        magXY->xAxis->setRange(-75, 75);
        magXY->yAxis->setRange(-75, 75);
        magXY->addGraph(magXY->xAxis, magXY->yAxis)
                ->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 1));

        magXZ->graph(0)->setLineStyle(QCPGraph::lsNone);
        magYZ->graph(0)->setLineStyle(QCPGraph::lsNone);
        magXY->graph(0)->setLineStyle(QCPGraph::lsNone);
    }

    if (Acc && Gyro && Euler) {
        Acc->plotLayout()->insertRow(0);
        Acc->plotLayout()->addElement(0, 0, new QCPTextElement(Acc, "Acc"));
        Acc->xAxis->setRange(200, 200, Qt::AlignRight);

        Gyro->plotLayout()->insertRow(0);
        Gyro->plotLayout()->addElement(0, 0, new QCPTextElement(Gyro, "Gyro"));
        Gyro->xAxis->setRange(200, 200, Qt::AlignRight);

        Euler->plotLayout()->insertRow(0);
        Euler->plotLayout()->addElement(0, 0, new QCPTextElement(Euler, "Euler"));
        Euler->xAxis->setRange(200, 200, Qt::AlignRight);

        QString legendName[3] = {"X", "Y", "Z"};
        QColor DataColor[3] = {
                QColor(29, 53, 87),
                QColor(131, 64, 38),
                QColor(78, 171, 144),
        };
        for (int i=0;i<3;i++) {
            auto accGraph = Acc->addGraph(Acc->xAxis, Acc->yAxis);
            accGraph->setName("Acc" + legendName[i]);
            accGraph->setPen(DataColor[i]);
            auto gyroGraph = Gyro->addGraph(Gyro->xAxis, Gyro->yAxis);
            gyroGraph->setName("Gyro" + legendName[i]);
            gyroGraph->setPen(DataColor[i]);
            auto eulerGraph = Euler->addGraph(Euler->xAxis, Euler->yAxis);
            eulerGraph->setName("Euler" + legendName[i]);
            eulerGraph->setPen(DataColor[i]);
        }
        Acc->legend->setVisible(true);
        Gyro->legend->setVisible(true);
        Euler->legend->setVisible(true);
    }
}


void CaliGraph::setSocket(MySocket *mySocket) {
    sock = mySocket;
    isConnected = true;
}

void CaliGraph::updatePlot(const MsgData &msg) {
    for (int i = 0; i < 3; i++) {
        Acc->graph(i)->addData(msg.timeCounter, msg.imuAGE[i]);
        Gyro->graph(i)->addData(msg.timeCounter, msg.imuAGE[i + 3]);
        Euler->graph(i)->addData(msg.timeCounter, msg.imuAGE[i + 6]);
        if (msg.timeCounter > 500) {
            Acc->graph(i)->data()->removeBefore(200);
            Gyro->graph(i)->data()->removeBefore(200);
            Euler->graph(i)->data()->removeBefore(200);
        }
    }
    Acc->xAxis->setRange((msg.timeCounter > 200) ? msg.timeCounter : 200, 200,
                         Qt::AlignRight);
    Acc->yAxis->rescale(true);
    Gyro->xAxis->setRange((msg.timeCounter > 200) ? msg.timeCounter : 200, 200,
                          Qt::AlignRight);
    Gyro->yAxis->rescale(true);
    Euler->xAxis->setRange((msg.timeCounter > 200) ? msg.timeCounter : 200, 200,
                           Qt::AlignRight);
    Euler->yAxis->rescale(true);

    double magX = msg.mag[0];
    double magY = msg.mag[1];
    double magZ = msg.mag[2];
    magXZ->graph(0)->addData(magX, magZ);
    magYZ->graph(0)->addData(magY, magZ);
    magXY->graph(0)->addData(magX, magY);

    Acc->replot(QCustomPlot::rpQueuedReplot);
    Gyro->replot(QCustomPlot::rpQueuedReplot);
    Euler->replot(QCustomPlot::rpQueuedReplot);
    magXZ->replot(QCustomPlot::rpQueuedReplot);
    magYZ->replot(QCustomPlot::rpQueuedReplot);
    magXY->replot(QCustomPlot::rpQueuedReplot);
}

void CaliGraph::resetPlot(bool onlyMag) {
    magXZ->graph(0)->data().data()->clear();
    magYZ->graph(0)->data().data()->clear();
    magXY->graph(0)->data().data()->clear();

    if(onlyMag) return;

    for(int i=0;i<3;i++){
        Acc->graph(i)->data().data()->clear();
        Gyro->graph(i)->data().data()->clear();
        Euler->graph(i)->data().data()->clear();
    }
    Acc->xAxis->setRange(200, 200, Qt::AlignRight);
    Gyro->xAxis->setRange(200, 200, Qt::AlignRight);
    Euler->xAxis->setRange(200, 200, Qt::AlignRight);

    Acc->replot(QCustomPlot::rpQueuedReplot);
    Gyro->replot(QCustomPlot::rpQueuedReplot);
    Euler->replot(QCustomPlot::rpQueuedReplot);
    magXZ->replot(QCustomPlot::rpQueuedReplot);
    magYZ->replot(QCustomPlot::rpQueuedReplot);
    magXY->replot(QCustomPlot::rpQueuedReplot);
}

void CaliGraph::startRecord() {
    if(!sock){
        QMessageBox::critical(this, "Failed", "Null Socket.");
        return;
    }
    QString saveDir = "../csvData/CaliData";
    sock->setCsvPath("cali", saveDir);
    RecvMsgThread *rmtForClient = sock->getRMT();
    connect(rmtForClient, &RecvMsgThread::resultReady, this, &CaliGraph::updatePlot);
    rmtForClient->resume();
    QByteArray cmd = "CMD: start record";
    emit sock->writeMsg(cmd);
}

void CaliGraph::pauseRecord() {
    if(!sock){
        QMessageBox::critical(this, "Failed", "Null Socket.");
        return;
    }
    RecvMsgThread *rmtForClient = sock->getRMT();
    disconnect(rmtForClient, &RecvMsgThread::resultReady, this, &CaliGraph::updatePlot);
    rmtForClient->pause();
    emit sock->writeMsg("CMD: pause record");
}

bool CaliGraph::status() {
    return isConnected && sock;
}

void CaliGraph::startAccCali() {
    if(sock && isConnected){
        emit sock->writeMsg("CMD: start accelerometer cali");
    }
}

void CaliGraph::sendCaliCmd(QByteArray &cmd) {
    if(sock && isConnected){
        emit sock->writeMsg(cmd);
    }
}


