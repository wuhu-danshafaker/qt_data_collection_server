//
// Created by 13372 on 2024/5/10.
//

#ifndef QT_PROJ_FSRDISPLAY_H
#define QT_PROJ_FSRDISPLAY_H
#include <QWidget>
#include "MsgData.h"
#include "../src_qCustomPlot/qcustomplot.h"
#include "MySocket.h"

class RecvMsgThread;
class MySocket;

class FSRDisplay : public QWidget{
    Q_OBJECT
public:
    explicit FSRDisplay(QWidget *parent = nullptr) : QWidget(parent){
        setFixedSize(230, 610);
        dataCounter = 10;
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
    void startDisplay(const QString& name, bool isResume = false);
    void pauseDisplay();
    void setIsLeft(bool flag);
    void setFsrMap();

public slots:
    void updateFootPrint(const MsgData& msg);

private:
    int dataCounter;
    QCustomPlot *fsrFootPrint;
    static QColor interpolate(QColor color1, QColor color2, qreal ratio);

    QCPItemEllipse *circles[8];

    bool isLeft;
    QList<QPointF> circle_pos_left = {
            // top left
            QPointF(132/230.0*5, (771-29)/771.0*5),
            QPointF(151/230.0*5, (771-132)/771.0*5),
            QPointF(82/230.0*5, (771-148)/771.0*5),
            QPointF(25/230.0*5, (771-188)/771.0*5),
            QPointF(34/230.0*5, (771-274)/771.0*5),
            QPointF(135/230.0*5, (771-449)/771.0*5),
            QPointF(66/230.0*5, (771-465)/771.0*5),
            QPointF(111/230.0*5, (771-532)/771.0*5)
    };
    QList<QPointF> circle_pos_right = {
            // top left
            QPointF(54/230.0*5, (771-29)/771.0*5),
            QPointF(35/230.0*5, (771-132)/771.0*5),
            QPointF(104/230.0*5, (771-148)/771.0*5),
            QPointF(161/230.0*5, (771-188)/771.0*5),
            QPointF(152/230.0*5, (771-274)/771.0*5),
            QPointF(51/230.0*5, (771-449)/771.0*5),
            QPointF(120/230.0*5, (771-465)/771.0*5),
            QPointF(75/230.0*5, (771-532)/771.0*5)
    };
    QPointF circleSize = QPointF(45/230.0*5, -45/771.0*5);
    QColor colors[8] = {QColor(30, 144, 255),QColor(30, 144, 255),
                        QColor(30, 144, 255),QColor(30, 144, 255),
                        QColor(30, 144, 255),QColor(30, 144, 255),
                        QColor(30, 144, 255),QColor(30, 144, 255)
    };

    QCustomPlot *fsrPlot;
//    QCPGraph *FsrGraphs[8]; fsrPlot->graph(i);

    QCustomPlot *imuPlot;
    QCPGraph *ImuGraphs[9];

    QCustomPlot *tempPlot;
    QCPBars *tempBar;

    qsizetype index;
    MySocket *socket;

    int fsrMap[8]={0};
};


#endif //QT_PROJ_FSRDISPLAY_H
