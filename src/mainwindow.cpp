//
// Created by 13372 on 2024/3/20.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved
#include "mainwindow.h"
#include "MyServer.h"
#include "MySocket.h"
#include "ui_MainWindow.h"
//#include <QGraphicsBlurEffect>

MainWindow::MainWindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->setWindowIcon(QIcon("../resources/qt_icon.png"));
    this->setWindowTitle("Oiseau Monitor");
    this->setWindowFlags(Qt::Dialog | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    this->setAttribute(Qt::WA_DeleteOnClose);
    m_server = nullptr;

    databaseInit();
    fsrBtnInit();
    setAsBtnInit();

    rightFoot = new FSRDisplay();
    leftFoot = new FSRDisplay();
    displayInit();

    caliGraph = new CaliGraph(ui->XZ, ui->YZ, ui->XY, ui->Acc, ui->Gyro, ui->Euler);

    pTimer = nullptr;

    model = new QSqlTableModel(this);
    model->setTable("patient");
    ui->databaseView->setModel(model);
    model->select();
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    tcpConnected = false;
    recording = false;
    qDebug() << "mainThread:" << QThread::currentThreadId();

    ui->Body->setContentsMargins(0,0,0,0);
    ui->CaliRecord->setProperty("isOn", false);
    ui->MagCaliSave->setEnabled(false);
    ui->MagCaliStop->setEnabled(false);
//    auto *blurEffect = new QGraphicsBlurEffect;
//    ui->Body->setGraphicsEffect(blurEffect);
}

MainWindow::~MainWindow() {
    delete ui;
    delete pTimer;
    delete m_server;
    delete rightFoot;
    delete leftFoot;
    udpThread->quit();
    udpThread->wait();
    delete udpThread;
}

void MainWindow::on_tcpBtn_clicked() {
    if(this->m_server== nullptr){
        m_server = new MyServer(this);
        m_server->SetThread(2);
        if(!m_server->listen(QHostAddress::Any, 8080)){
            QMessageBox::critical(this, "Failed", "Server Start Failed");
        }

        ui->tcpMsg->append("开始监听……");
        ui->tcpBtn->setText("Disconnect");
        tcpConnected = true;
        udpThread = new UdpThread(m_server);
        udpThread->start();

    } else {
        // 解绑socket
        leftFoot->clearSocket();
        rightFoot->clearSocket();
        ui->LeftIs->clear();
        ui->RightIs->clear();
        // 清空combo box
        while (ui->clientComboBox->count()>0) {
            ui->clientComboBox->removeItem(0);
        }

        // 不清空socket连接,直接进行删除，因此removeInfo不会运行，需要手动clear clientComboBox
        udpThread->quit();
        udpThread->wait();
        delete udpThread;
        udpThread = nullptr;
        m_server->close();
        delete m_server;
        m_server = nullptr;

        ui->tcpMsg->append("停止监听……");
        ui->tcpBtn->setText("Connect");

        if(pTimer){
            pTimer->stop();
        }

        tcpConnected = false;
    }
}

QString MainWindow::setSaveDir(QString& name, QString& trialType) {
    QDir dir;
    QString targetDir;
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString dateStr = currentDateTime.toString("MMdd");
    QString csvDir = QString("../csvData/%1/%2").arg(name, dateStr);
    if(trialType=="Test"){
        targetDir = QString("%1/%2").arg(csvDir, trialType);
        if(!dir.exists(targetDir)){
            dir.mkpath(targetDir);
        }
    } else{
        int trialTimes = 1;
        targetDir = QString("%1/%2_%3").arg(csvDir, trialType, QString::number(trialTimes));
        while(dir.exists(targetDir)){
            trialTimes++;
            targetDir = QString("%1/%2_%3").arg(csvDir, trialType, QString::number(trialTimes));
        }
        dir.mkpath(targetDir);
    }
    return targetDir;
}

void MainWindow::on_reset_clicked() {
    leftFoot->resetPlot();
    rightFoot->resetPlot();
}

void MainWindow::on_recordBtn_clicked() {
    QString name = ui->pName->text();
    if(name.isEmpty()){
        // name is a must
        QMessageBox::critical(this, "Failed", "The name must be specified.");
        return;
    }

    if(leftFoot->getIdx()<0&&rightFoot->getIdx()<0){
        QMessageBox::critical(this, "Failed", "None socket was banded with plots");
        return;
    }

    bool isResume = true;
    // 初次按下
    if(!pTimer){
        pTimer = new QTimer;
        baseTime = QTime::currentTime();
        connect(pTimer, &QTimer::timeout, this, &MainWindow::updateTimeAndDisplay);
        isResume = false;
    }

    if(!recording){
        qDebug()<<"restart recording";
        leftFoot->resetPlot();
        rightFoot->resetPlot();
        baseTime = QTime::currentTime();
        updateTimeAndDisplay();
        pTimer->start(800);
        QString trialType = ui->trialComboBox->currentText();
        QString saveDir = setSaveDir(name, trialType);
        leftFoot->startDisplay(name, saveDir, isResume);
        rightFoot->startDisplay(name, saveDir, isResume);
        recording = true;
        on_addServerMessage("pTimer: recording");
    } else{
        pTimer->stop();
        recording = false;
        leftFoot->pauseDisplay();
        rightFoot->pauseDisplay();
    }
}

void MainWindow::updateTimeAndDisplay() {
    QTime current = QTime::currentTime();
    int t = this->baseTime.msecsTo(current);
    QTime showtime(0,0,0,0);
    showtime = showtime.addMSecs(t);
    showStr = showtime.toString("mm:ss");
    ui->timeCounter->setText(showStr);
}

void MainWindow::on_addServerMessage(const QString& message) {
    ui->tcpMsg->append(message);
    ui->tcpMsg->moveCursor(QTextCursor::End);
}

void MainWindow::AddClientComboBox(const QString& s, qsizetype idx) {
    QVariant var;
    var.setValue(idx);
    ui->clientComboBox->addItem(s, var);
}

void MainWindow::removeClientComboBox(int num) {
    QString ipStr = ui->clientComboBox->itemText(num);
    if(ui->LeftIs->text() == ipStr){
        ui->LeftIs->clear();
    }
    if(ui->RightIs->text() == ipStr){
        ui->RightIs->clear();
    }
    ui->clientComboBox->removeItem(num);
}

void MainWindow::fsrBtnInit() {
    // fsr btn 左右各8个
    QList<QString> fsrTypes = {"left", "right"};
    for(auto &fsrType: fsrTypes){
        for(int i=1;i<9;i++){
            QString btn = QString("%1Fsr_%2").arg(fsrType).arg(i);
            auto *fsrBtn = findChild<QPushButton*>(btn);

            Q_ASSERT(fsrBtn);
            fsrBtn->setProperty("type", fsrType);
            fsrBtn->setProperty("fsrNum", i);
            connect(fsrBtn, SIGNAL(clicked()), this, SLOT(fsrBtnClicked()));
        }
    }
}

void MainWindow::fsrBtnClicked() {
    auto *fsrBtn = qobject_cast<QPushButton*>(sender());
    if(fsrBtn){
        QString fsrType = fsrBtn->property("type").toString();
        int fsrNum = fsrBtn->property("fsrNum").toInt();
        FSRDisplay *fsrDisplay = nullptr;
        if(fsrType=="left"){
            fsrDisplay = leftFoot;
        } else if(fsrType=="right"){
            fsrDisplay = rightFoot;
        } else {
            qDebug() << "Unknown Type of Btn was triggered";
        }

        // fsrNum 1~8, 而showFsr数组是0~7
        if(fsrDisplay&&fsrNum){
            fsrDisplay->showFsr(fsrNum-1);
        }
    }
}

void MainWindow::setAsBtnInit() {
    QList<QString> fsrTypes = {"Left", "Right"};
    for(auto &fsrType: fsrTypes){
        QString btn = QString("setAs%1").arg(fsrType);
        auto *fsrBtn = findChild<QPushButton*>(btn);
        Q_ASSERT(fsrBtn);
        fsrBtn->setProperty("type", fsrType);
        connect(fsrBtn, SIGNAL(clicked()), this, SLOT(setAsBtnClicked()));
    }
}

void MainWindow::setAsBtnClicked() {
    auto *btn = qobject_cast<QPushButton*>(sender());
    QComboBox *comboBox = ui->clientComboBox;
    QString espIp = comboBox->currentText();
    QVariant var = comboBox->currentData();
    if(!espIp.isEmpty()){
        QString str = QString("%1%2").arg(btn->property("type").toString(), "Is");
        auto idx = var.value<qsizetype>();
        if(str=="LeftIs"){
            auto *espIs = ui->LeftIs;
            espIs->setText(espIp);
            Q_ASSERT(espIs);
            leftFoot->setIdx(idx);
            leftFoot->setSocket(m_server->list_information[idx].getSocket());
        }
        if(str=="RightIs"){
            auto *espIs = ui->RightIs;
            espIs->setText(espIp);
            Q_ASSERT(espIs);
            rightFoot->setIdx(idx);
            rightFoot->setSocket(m_server->list_information[idx].getSocket());
        }
    }
}

void MainWindow::displayInit() {
    leftFoot->setIsLeft(true);
    leftFoot->setPlotUI(ui->FootPrint_Left);
    leftFoot->setDynamicUI(ui->fsrData_Left);
    leftFoot->setImuUI(ui->imuAGE_Left);
    leftFoot->setTempUI(ui->temp_Left);
    leftFoot->setupPlot();

    rightFoot->setIsLeft(false);
    rightFoot->setPlotUI(ui->FootPrint_Right);
    rightFoot->setDynamicUI(ui->fsrData_Right);
    rightFoot->setImuUI(ui->imuAGE_Right);
    rightFoot->setTempUI(ui->temp_Right);
    rightFoot->setupPlot();
}

void MainWindow::databaseInit() {
    if(QSqlDatabase::contains("qt_sql_default_connection")){
        database = QSqlDatabase::database("qt_sql_default_connection");
    } else{
        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName("../database/myDatabase.db");
        database.setUserName("oiseau");
        database.setPassword("687207");
        qDebug() << "database connected.";
    }
    database.close();
}

void MainWindow::databaseInsert(const QString& name,
                                const QString& date,
                                const QString& age,
                                const QString& height,
                                const QString& weight,
                                const QString& info,
                                const QString& csv_path) {
    if (!database.open()){
        qDebug() << "Error: Failed to connect database." << database.lastError();
    } else{
        QSqlQuery sqlQuery;
//        QString insert_sql = QString("insert into patient "
//                                     "(id, name, date_time, age, height, weight, information, csv_path) "
//                                     "values (null, '%1', '%2', %3, %4, %5, '%6', '%7')")
//                .arg(name, date, age, height, weight, info, csv_path);
//        sqlQuery.prepare(insert_sql);
        QString insert_sql = QString("insert into patient "
                                     "(id, name, date_time, age, height, weight, information, csv_path) "
                                     "values (:id, :name, :date_time, :age, :height, :weight, :information, "
                                     ":csv_path)");
        sqlQuery.prepare(insert_sql);
        sqlQuery.bindValue(":name", name);
        sqlQuery.bindValue(":date_time", date);
        sqlQuery.bindValue(":age", age);
        sqlQuery.bindValue(":height", height);
        sqlQuery.bindValue(":weight", weight);
        sqlQuery.bindValue(":information", info);
        sqlQuery.bindValue(":csv_path", csv_path);

        if(!sqlQuery.exec()){
            qDebug() << sqlQuery.lastError();
        } else{
            qDebug() << "inserted!";
        }
    }
    database.close();
    database.open();
}

void MainWindow::on_addNewDoc_clicked() {
    auto *patientDoc = ui->patientDoc;
    auto *pName = patientDoc->findChild<QLineEdit *>("pName");
    auto *pAge = patientDoc->findChild<QLineEdit *>("pAge");
    auto *pHeight = patientDoc->findChild<QLineEdit *>("pHeight");
    auto *pWeight = patientDoc->findChild<QLineEdit *>("pWeight");
    auto *pInfo = patientDoc->findChild<QTextEdit *>("pInfo");
    auto *pCsvPath = patientDoc->findChild<QLineEdit *>("pCsvPath");
    Q_ASSERT(pName);

    QString name = pName->text();
    if(name.isEmpty()){
        // name is a must
        QMessageBox::critical(this, "Failed", "The name must be specified.");
        return;
    }

    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString date_time = currentDateTime.toString("yy-MM-dd");

    QString age = pAge->text();
    QString height = pHeight->text();
    QString weight = pWeight->text();
    QString info = pInfo->toPlainText();

    QString csvPath = pCsvPath->text();

    if(csvPath.isEmpty()){
        QDir dir(QDir::currentPath());
        dir.cdUp();
        QString path = dir.path();
//        size_t pos = path.lastIndexOf('/');
        csvPath = path + "/csvData/" + name;
        pCsvPath->setText(csvPath);
    } else{
        QDir dir(csvPath);
        if(dir.exists()){
            QMessageBox::critical(this, "Failed", "Path not exists.");
            return;
        }
    }
    databaseInsert(name, date_time, age, height, weight, info, csvPath);
}

void MainWindow::on_editCurrentDoc_clicked() {
    ui->mainPage->setCurrentIndex(1);
}

void MainWindow::on_databaseAdd_clicked() {
    QSqlRecord record = model->record();
    int row = model->rowCount();
    model->insertRecord(row, record);
}

void MainWindow::on_databaseDelete_clicked() {
    QItemSelectionModel *sModel = ui->databaseView->selectionModel();
    QModelIndexList list = sModel->selectedRows();
    if(!list.isEmpty()){
        for (const auto & i : list) {
            model->removeRow(i.row());
        }
    } else {
        int curRow = ui->databaseView->currentIndex().row();
        model->removeRow(curRow);
    }

}

void MainWindow::on_databaseConfirm_clicked() {
    model->submitAll();
}

void MainWindow::on_databaseCancel_clicked() {
    model->revertAll();
    model->submitAll();
}

void MainWindow::on_databaseFind_clicked() {
    QString keyDate = ui->findByDate->text();
    QString keyName = ui->findByName->text();
    QString filterStr = "";
    QList<QString> filterList;
    if(!keyDate.isEmpty()){
        filterList.append(QString("date_time = '%1'").arg(keyDate));
    }
    if(!keyName.isEmpty()){
        filterList.append(QString("name = '%1'").arg(keyName));
    }
    if(!filterList.isEmpty()){
        filterStr = filterList.at(0);
        for(int i=1;i<filterList.size();i++){
            filterStr = filterStr + " and " + filterList.at(i);
        }
        if(filterList.size()>1){
            filterStr = "(" + filterStr + ")";
        }
        model->setFilter(filterStr);
        model->select();
    } else{
        model->setTable("patient");
        model->select();
    }
    qDebug() << "FIND: " << filterStr;
}

void MainWindow::on_caliBtn_clicked() {
    QComboBox *comboBox = ui->clientComboBox;
    QString espIp = comboBox->currentText();
    QVariant var = comboBox->currentData();
    if(!espIp.isEmpty()) {
        auto idx = var.value<qsizetype>();
        auto *espIs = ui->caliIp;
        espIs->setText(espIp);
        caliGraph->setSocket(m_server->list_information[idx].getSocket());
    }
    ui->mainPage->setCurrentIndex(2);
}

void MainWindow::on_CaliRecord_clicked() {
    if(!caliGraph->status()) return;
    bool isOn = ui->CaliRecord->property("isOn").toBool();
    if(!isOn){
        qDebug()<<"is On";
        caliGraph->resetPlot();
        caliGraph->startRecord();
//        TODO:
//        新连接的无法record的原因就在isResume,该设置是有问题的——他以pTimer为标准。
        ui->CaliRecord->setProperty("isOn", true);
        ui->CaliRecord->setText("Stop Record");
    } else{
        qDebug()<<"is Off";
        caliGraph->pauseRecord();
        ui->CaliRecord->setProperty("isOn", false);
        ui->CaliRecord->setText("Start Record");
    }
}

void MainWindow::on_AccCaliOn_clicked() {
    if(caliGraph->status()){
        caliGraph->startAccCali();
    }
}

void MainWindow::on_GyroCaliAutoOn_clicked() {
    if(caliGraph->status()){
        QByteArray cmd = "CMD: set gyro auto cali on";
        caliGraph->sendCaliCmd(cmd);
    }
}

void MainWindow::on_GyroCaliAutoOff_clicked() {
    if(caliGraph->status()){
        QByteArray cmd = "CMD: set gyro auto cali off";
        caliGraph->sendCaliCmd(cmd);
    }
}

void MainWindow::on_MagCaliStart_clicked() {
    if(caliGraph->status()){
        caliGraph->resetPlot(true);
        QByteArray cmd = "CMD: start magnetometer cali";
        caliGraph->sendCaliCmd(cmd);
        ui->MagCaliStop->setEnabled(true);
        ui->MagCaliStart->setEnabled(false);
    }
}

void MainWindow::on_MagCaliStop_clicked() {
    if(caliGraph->status()){
        QByteArray cmd = "CMD: stop magnetometer cali";
        caliGraph->sendCaliCmd(cmd);
        ui->MagCaliSave->setEnabled(true);
        ui->MagCaliStart->setEnabled(true);
        ui->MagCaliStop->setEnabled(false);
    }
}

void MainWindow::on_MagCaliSave_clicked() {
    if(caliGraph->status()){
        QByteArray cmd = "CMD: save cali result";
        caliGraph->sendCaliCmd(cmd);
        ui->MagCaliStart->setEnabled(true);
    }
}

void MainWindow::on_setAngleRef_clicked() {
    if(caliGraph->status()){
        QByteArray cmd = "CMD: set angle reference";
        caliGraph->sendCaliCmd(cmd);
    }
}




