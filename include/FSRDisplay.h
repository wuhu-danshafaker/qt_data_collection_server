//
// Created by 13372 on 2024/5/10.
//

#ifndef QT_PROJ_FSRDISPLAY_H
#define QT_PROJ_FSRDISPLAY_H
#include <QWidget>
#include "MsgData.h"
#include "../src_qCustomPlot/qcustomplot.h"
#include "MySocket.h"

#define Height 610.0

class RecvMsgThread;
class MySocket;

class FSRDisplay : public QWidget{
    Q_OBJECT
public:
    explicit FSRDisplay(QWidget *parent = nullptr) : QWidget(parent){
        setFixedSize(230, 610);
        index = -1;
        socket = nullptr;
    }
    void setPlotUI(QCustomPlot *pcustom);
    void setDynamicUI(QCustomPlot *pcustom);
    void setImuUI(QCustomPlot *pcustom);
    void setTempUI(QCustomPlot *pcustom);
    void setupPlot();
    void resetPlot();
    void showFsr(int i);
    void setIdx(qsizetype idx);
    qsizetype getIdx(){
        return index;
    }
    void setSocket(MySocket *sock);
    void clearSocket();
    void checkConnection();
    void startDisplay(const QString& name, const QString& saveDir, bool isResume = false);
    void pauseDisplay();
    void setIsLeft(bool flag);
//    void setFsrNtcMap();

public slots:
    void updateFootPrint(const MsgData& msg);

private:
    QCustomPlot *fsrFootPrint;
    static QColor interpolate(QColor color1, QColor color2, qreal ratio);

    QCPItemEllipse *circles[8];

    bool isLeft;
    QList<QPointF> circle_pos_left = {
            // top left
            QPointF(132/230.0*5, (Height-29)/Height*5),
            QPointF(151/230.0*5, (Height-132)/Height*5),
            QPointF(82/230.0*5, (Height-148)/Height*5),
            QPointF(25/230.0*5, (Height-188)/Height*5),
            QPointF(34/230.0*5, (Height-274)/Height*5),
            QPointF(135/230.0*5, (Height-449)/Height*5),
            QPointF(66/230.0*5, (Height-465)/Height*5),
            QPointF(111/230.0*5, (Height-532)/Height*5)
    };
    QList<QPointF> circle_pos_right = {
            // top left
            QPointF(54/230.0*5, (Height-29)/Height*5),
            QPointF(35/230.0*5, (Height-132)/Height*5),
            QPointF(104/230.0*5, (Height-148)/Height*5),
            QPointF(161/230.0*5, (Height-188)/Height*5),
            QPointF(152/230.0*5, (Height-274)/Height*5),
            QPointF(51/230.0*5, (Height-449)/Height*5),
            QPointF(120/230.0*5, (Height-465)/Height*5),
            QPointF(75/230.0*5, (Height-532)/Height*5)
    };
    QPointF circleSize = QPointF(45/230.0*5, -45/Height*5);
    QColor colors[8] = {QColor(30, 144, 255),QColor(30, 144, 255),
                        QColor(30, 144, 255),QColor(30, 144, 255),
                        QColor(30, 144, 255),QColor(30, 144, 255),
                        QColor(30, 144, 255),QColor(30, 144, 255)
    };

    QCustomPlot *fsrPlot;

    QCustomPlot *imuPlot;
    QCPGraph *ImuGraphs[9];

    QCustomPlot *tempPlot;
    QCPBars *tempBar;

    qsizetype index;
    MySocket *socket;

};


#endif //QT_PROJ_FSRDISPLAY_H
