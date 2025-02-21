#include "MsgData.h"
#include <QDebug>
#include <utility>
//#include <QtMath>

MsgData::MsgData() {
    isEmpty = true;
    vcc = 0;
}

MsgData::MsgData(bool side, QByteArray& msg) {
    isLeft = side;
    byteInput(msg);

    isEmpty = true;
    vcc = 0;
}

MsgData::MsgData(bool side, QByteArray &msg, const QVector<double> &left, const QVector<double> &right) {
    isLeft = side;
    byteInput(msg);

    isEmpty = true;
    vcc = 0;
    leftFsrFactor = left;
    rightFsrFactor = right;
}

void MsgData::byteInput(QByteArray& msg) {
    byteMsg = msg;
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
                int idx;
                double factor;
                if(isLeft){
                    idx = fsrMapL[i-1];
                    factor = leftFsrFactor[i-1];
                } else{
                    idx = fsrMapR[i-1];
                    factor = rightFsrFactor[i-1];
                }
//                fsr[idx] = fsrVol2F(adc_vol[i], vcc, fsrFactor[i-1]);
                fsr[idx] = fsrVol2F(adc_vol[i], vcc, factor);  // 效果如何？
            }
            else{
                int idx;
                if(isLeft){
                    idx = ntcMapL[i-9];
                } else{
                    idx = ntcMapR[i-9];
                }
                ntc[idx] = ntcVol2T(adc_vol[i], vcc);
            }
        }

        QByteArray imuArr;
        int imu_idx;
        for (int i=0;i<9;i++) {
            imu_idx = i*8+32; // 注意此处
            imuArr = byteMsg.mid(imu_idx, 8);
            imuAGE[i] = qbyte2double(imuArr);
        }

        QByteArray magArr;
        int mag_idx;
        for(int i=0;i<3;i++){
            mag_idx = i*8 + 6 + 26 + 72;
            magArr = byteMsg.mid(mag_idx, 8);
            mag[i] = qbyte2double(magArr);
        }
        ip_esp32s3 = ipByte2Str(byteMsg.right(4));
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

double MsgData::fsrVol2F(double vol, double vcc_real, double factor) {
    return vol/vcc_real*factor;
}

double MsgData::ntcVol2T(int vol, double vcc_real) {
    if (vcc_real>0){
        return (3795.9 - vol*5/vcc_real)/51;
    }
    return (3795.9 - vol*5/4.98)/51 ;
}

QString MsgData::ipByte2Str(const QByteArray& src) {
    QByteArray src_data = src;
    if(src_data.isEmpty()){
        src_data = byteMsg.right(4);
    }
    int ip1 = static_cast<byte>(src_data.at(0));
    int ip2 = static_cast<byte>(src_data.at(1));
    int ip3 = static_cast<byte>(src_data.at(2));
    int ip4 = static_cast<byte>(src_data.at(3));
    return QString("%1.%2.%3.%4").arg(ip1).arg(ip2).arg(ip3).arg(ip4);
}



