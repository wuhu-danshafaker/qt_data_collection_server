#ifndef QT_PROJ_MSGDATA_H
#define QT_PROJ_MSGDATA_H
#include <QQueue>
#include <QByteArray>
#include <QMutex>

class MsgData{
public:
    MsgData();
    bool MsgByteExplain();
    void byteInput(QByteArray msg);
    int adc_vol[12];
    double fsr[8];
    double ntc[4];
//    std::vector<int> adc_vol{std::vector<int>(5)};
    double imuAGE[9];
//    std::vector<double> imuAGE{std::vector<double>(9)};
    unsigned long timeCounter;

private:
    QByteArray byteMsg;
    static int qbyte2int(char high, char low);
    bool isEmpty;
    static double qbyte2double(QByteArray src);
    static double fsrVol2F(int vol);
    static double mapF2R_fsr(double f);
    static double k_map(double f);
    static double ntcVol2T(int vol);
};

//extern QQueue<MsgData> Msg_queue;
//extern QMutex mutexMsg;

#endif //QT_PROJ_MSGDATA_H
