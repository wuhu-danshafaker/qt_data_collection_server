#include "FSRDisplay.h"

double median(double a, double b, double c){
    if((a<=b&&a>=c)||(a<=c&&a>=b)) return a;
    if((b<=a&&b>=c)||(b<=c&&b>=a)) return b;
    if((c<=a&&c>=b)||(c<=b&&c>=a)) return c;
    return -1;
}

void FSRDisplay::updateFootPrint(const MsgData& msg) {
    for(int i=0;i<8;i++){
        qreal ratio = std::min(msg.fsr[i]/40.0, 1.0);  // 3000!
        colors[i] = interpolate(Qt::red, QColor(85, 170, 127), ratio);
        circles[i]->setBrush(QBrush(colors[i]));
        int cnt = fsrPlot->graph(i)->data().data()->size();
        auto result = (cnt>3)? (median(msg.fsr[i], fsrPlot->graph(i)->data()->at(cnt-1)->value, fsrPlot->graph(i)->data
        ()->at
        (cnt-2)->value)) : msg.fsr[i];  //不对！导致没有变化了
        result = msg.fsr[i];
        fsrPlot->graph(i)->addData(msg.timeCounter, result); // 中位数滤波
        if(msg.timeCounter>500){
            fsrPlot->graph(i)->data()->removeBefore(200);
        }
    }

    QVector<double> temp;
    for(double t : msg.ntc){
        temp << t;
    }

    tempBar->setData({1,2,3,4},temp);
    for(int i=0;i<9;i++){
        ImuGraphs[i]->addData(msg.timeCounter, msg.imuAGE[i]);
        if(msg.timeCounter>500){
            ImuGraphs[i]->data()->removeBefore(200);
        }
    }

    // 原本借助 data counter 降低刷新频率，如今看来疑似没有必要
    fsrFootPrint->replot(QCustomPlot::rpQueuedReplot);
    fsrPlot->xAxis->setRange((msg.timeCounter > 200) ? msg.timeCounter : 200, 200, Qt::AlignRight);
    fsrPlot->yAxis->rescale();
    fsrPlot->replot(QCustomPlot::rpQueuedReplot);
    for(int i=0;i<3;i++){
        imuPlot->axisRect(i)->axis(QCPAxis::atBottom)->setRange((msg.timeCounter > 200) ? msg.timeCounter : 200, 200, Qt::AlignRight);
        imuPlot->axisRect(i)->axis(QCPAxis::atLeft)->rescale();
    }
    imuPlot->replot(QCustomPlot::rpQueuedReplot);
    tempPlot->replot(QCustomPlot::rpQueuedReplot);

}

QColor FSRDisplay::interpolate(QColor color1, QColor color2, qreal ratio) {
    int r = (int) std::min(ratio*color1.red() + (1-ratio)*color2.red(), 255.0);
    int g = (int) std::min(ratio*color1.green() + (1-ratio)*color2.green(), 255.0);
    int b = (int) std::min(ratio*color1.blue() + (1-ratio)*color2.blue(), 255.0);
    return {r, g, b};
}

void FSRDisplay::setPlotUI(QCustomPlot *pcustom) {
    fsrFootPrint = pcustom;
}

void FSRDisplay::setupPlot() {
    // footprint
    QCPAxisRect *axisRect = fsrFootPrint->axisRect();
    axisRect->setAutoMargins(QCP::msNone);
    axisRect->setMargins(QMargins(0, 0, 0, 0));
    fsrFootPrint->xAxis->setVisible(false);
    fsrFootPrint->yAxis->setVisible(false);

    //dynamic
    fsrPlot->legend->setVisible(true);

//    fsrFootPrint->addGraph();
    QColor DataColor[8] = {
            QColor(29, 53, 87),
            QColor(69, 123, 157),
            QColor(168, 218, 219),
            QColor(131, 64, 38),
            QColor(231, 56, 71),
            QColor(238, 191, 109),
            QColor(142, 182, 156),
            QColor(78, 171, 144),
    };

    QList<QPointF> circle_pos = (isLeft) ? circle_pos_left : circle_pos_right;
    for(int i=0;i<8;i++){
        circles[i] = new QCPItemEllipse(fsrFootPrint);
        circles[i]->setPen(QPen(Qt::black));
        circles[i]->topLeft->setCoords(circle_pos[i]);
        circles[i]->bottomRight->setCoords(circle_pos[i]+circleSize);
        circles[i]->setBrush(QBrush(QColor(85, 170, 127)));
        auto fsrGraph = fsrPlot->addGraph();
        fsrGraph->setName("fsr " + QString::number(i+1));
        fsrGraph->setVisible(true);
        fsrGraph->setPen(DataColor[i]);
        fsrPlot->legend->item(i)->setVisible(true);
    }

    fsrPlot->xAxis->setRange(200, 200, Qt::AlignRight);
    fsrPlot->yAxis->setRange(0, 50);

    auto *fsrLegendLayout = new QCPLayoutGrid;
    fsrPlot->plotLayout()->addElement(0, 1, fsrLegendLayout);
    fsrLegendLayout->setMargins(QMargins(0, 5, 5, 50));
    fsrLegendLayout->addElement(0, 0, fsrPlot->legend);
    fsrPlot->plotLayout()->setColumnStretchFactor(1, 0.001);
//    fsrPlot->legend->item(0)->setVisible(true);

    auto *imuAcc = new QCPAxisRect(imuPlot);
    auto *imuGyro = new QCPAxisRect(imuPlot);
    auto *imuEuler = new QCPAxisRect(imuPlot);
    imuPlot->plotLayout()->clear();
    imuPlot->plotLayout()->addElement(0, 0, imuAcc);
    imuPlot->plotLayout()->addElement(1, 0, imuGyro);
    imuPlot->plotLayout()->addElement(2, 0, imuEuler);
    for(int i=0;i<3;i++){
        imuPlot->axisRect(i)->axis(QCPAxis::atBottom)->setRange(200, 200, Qt::AlignRight);
//        imuPlot->axisRect(i)->axis(QCPAxis::atLeft)->rescale();
        imuPlot->axisRect(i)->axis(QCPAxis::atLeft)->setRange(-1,12);
    }

    QString legendName[3] = {"X", "Y", "Z"};
    for(int i=0;i<3;i++){
        ImuGraphs[i] = imuPlot->addGraph(imuAcc->axis(QCPAxis::atBottom), imuAcc->axis(QCPAxis::atLeft));
        ImuGraphs[i]->setName("Acc" + legendName[i]);
        ImuGraphs[i]->setPen(DataColor[i*3+1]);
        ImuGraphs[i+3] = imuPlot->addGraph(imuGyro->axis(QCPAxis::atBottom), imuGyro->axis(QCPAxis::atLeft));
        ImuGraphs[i+3]->setName("Gyro" + legendName[i]);
        ImuGraphs[i+3]->setPen(DataColor[i*3+1]);
        ImuGraphs[i+6] = imuPlot->addGraph(imuEuler->axis(QCPAxis::atBottom), imuEuler->axis(QCPAxis::atLeft));
        ImuGraphs[i+6]->setName("Euler" + legendName[i]);
        ImuGraphs[i+6]->setPen(DataColor[i*3+1]);
    }
    imuPlot->setAutoAddPlottableToLegend(true);
//    imuPlot->legend->setVisible(true);

    tempBar = new QCPBars(tempPlot->xAxis, tempPlot->yAxis);
    tempBar->setAntialiased(false);
    tempBar->setName("Temperature");
    tempBar->setPen(QPen(QColor(0, 168, 140).lighter(130)));
    tempBar->setBrush(QColor(214, 120, 120));
    QVector<double> ticks;
    QVector<QString> labels;
    ticks << 1 << 2 << 3 << 4;
    labels << "NTC 1" << "NTC 2" << "NTC 3" << "NTC 4";
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(ticks, labels);
    tempPlot->xAxis->setTicker(textTicker);
    tempPlot->xAxis->setSubTicks(false);
    tempPlot->yAxis->setSubTicks(false);
    tempPlot->xAxis->setRange(0, 5);
    tempPlot->yAxis->setRange(20, 40);
}

void FSRDisplay::setDynamicUI(QCustomPlot *pcustom) {
    fsrPlot = pcustom;
}

// set subgraph i(0~7) to be visible/invisible
void FSRDisplay::showFsr(int i) {
    if(i>7||i<0) {
       qDebug() << "Subgraph index exceeded range";
       return;
    }

    bool vis = fsrPlot->graph(i)->visible();
    fsrPlot->graph(i)->setVisible(!vis);
    fsrPlot->legend->item(i)->setVisible(!vis);
    fsrPlot->replot(QCustomPlot::rpQueuedReplot);
}

void FSRDisplay::setImuUI(QCustomPlot *pcustom) {
    imuPlot = pcustom;
}

void FSRDisplay::setTempUI(QCustomPlot *pcustom) {
    tempPlot = pcustom;
}

void FSRDisplay::setIdx(qsizetype idx) {
    index = idx;
}

void FSRDisplay::setSocket(MySocket *sock) {
    socket = sock;
    socket->setLeft(isLeft);
}

void FSRDisplay::clearSocket() {
    socket = nullptr;
}

void FSRDisplay::checkConnection() {
    if(!socket){
        QString lor = (isLeft) ? "left" : "right";
        qDebug() << lor + " 尚未绑定socket!";
        return;
    }
    QByteArray cmd = "Check Connection";
    emit socket->writeMsg(cmd);  // emit以后 槽函数会在socketThread中运行而非主线程
}

void FSRDisplay::startDisplay(const QString& name, const QString& saveDir, bool isResume) {
    if(!socket){
        QString lor = (isLeft) ? "left" : "right";
        qDebug() << lor + " 尚未绑定socket!";
        return;
    }
    socket->setCsvPath(isLeft, name, saveDir);
    RecvMsgThread *rmtForClient = socket->getRMT();
    connect(rmtForClient, &RecvMsgThread::resultReady, this, &FSRDisplay::updateFootPrint);
    rmtForClient->resume();
    QByteArray cmd = "CMD: start record";
    emit socket->writeMsg(cmd);  // emit以后 槽函数会在socketThread中运行而非主线程
}

void FSRDisplay::pauseDisplay() {
    if(!socket){
        QString lor = (isLeft) ? "left" : "right";
        qDebug() << lor + " 尚未绑定socket!";
        return;
    }
    RecvMsgThread *rmtForClient = socket->getRMT();
    qDebug()<<"socket thread disconnecting";
    disconnect(rmtForClient, &RecvMsgThread::resultReady, this, &FSRDisplay::updateFootPrint);
    rmtForClient->pause();
    emit socket->writeMsg("CMD: pause record");
    // 断了以后要重新连接，然而在此之前居然已经删掉了socket的连接，这是因为stop之后esp端主动切断此连接了。
}

void FSRDisplay::setIsLeft(bool flag) {
    isLeft = flag;
}

void FSRDisplay::resetPlot() {
    if(!fsrPlot || !imuPlot || !tempPlot) return;
    // fsr
    for(int i=0;i<8;i++){
        fsrPlot->graph(i)->data().data()->clear();
    }
    // imu
    for(auto imuGraph : ImuGraphs){
        imuGraph->data().data()->clear();
    }
    //ntc
    tempBar->data().data()->clear();
    fsrPlot->xAxis->setRange(200, 200, Qt::AlignRight);
    fsrPlot->yAxis->setRange(0, 50);
    for(int i=0;i<3;i++){
        imuPlot->axisRect(i)->axis(QCPAxis::atBottom)->setRange(200, 200, Qt::AlignRight);
        imuPlot->axisRect(i)->axis(QCPAxis::atLeft)->setRange(-1,12);
    }
    fsrPlot->replot(QCustomPlot::rpQueuedReplot);
    imuPlot->replot(QCustomPlot::rpQueuedReplot);
    tempPlot->replot(QCustomPlot::rpQueuedReplot);
}
