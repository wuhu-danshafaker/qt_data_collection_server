//
// Created by 13372 on 2025/2/21.
//

#ifndef QT_PROJ_CALIWINDOW_H
#define QT_PROJ_CALIWINDOW_H

#include <QWidget>
#include "CaliData.h"
#include "MySocket.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CaliWindow; }
QT_END_NAMESPACE

class CaliWindow : public QWidget {
Q_OBJECT

public:
    explicit CaliWindow(QWidget *parent = nullptr);

    ~CaliWindow() override;


    void setSocket(MySocket* l, MySocket* r);
    void setCaliEnabled();
    void setWeight(double w);
    void setWeightLine(QLineEdit* pW);
    void setSize(QComboBox* pS);

private:
    QLineEdit* pWeight;
    QComboBox* pSize;
    Ui::CaliWindow *ui;
    CaliData* leftCD;
    CaliData* rightCD;

    MySocket* leftSocket;
    MySocket* rightSocket;

    bool leftOK = false;
    bool rightOK = false;

    bool isDs = false;
    bool isLs = false;
    bool isRs = false;
    bool isTemp = false;

    double leftDataDS = 0;
    double rightDataDS = 0;
    double leftDataLS = 0;
    double rightDataLS = 0;
    double leftDataRS = 0;
    double rightDataRS = 0;

    double weightG = 0;

    QVector<double> leftTemp;
    QVector<double> rightTemp;

public slots:
    void leftCollected(double meanVal);
    void rightCollected(double meanVal);
    void leftTempCollected(const std::vector<double>& tempMean);
    void rightTempCollected(const std::vector<double>& tempMean);
    void on_dSupportBtn_clicked();
    void on_lSupportBtn_clicked();
    void on_rSupportBtn_clicked();
    void on_caliResBtn_clicked();
    void on_applyCaliBtn_clicked();
    void on_tempBtn_clicked();
    void on_caliTempBtn_clicked();
    void on_applyTempBtn_clicked();
};


#endif //QT_PROJ_CALIWINDOW_H
