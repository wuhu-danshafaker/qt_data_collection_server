#include "MsgData.h"
#include <QDebug>
#include <utility>
//#include <QtMath>

MsgData::MsgData() {
    isEmpty = true;
    vcc = 0;
}

void MsgData::byteInput(QByteArray msg) {
    byteMsg = std::move(msg);
    if (!byteMsg.isEmpty()){
        isEmpty = false;
    }
    Q_ASSERT(byteMsg.size()==BYTE_LENGTH);
}

bool MsgData::MsgByteExplain() {
    if (byteMsg.isEmpty()){
        isEmpty = true;
        return false;
    }
    if (byteMsg.size()==BYTE_LENGTH && byteMsg.at(0) == 0x5A && byteMsg.at(1) == 0x55){
        unsigned long value = ((byteMsg.at(2)&0xFF) << 24) | ((byteMsg.at(3)&0xFF) << 16) | ((byteMsg.at(4)&0xFF) << 8) | (byteMsg.at(5)&0xFF);
        timeCounter = value;
        int adc_idx;
        for(int i=0;i<13;i++){
            adc_idx = i*2+6;
            adc_vol[i] = qbyte2int(byteMsg.at(adc_idx), byteMsg.at(adc_idx+1));
            if(i==0){
                vcc = adc_vol[i]/1000.0 * 2.0;
            }
            else if(i<9){
                fsr[i-1] = fsrVol2F(adc_vol[i], vcc);
            }
            else{
                ntc[i-9] = ntcVol2T(adc_vol[i], vcc);
            }
        }

        QByteArray imuArr;
        int imu_idx;
        for (int i=0;i<9;i++) {
            imu_idx = i*8+32; // 注意此处
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
    const byte* src_data = reinterpret_cast<const byte *>(src.data());
    memcpy(arr, src_data, src.size());
    return *((double *)arr);
}

double MsgData::fsrVol2F(int vol, double vcc_real) {
    // vol: mV
    double x;
    if(vcc_real>0){
        x = (double)vol/1000.0 * 5.0/vcc_real;
    } else{
        x = (double)vol/1000.0;
    }
    return 14.298*x*x + 10.058*x;
}

double MsgData::ntcVol2T(int vol, double vcc_real) {
    if (vcc_real>0){
        return (3795.9 - vol*5/vcc_real)/51;
    }
    return (3795.9 - vol*5/4.98)/51 ;
}

QByteArray MsgData::imuArr() {
    return byteMsg.mid(40, 72);
}

//QQueue<MsgData> Msg_queue;
//QMutex mutexMsg;