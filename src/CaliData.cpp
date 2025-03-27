//
// Created by 13372 on 2025/2/21.
//

#include "CaliData.h"

CaliData::CaliData(bool flag) {
    isLeft = flag;
}

void CaliData::initTemp(int rows, int cols) {
    temp.resize(rows);
    for (int i = 0; i < rows; ++i) {
        temp[i].resize(cols);
    }
}

void CaliData::getCaliData(MsgData msg) {
    if(msg.getIsLeft() != isLeft){
        qDebug() << "Cali: isLeft not match";
        return;
    }
    if (idx>=data.size()){
//        qDebug() << "Cali Completed Already";
        return;
    }
    double sum = 0;
    for (double i : msg.fsr_raw){
        sum += i;
    }
    data[idx] = sum;
    for (int i = 0; i < 4; ++i) {
        temp[idx][i] = msg.ntc[i];
    }

    idx++;
    if(idx==data.size()){
        if(!isInTempCali){
            double val = std::accumulate(data.begin(), data.end(), 0.0)/data.size();
            emit this->caliRes(val);
        }else{
            std::vector<double> tempMeans(4, 0.0); // 初始化均值向量为 0

            // 遍历每一列，使用 std::accumulate 计算列的和
            for (int col = 0; col < 4; ++col) {
                tempMeans[col] = std::accumulate(temp.begin(), temp.end(), 0.0,
                                                 [col](double acc, const QVector<double>& row) {
                                                     return acc + row[col];
                                                 }) / data.size(); // 计算均值
            }
            emit this->caliTemp(tempMeans);
        }
        qDebug() << "Cali Completed";
    }
}

void CaliData::clearData() {
    for(auto i: data){
        i = 0;
    }
    idx = 0;
}

void CaliData::resize(int size) {
    data.resize(size);
    initTemp(size, 4);
}




