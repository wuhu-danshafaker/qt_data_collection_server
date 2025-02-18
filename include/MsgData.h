#ifndef QT_PROJ_MSGDATA_H
#define QT_PROJ_MSGDATA_H
#include <QQueue>
#include <QByteArray>
#include <QMutex>

#define BYTE_LENGTH 132

class MsgData{
public:
    MsgData();
    MsgData(bool side, QByteArray& msg);
    bool MsgByteExplain();
    void byteInput(QByteArray& msg);
    QString ipByte2Str(const QByteArray& src="");
    int adc_vol[13];
    double fsr[8];
    double ntc[4];
    double vcc;
    double imuAGE[9];
    double mag[3];
    unsigned long timeCounter;
    QString ip_esp32s3;

private:
    QByteArray byteMsg;
    static int qbyte2int(char high, char low);
    bool isEmpty;
    bool isLeft;
    static double qbyte2double(QByteArray src);
    static double fsrVol2F(double vol, double vcc_real, double factor);
    static double ntcVol2T(int vol, double vcc_real);
//    int fsrMapL[8] = {3,4,2,0,1,7,5,6};
    int fsrMapL[8] = {4,3,2,0,1,6,7,5};
    int fsrMapR[8] = {1,0,2,3,4,5,7,6};
//    int ntcMapL[4] = {2,1,3,0};
    int ntcMapL[4] = {3,1,0,2};
    int ntcMapR[4] = {3,2,1,0};
    double fsrFactor[8] = {3.55, 4.65, 4.85, 4.6, 4.65, 4.2, 4.55, 3.75};
//    double fsrFactorR[8] = {4.8,4.3,4.6,3.3,4.5,3.6,4.1,4.5};
};

#endif //QT_PROJ_MSGDATA_H
