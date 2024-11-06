//
// Created by 13372 on 2024/3/20.
//

#ifndef QT_PROJ_MAINWINDOW_H
#define QT_PROJ_MAINWINDOW_H

#include <QWidget>
#include <QList>
#include <QDebug>
#include <QTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRecord>

#include "UdpSocket.h"
#include "RecvMsgThread.h"
#include "MsgData.h"
#include "FSRDisplay.h"
#include "CaliGraph.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MyServer;
class SocketInformation;

class MainWindow : public QWidget {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void AddClientComboBox(const QString& s, qsizetype idx);
    void removeClientComboBox(int num);

private slots:
    void on_tcpBtn_clicked();
    void on_recordBtn_clicked();
    void on_addNewDoc_clicked();
    void on_editCurrentDoc_clicked();
    void on_reset_clicked();
    void on_databaseAdd_clicked();
    void on_databaseDelete_clicked();
    void on_databaseConfirm_clicked();
    void on_databaseCancel_clicked();
    void on_databaseFind_clicked();
    void on_caliBtn_clicked();
    void on_CaliRecord_clicked();
    void on_AccCaliOn_clicked();
    void on_GyroCaliAutoOn_clicked();
    void on_GyroCaliAutoOff_clicked();
    void on_MagCaliStart_clicked();
    void on_MagCaliStop_clicked();
    void on_MagCaliSave_clicked();
    void on_setAngleRef_clicked();

    void updateTimeAndDisplay();
    void fsrBtnClicked();
    void setAsBtnClicked();

private:
    Ui::MainWindow *ui;

    QSqlDatabase database;

    QTimer *pTimer;
    QTime baseTime;
    QString showStr;
    QList<QPushButton*> fsrBtnList;

    FSRDisplay *leftFoot;
    FSRDisplay *rightFoot;

    CaliGraph *caliGraph;

    MyServer *m_server;
    UdpThread *udpThread;

    QSqlTableModel *model;

    bool tcpConnected;
    bool recording;

    void databaseInit();
    void databaseInsert(const QString& name,
                        const QString& date="",
                        const QString& age="",
                        const QString& height="",
                        const QString& weight="",
                        const QString& info="",
                        const QString& csv_path="");
    void fsrBtnInit();
    void setAsBtnInit();
    void displayInit();
    static QString setSaveDir(QString& name, QString& trialType);
public slots:
    void on_addServerMessage(const QString& message);
};


#endif //QT_PROJ_MAINWINDOW_H
