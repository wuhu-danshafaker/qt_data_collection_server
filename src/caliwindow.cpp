//
// Created by 13372 on 2025/2/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_CaliWindow.h" resolved

#include "caliwindow.h"
#include "ui_CaliWindow.h"

CaliWindow::CaliWindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::CaliWindow), leftTemp(4,0), rightTemp(4,0) {
    ui->setupUi(this);
    this->setWindowTitle("Oiseau Monitor - FSR Calibration");
    leftCD = new CaliData(true);
    rightCD = new CaliData(false);
    leftCD->isInTempCali = false;
    rightCD->isInTempCali = false;
    connect(leftCD, &CaliData::caliRes, this, &CaliWindow::leftCollected);
    connect(rightCD, &CaliData::caliRes, this, &CaliWindow::rightCollected);

    connect(leftCD, &CaliData::caliTemp, this, &CaliWindow::leftTempCollected);
    connect(rightCD, &CaliData::caliTemp, this, &CaliWindow::rightTempCollected);
    leftSocket = nullptr;
    rightSocket = nullptr;
}

CaliWindow::~CaliWindow() {
    delete leftCD;
    delete rightCD;
    delete ui;
}

void CaliWindow::on_dSupportBtn_clicked() {
    if(!leftSocket || !rightSocket){
        qDebug() << "Socket NULL";
        return;
    }
    ui->lSupportBtn->setEnabled(false);
    ui->rSupportBtn->setEnabled(false);
    ui->tempBtn->setEnabled(false);

    int size = ui->timeSpinBox->value() * 100;
    leftCD->clearData();
    rightCD->clearData();
    leftCD->isInTempCali = false;
    rightCD->isInTempCali = false;
    if(leftCD->getSize()!=size){
        leftCD->resize(size);
    }
    if(rightCD->getSize()!=size){
        rightCD->resize(size);
    }

    RecvMsgThread *rmtL = leftSocket->getRMT();
    RecvMsgThread *rmtR = rightSocket->getRMT();
    connect(rmtL, &RecvMsgThread::resultReady, this->leftCD, &CaliData::getCaliData);
    connect(rmtR, &RecvMsgThread::resultReady, this->rightCD, &CaliData::getCaliData);

    isDs = true;
    isLs = false;
    isRs = false;
    isTemp = false;
    leftOK = false;
    rightOK = false;
    ui->dSupportLine->setText("正在记录……");
}

void CaliWindow::on_lSupportBtn_clicked() {
    if(!leftSocket || !rightSocket){
        qDebug() << "Socket NULL";
        return;
    }
    ui->dSupportBtn->setEnabled(false);
    ui->rSupportBtn->setEnabled(false);
    ui->tempBtn->setEnabled(false);
    ui->lSupportMsg->clear();
    int size = ui->timeSpinBox->value() * 100;
    leftCD->clearData();
    rightCD->clearData();
    leftCD->isInTempCali = false;
    rightCD->isInTempCali = false;
    if(leftCD->getSize()!=size){
        leftCD->resize(size);
    }
    if(rightCD->getSize()!=size){
        rightCD->resize(size);
    }

    RecvMsgThread *rmtL = leftSocket->getRMT();
    RecvMsgThread *rmtR = rightSocket->getRMT();
    connect(rmtL, &RecvMsgThread::resultReady, this->leftCD, &CaliData::getCaliData);
    connect(rmtR, &RecvMsgThread::resultReady, this->rightCD, &CaliData::getCaliData);

    isDs = false;
    isLs = true;
    isRs = false;
    isTemp = false;
    leftOK = false;
    rightOK = false;
    ui->lSupportLine->setText("正在记录……");
}

void CaliWindow::on_rSupportBtn_clicked() {
    if(!leftSocket || !rightSocket){
        qDebug() << "Socket NULL";
        return;
    }
    ui->dSupportBtn->setEnabled(false);
    ui->lSupportBtn->setEnabled(false);
    ui->tempBtn->setEnabled(false);
    ui->rSupportMsg->clear();
    int size = ui->timeSpinBox->value() * 100;
    leftCD->clearData();
    rightCD->clearData();
    leftCD->isInTempCali = false;
    rightCD->isInTempCali = false;
    if(leftCD->getSize()!=size){
        leftCD->resize(size);
    }
    if(rightCD->getSize()!=size){
        rightCD->resize(size);
    }

    RecvMsgThread *rmtL = leftSocket->getRMT();
    RecvMsgThread *rmtR = rightSocket->getRMT();
    connect(rmtL, &RecvMsgThread::resultReady, this->leftCD, &CaliData::getCaliData);
    connect(rmtR, &RecvMsgThread::resultReady, this->rightCD, &CaliData::getCaliData);

    isDs = false;
    isLs = false;
    isRs = true;
    isTemp = false;
    leftOK = false;
    rightOK = false;
    ui->rSupportLine->setText("正在记录……");
}

void CaliWindow::setSocket(MySocket *l, MySocket *r) {
    leftSocket = l;
    rightSocket = r;
}

void CaliWindow::leftCollected(double meanVal) {
    if(leftSocket == nullptr){
        return;
    }
    RecvMsgThread *rmtL = leftSocket->getRMT();
    disconnect(rmtL, &RecvMsgThread::resultReady, this->leftCD, &CaliData::getCaliData);
    leftOK = true;
    if(isDs){
        leftDataDS = meanVal;
        if(rightOK){
            // DS finished
            ui->dSupportLine->setText(QString("Left: %1, Right: %2").arg(leftDataDS).arg(rightDataDS));
            ui->lSupportBtn->setEnabled(true);
            ui->rSupportBtn->setEnabled(true);
            ui->tempBtn->setEnabled(true);
        }
    }else if(isLs){
        leftDataLS = meanVal;
        if(rightOK){
            ui->lSupportLine->setText(QString("Left: %1, Right: %2").arg(leftDataLS).arg(rightDataLS));
            ui->dSupportBtn->setEnabled(true);
            ui->rSupportBtn->setEnabled(true);
            ui->tempBtn->setEnabled(true);
        }
    }else if(isRs){
        leftDataRS = meanVal;
        if(rightOK){
            if(meanVal!=0){
                ui->rSupportMsg->setText("左足未置零!");
            }
            ui->rSupportLine->setText(QString("Left: %1, Right: %2").arg(leftDataRS).arg(rightDataRS));
            ui->dSupportBtn->setEnabled(true);
            ui->lSupportBtn->setEnabled(true);
            ui->tempBtn->setEnabled(true);
        }
    }
}

void CaliWindow::rightCollected(double meanVal) {
    if(rightSocket == nullptr){
        return;
    }
    RecvMsgThread *rmtR = rightSocket->getRMT();
    disconnect(rmtR, &RecvMsgThread::resultReady, this->rightCD, &CaliData::getCaliData);
    rightOK = true;
    if(isDs){
        rightDataDS = meanVal;
        if(leftOK){
            ui->dSupportLine->setText(QString("Left: %1, Right: %2").arg(leftDataDS).arg(rightDataDS));
            ui->lSupportBtn->setEnabled(true);
            ui->rSupportBtn->setEnabled(true);
            ui->tempBtn->setEnabled(true);
        }
    }else if(isRs){
        rightDataRS = meanVal;
        if(leftOK){
            ui->rSupportLine->setText(QString("Left: %1, Right: %2").arg(leftDataRS).arg(rightDataRS));
            ui->dSupportBtn->setEnabled(true);
            ui->lSupportBtn->setEnabled(true);
            ui->tempBtn->setEnabled(true);
        }
    }else if(isLs){
        rightDataLS = meanVal;
        if(leftOK){
            if(meanVal!=0){
                ui->lSupportMsg->setText("右足未置零!");
            }
            ui->lSupportLine->setText(QString("Left: %1, Right: %2").arg(leftDataLS).arg(rightDataLS));
            ui->dSupportBtn->setEnabled(true);
            ui->rSupportBtn->setEnabled(true);
            ui->tempBtn->setEnabled(true);
        }
    }
}

void CaliWindow::setCaliEnabled() {
    ui->dSupportBtn->setEnabled(true);
    ui->lSupportBtn->setEnabled(true);
    ui->rSupportBtn->setEnabled(true);
}

void CaliWindow::on_caliResBtn_clicked() {
    if (pWeight->text().isEmpty()){
        QMessageBox::critical(this, "Failed", "体重未输入");
    } else{
        double w = pWeight->text().toDouble();
        setWeight(w);
    }
    if(!leftDataLS || !leftDataDS || !rightDataRS || !rightDataDS || !weightG){
        qDebug() << "invalid cali args";
//        return;
        leftDataLS = 200;
        leftDataDS = 100;
        rightDataRS = 200;
        rightDataDS = 100;
        weightG = 50;
    }

    auto *process = new QProcess(this);

    // 设置要执行的命令和参数
    QString program = "../scripts/analysis/analysis.exe"; // 替换为你的 EXE 路径
    QStringList arguments;
    arguments << "--todo" << "cali";
    arguments << "--caliArg" << QString("%1,%2,%3,%4,%5").arg(leftDataLS).arg(leftDataDS).arg(rightDataRS).arg(rightDataDS).arg
    (weightG);
    arguments << "--size" << pSize->currentText();
//    arguments << "--caliArg" << QString("%1,%2,%3,%4,%5").arg(500).arg(250).arg(480).arg(260).arg(600);
    process->start(program, arguments);
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
        QByteArray output = process->readAllStandardOutput();
        auto res = QString::fromLocal8Bit(output);
        qDebug() << res;
        if(res.contains("Calibrated:")){
            QStringList parts = res.split(" ");
            // 提取 kl 和 kr
            if (parts.size() >= 5) {
                double kl = parts[3].toDouble(); // KL 后的值
                double kr = parts[5].toDouble(); // KR 后的值
                ui->caliResLine->setText(QString("Kl: %1, Kr: %2").arg(kl).arg(kr));
            } else {
                qDebug() << "Invalid input format!";
            }
        }

    });

    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        QByteArray errorOutput = process->readAllStandardError();
        qDebug() << QString::fromLocal8Bit(errorOutput);
    });
    // 可选：处理进程结束信号
    connect(process, &QProcess::finished, this, [process](int exitCode, QProcess::ExitStatus exitStatus){
        if (exitStatus == QProcess::NormalExit) {
            qDebug() << "Process finished with exit code:" << exitCode;
        } else {
            qDebug() << "Process crashed";
        }
        process->deleteLater(); // 清理进程对象
    });
}

void CaliWindow::setWeight(double w) {
    weightG = w * 9.8;
}

void CaliWindow::on_applyCaliBtn_clicked() {
    if(!leftSocket || !rightSocket){
        qDebug() << "Socket NULL";
        return;
    }
    bool isInt;
    QString nowS = pSize->currentText();
    int size = nowS.toInt(&isInt);
    QString offSize = "";
    if (isInt){
        size = size + 1 - size%2;
        offSize = QString("%1").arg(size);
    }
    leftSocket->setFsrFactor(offSize);
    rightSocket->setFsrFactor(offSize);
}

void CaliWindow::on_tempBtn_clicked() {
    if(!leftSocket || !rightSocket){
        qDebug() << "Socket NULL";

        isDs = false;
        isLs = false;
        isRs = false;
        isTemp = true;
        leftOK = false;
        rightOK = false;
        ui->tempLine->setText("正在记录……");
        return;
    }
    ui->dSupportBtn->setEnabled(false);
    ui->lSupportBtn->setEnabled(false);
    ui->rSupportBtn->setEnabled(false);
    int size = ui->timeSpinBox->value() * 100;
    leftCD->clearData();
    rightCD->clearData();
    leftCD->isInTempCali = true;
    rightCD->isInTempCali = true;
    if(leftCD->getSize()!=size){
        leftCD->resize(size);
    }
    if(rightCD->getSize()!=size){
        rightCD->resize(size);
    }

    RecvMsgThread *rmtL = leftSocket->getRMT();
    RecvMsgThread *rmtR = rightSocket->getRMT();
    connect(rmtL, &RecvMsgThread::resultReady, this->leftCD, &CaliData::getCaliData);
    connect(rmtR, &RecvMsgThread::resultReady, this->rightCD, &CaliData::getCaliData);

    isDs = false;
    isLs = false;
    isRs = false;
    isTemp = true;
    leftOK = false;
    rightOK = false;
    ui->tempLine->setText("正在记录……");
}

void CaliWindow::leftTempCollected(const std::vector<double>& tempMean) {
    if(leftSocket == nullptr){
        return;
    }
    RecvMsgThread *rmtL = leftSocket->getRMT();
    disconnect(rmtL, &RecvMsgThread::resultReady, this->leftCD, &CaliData::getCaliData);
    leftOK = true;
    if(isTemp){
        for (int i=0;i<4;i++){
            leftTemp[i] = tempMean[i];
        }
        if(rightOK){
            // DS finished
            ui->dSupportLine->setEnabled(true);
            ui->lSupportBtn->setEnabled(true);
            ui->rSupportBtn->setEnabled(true);
            ui->tempLine->setText("记录完成");
        }
    }
}

void CaliWindow::rightTempCollected(const std::vector<double>& tempMean) {
    if(rightSocket == nullptr){
        return;
    }
    RecvMsgThread *rmtR = leftSocket->getRMT();
    disconnect(rmtR, &RecvMsgThread::resultReady, this->rightCD, &CaliData::getCaliData);
    rightOK = true;
    if(isTemp){
        for (int i=0;i<4;i++){
            rightTemp[i] = tempMean[i];
        }
        if(leftOK){
            // DS finished
            ui->dSupportLine->setEnabled(true);
            ui->lSupportBtn->setEnabled(true);
            ui->rSupportBtn->setEnabled(true);
            ui->tempLine->setText("记录完成");
        }
    }
}

void CaliWindow::on_caliTempBtn_clicked() {
    if (leftTemp.contains(0) || rightTemp.contains(0)){
        qDebug() << "invalid temp args";
        return;
    }

    QVector<double> leftOff(4, 0);
    QVector<double> rightOff(4, 0);
    bool isValidT;
    double t = ui->tempLine->text().toDouble(&isValidT);
    double meanTemp = 0;
    if (isValidT){
        meanTemp = t;
    } else{
        for (auto& temp: leftTemp){
            meanTemp += temp;
        }
        for (auto& temp: rightTemp){
            meanTemp += temp;
        }
        meanTemp = meanTemp / 8;
    }

    QJsonObject rootObject;
    QJsonObject leftObject;
    QJsonObject rightObject;
    for (int i=0;i<4;i++){
        leftOff[i] = meanTemp - leftTemp[i];
        rightOff[i] = meanTemp - rightTemp[i];
        leftObject[QString("ntc%1").arg(i+1)] = leftOff[i];
        rightObject[QString("ntc%1").arg(i+1)] = rightOff[i];
    }
    rootObject.insert("Left NTC Offset", leftObject);
    rootObject.insert("Right NTC Offset", rightObject);
    QJsonDocument jsonDoc(rootObject);
    QString jsonString = jsonDoc.toJson(QJsonDocument::Indented);

    bool isInt;
    QString nowS = pSize->currentText();
    int size = nowS.toInt(&isInt);
    QString offSize = "";
    if (isInt){
        size = size + 1 - size%2;
        offSize = QString("%1").arg(size);
    }

    QString filePath = QString("../scripts/tempOffset%1.json").arg(offSize);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << jsonString;
        file.close();
        qDebug() << "JSON file written successfully to" << filePath;
    } else {
        qDebug() << "Failed to open file for writing.";
    }
}


void CaliWindow::on_applyTempBtn_clicked() {
    if(!leftSocket || !rightSocket){
        qDebug() << "Socket NULL";
        return;
    }
    bool isInt;
    QString nowS = pSize->currentText();
    int size = nowS.toInt(&isInt);
    QString offSize = "";
    if (isInt){
        size = size + 1 - size%2;
        offSize = QString("%1").arg(size);
    }
    leftSocket->setTempOffset(offSize);
    rightSocket->setTempOffset(offSize);
}

void CaliWindow::setWeightLine(QLineEdit* pW) {
    pWeight = pW;
}

void CaliWindow::setSize(QComboBox *pS) {
    pSize = pS;
}





