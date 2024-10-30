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
        // 清空combo box
        if(int numSock = ui->clientComboBox->count()){
            for(;numSock>0;numSock--){
                ui->clientComboBox->removeItem(numSock);
            }
            ui->clientComboBox->setCurrentIndex(0);
        }

        // 不清空socket连接,直接进行删除，因此removeInfo不会运行，需要手动clear clientComboBox
        udpThread->quit();
        udpThread->wait();
        delete udpThread;
        udpThread = nullptr;
        qDebug() << "clear udpThread";
        m_server->close();
        delete m_server;
        m_server = nullptr;

        ui->tcpMsg->append("停止监听……");
        ui->tcpBtn->setText("Connect");
        ui->clientComboBox->clear();

        pTimer->stop();
        tcpConnected = false;
    }
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
        QString trailType = ui->trailComboBox->currentText();
        leftFoot->startDisplay(name, trailType, isResume);
        rightFoot->startDisplay(name, trailType, isResume);
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
        qDebug() << espIp;
        QString str = QString("%1%2").arg(btn->property("type").toString(), "Is");
        auto idx = var.value<qsizetype>();
        qDebug() << idx;
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


