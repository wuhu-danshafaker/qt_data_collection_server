#ifndef QT_PROJ_MSGDATA_H
#define QT_PROJ_MSGDATA_H
#include <QQueue>
#include <QByteArray>
#include <QMutex>

#define BYTE_LENGTH 104

class MsgData{
public:
    MsgData();
    bool MsgByteExplain();
    void byteInput(QByteArray msg);
    int adc_vol[13];
    double fsr[8];
    double ntc[4];
    double vcc;
//    std::vector<int> adc_vol{std::vector<int>(5)};
    double imuAGE[9];
//    std::vector<double> imuAGE{std::vector<double>(9)};
    unsigned long timeCounter;
    QByteArray imuArr();

private:
    QByteArray byteMsg;
    static int qbyte2int(char high, char low);
    bool isEmpty;
    static double qbyte2double(QByteArray src);
    static double fsrVol2F(int vol, double vcc_real);
    static double ntcVol2T(int vol, double vcc_real);
};

//extern QQueue<MsgData> Msg_queue;
//extern QMutex mutexMsg;

#endif //QT_PROJ_MSGDATA_H
