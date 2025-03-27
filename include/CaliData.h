//
// Created by 13372 on 2025/2/21.
//

#ifndef QT_PROJ_CALIDATA_H
#define QT_PROJ_CALIDATA_H

#include "MsgData.h"


class CaliData : public QWidget {
    Q_OBJECT
public:
    explicit CaliData(QWidget *parent = nullptr) : QWidget(parent), data(200){
        initTemp(200, 4);
    };
    explicit CaliData(bool flag, int size) : isLeft(flag), data(size){
        initTemp(size, 4);
    };
    explicit CaliData(bool flag);

    void clearData();
    void resize(int size);
    int getSize(){
        return data.size();
    }

    bool isInTempCali = false;

private:
    QVector<double> data;
    QVector<QVector<double>> temp;
    void initTemp(int rows, int cols);
    int idx = 0;
    bool isLeft;

signals:
    void caliRes(double meanVal);
    void caliTemp(const std::vector<double>& tempMean);

public slots:
    void getCaliData(MsgData msg);
};


#endif //QT_PROJ_CALIDATA_H
