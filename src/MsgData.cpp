#include "MsgData.h"
#include <QDebug>
#include <utility>
//#include <QtMath>

MsgData::MsgData() {
    isEmpty = true;
}

void MsgData::byteInput(QByteArray msg) {
    byteMsg = std::move(msg);
    if (!byteMsg.isEmpty()){
        isEmpty = false;
    }
}

bool MsgData::MsgByteExplain() {
    if (byteMsg.isEmpty()){
        isEmpty = true;
        return false;
    }
    if (byteMsg.size()==102 && byteMsg.at(0) == 0x5A && byteMsg.at(1) == 0x55){
        unsigned long value = ((byteMsg.at(2)&0xFF) << 24) | ((byteMsg.at(3)&0xFF) << 16) | ((byteMsg.at(4)&0xFF) << 8) | (byteMsg.at(5)&0xFF);
        timeCounter = value;
        int adc_idx;
        for(int i=0;i<12;i++){
            adc_idx = i*2+6;
            adc_vol[i] = qbyte2int(byteMsg.at(adc_idx), byteMsg.at(adc_idx+1));
            if(i<8){
                fsr[i] = fsrVol2F(adc_vol[i]);
            } else{
                ntc[i-8] = ntcVol2T(adc_vol[i]);
            }
        }

        QByteArray imuArr;
        int imu_idx;
        for (int i=0;i<9;i++) {
            imu_idx = i*8+30;
            imuArr = byteMsg.mid(imu_idx, 8);
            imuAGE[i] = qbyte2double(imuArr);
        }
        return true;
    } else{
        return false;
    }
}

int MsgData::qbyte2int(char high, char low) {
    int value;
    value = ((high & 0xFF)<<8) + (low & 0xFF);
    return value;
}


double MsgData::qbyte2double(QByteArray src) {
    char arr[src.size()];
    memcpy(arr, src.data(), src.size());
    return *((double *)arr);
}

double MsgData::fsrVol2F(int vol) {
    // vol: mV
    double x = (double)vol/1000;
    return 18.199*x*x - 3.109*x + 9.8041;
}

double MsgData::mapF2R_fsr(double f) {
    return 7.848 * exp(-0.08216*f) + 15.62 * exp(-0.9062*f) + 1.067;
}

double MsgData::k_map(double f) {
    return -0.08216 * 7.848 * exp(-0.08216*f) - 0.9062 * 15.62 * exp(-0.9062*f);
}

double MsgData::ntcVol2T(int vol) {
    return (3795.9 - vol*5/4.98)/51 ;
}

//QQueue<MsgData> Msg_queue;
//QMutex mutexMsg;