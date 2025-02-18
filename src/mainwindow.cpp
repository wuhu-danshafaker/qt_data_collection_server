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

    folderComboInit();
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

    loaderThread->quit();
    loaderThread->wait();
    delete folderLoader;
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
    QString csvDir = QString("../csvData/%1/%2").arg(dateStr, name);
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

    bool isResume = true;  // 已被弃用，在esp32上解决了resume和start的兼容问题
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
        leftFoot->checkConnection();
        rightFoot->checkConnection();
        QString trialType = ui->trialComboBox->currentText();
        QString saveDir = setSaveDir(name, trialType);
        leftFoot->startDisplay(name, saveDir, isResume);
        rightFoot->startDisplay(name, saveDir, isResume);
        recording = true;
        on_addServerMessage("pTimer: recording");
        baseTime = QTime::currentTime();
        updateTimeAndDisplay();
        pTimer->start(800);
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

//    loadSubFoldersToComboBox(ui->comboBox_date, "../csvData");
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

void MainWindow::loadSubFoldersToComboBox(QComboBox *comboBox, const QString &folderPath) {
    comboBox->clear();

    // 检查文件夹是否存在
    QDir dir(folderPath);
    if (!dir.exists()) {
        qDebug() << "Folder does not exist:" << folderPath;
        return;
    }

    // 获取所有子文件夹
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot); // 只获取文件夹，排除 "." 和 ".."
    QStringList subFolders = dir.entryList();

    // 将子文件夹名称添加到 QComboBox
    comboBox->addItems(subFolders);
}

void MainWindow::folderComboInit() {
    // 初始化 UI
    QComboBox* comboBoxDate = ui->comboBox_date;
    QComboBox* comboBoxName = ui->comboBox_name;
//    QComboBox* comboBoxTrail = ui->comboBox_trail;

    // 初始化文件夹加载器
    folderLoader = new FolderLoader;
    loaderThread = new QThread;
    folderLoader->moveToThread(loaderThread);
    loaderThread->start();

    connect(ui->checkBox_left, &QCheckBox::clicked, this, [&]() {
        ui->checkBox_left->setChecked(!ui->checkBox_left->isChecked());
    });
    connect(ui->checkBox_right, &QCheckBox::clicked, this, [&]() {
        ui->checkBox_right->setChecked(!ui->checkBox_right->isChecked()); // 保持当前状态不变
    });
    connect(ui->checkBox_result, &QCheckBox::clicked, this, [&]() {
        ui->checkBox_result->setChecked(!ui->checkBox_result->isChecked()); // 保持当前状态不变
    });

    // 连接信号槽
    connect(comboBoxDate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateComboBoxName);
    connect(comboBoxName, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateComboBoxTrail);
    connect(folderLoader, &FolderLoader::subFoldersLoaded, this, &MainWindow::onSubFoldersLoaded);

    // 初始化日期文件夹
    loadSubFoldersAsync("../csvData", comboBoxDate);
}

// 更新第二个 ComboBox（名称文件夹）
void MainWindow::updateComboBoxName() {
    QComboBox* comboBoxDate = ui->comboBox_date;
    QComboBox* comboBoxName = ui->comboBox_name;

    // 获取选定的日期文件夹
    QString dateFolder = comboBoxDate->currentText();
    QString csvDataPath = "../csvData"; // 替换为你的 csvData 文件夹路径
    QString nameFolderPath = csvDataPath + "/" + dateFolder;

    // 加载名称文件夹到 comboBoxName
//    loadSubFoldersToComboBox(comboBoxName, nameFolderPath);
    loadSubFoldersAsync(nameFolderPath, comboBoxName);
}

void MainWindow::updateComboBoxTrail() {
    QComboBox* comboBoxDate = ui->comboBox_date;
    QComboBox* comboBoxName = ui->comboBox_name;
    QComboBox* comboBoxTrail = ui->comboBox_trail;

    // 获取选定的日期文件夹和名称文件夹
    QString dateFolder = comboBoxDate->currentText();
    QString nameFolder = comboBoxName->currentText();
    QString csvDataPath = "../csvData";
    QString trailFolderPath = csvDataPath + "/" + dateFolder + "/" + nameFolder;

    loadSubFoldersAsync(trailFolderPath, comboBoxTrail);
}

void MainWindow::onSubFoldersLoaded(const QStringList &subFolders) {
    auto targetComboBox = qobject_cast<QComboBox*>(sender()->property("targetComboBox").value<QObject*>());
    if (targetComboBox) {
        targetComboBox->clear();
        targetComboBox->addItems(subFolders);
    }
}

void MainWindow::loadSubFoldersAsync(const QString &folderPath, QComboBox *targetComboBox) {
    // 设置目标 QComboBox
    folderLoader->setProperty("targetComboBox", QVariant::fromValue(static_cast<QObject*>(targetComboBox)));
    // 异步加载子文件夹
    QMetaObject::invokeMethod(folderLoader, "loadSubFolders", Qt::QueuedConnection, Q_ARG(QString, folderPath));
    ui->checkBox_left->setChecked(false);
    ui->checkBox_right->setChecked(false);
    ui->checkBox_result->setChecked(false);
}

void MainWindow::on_findRecordsBtn_clicked() {
    QComboBox* comboBoxDate = ui->comboBox_date;
    QComboBox* comboBoxName = ui->comboBox_name;
    QComboBox* comboBoxTrail = ui->comboBox_trail;

    QString dateFolder = comboBoxDate->currentText();
    QString nameFolder = comboBoxName->currentText();
    QString trailFolder = comboBoxTrail->currentText();
    if(dateFolder.isEmpty() | nameFolder.isEmpty() | trailFolder.isEmpty()){
        qDebug() << "Folder Not Exists";
    }

    QString csvDataPath = "../csvData";
    QString targetFolder = csvDataPath + "/" + dateFolder + "/" + nameFolder + "/" + trailFolder;

    QDir dir(targetFolder);
    if (!dir.exists()) {
        qDebug() << "Folder does not exist:" << targetFolder;
        return;
    }

    QStringList leftFiles = dir.entryList({"*_left_*.csv"}, QDir::Files);
    QStringList rightFiles = dir.entryList({"*_right_*.csv"}, QDir::Files);

    bool hasLeft = !leftFiles.isEmpty();
    bool hasRight = !rightFiles.isEmpty();
    bool hasResult = dir.exists("result.log") & dir.exists("data.json");

    ui->checkBox_left->setChecked(hasLeft);
    ui->checkBox_right->setChecked(hasRight);
    ui->checkBox_result->setChecked(hasResult);
    QString records = (hasLeft & hasRight)? "左右足记录已查询到" : "左右足记录缺失";
    ui->label_detail->setText("Detail: " + targetFolder + " " + records);
}

void MainWindow::on_calculateResultBtn_clicked() {
    if (!ui->checkBox_left->isChecked() | !ui->checkBox_right->isChecked()){
        qDebug() << "记录缺失";
        ui->docDetail->setText("记录缺失,无法计算");
        return;
    }

    // 创建 QProcess 实例
    auto *process = new QProcess(this);

    // 设置要执行的命令和参数
    QString program = "../scripts/analysis.exe"; // 替换为你的 EXE 路径
    QStringList arguments;

    QComboBox* comboBoxDate = ui->comboBox_date;
    QComboBox* comboBoxName = ui->comboBox_name;
    QComboBox* comboBoxTrail = ui->comboBox_trail;
    QString dateFolder = comboBoxDate->currentText();
    QString nameFolder = comboBoxName->currentText();
    QString trailFolder = comboBoxTrail->currentText();
    QString csvDataPath = "../csvData";
    QString targetFolder = csvDataPath + "/" + dateFolder + "/" + nameFolder + "/" + trailFolder;

    arguments << "--csvPath" << targetFolder;
    // 启动进程
    process->start(program, arguments);
    ui->docDetail->setText("计算中\n");
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
//        QByteArray output = process->readAllStandardOutput();
        ui->docDetail->append("计算完成");
    });

    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        QByteArray errorOutput = process->readAllStandardError();
        ui->docDetail->append(QString::fromLocal8Bit(errorOutput));
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

void MainWindow::on_showResultBtn_clicked() {
    ui->docDetail->clear();
    QComboBox* comboBoxDate = ui->comboBox_date;
    QComboBox* comboBoxName = ui->comboBox_name;
    QComboBox* comboBoxTrail = ui->comboBox_trail;
    QString dateFolder = comboBoxDate->currentText();
    QString nameFolder = comboBoxName->currentText();
    QString trailFolder = comboBoxTrail->currentText();
    QString csvDataPath = "../csvData";
    QString targetFolder = csvDataPath + "/" + dateFolder + "/" + nameFolder + "/" + trailFolder;

    QString jsonFile = targetFolder + "/" + "data.json";
    QFile file(jsonFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ui->docDetail->append("Failed to open JSON file.");
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull()) {
        ui->docDetail->append("Failed to parse JSON data.");
        return;
    }

    // 获取根对象
    QJsonObject rootObj = jsonDoc.object();

    // 解析 "imu para"
    if (rootObj.contains("imu para") && rootObj["imu para"].isObject()) {
        QJsonObject imuParaObj = rootObj["imu para"].toObject();
        ui->docDetail->append("IMU Result: \n");
        // 解析 "left"
        if (imuParaObj.contains("left") && imuParaObj["left"].isObject()
        && imuParaObj.contains("right") && imuParaObj["right"].isObject()) {
            QJsonObject leftObj = imuParaObj["left"].toObject();
            QJsonObject rightObj = imuParaObj["right"].toObject();
            ui->docDetail->append(QString("Stride Length(left, right): %1, %2")
                                          .arg(leftObj["stride length"].toDouble())
                                          .arg(rightObj["stride length"].toDouble()));
            ui->docDetail->append(QString("Stride Time(left, right): %1, %2")
                                          .arg(leftObj["stride time"].toDouble())
                                          .arg(rightObj["stride time"].toDouble()));
            ui->docDetail->append(QString("Stance Time(left, right): %1, %2")
                                          .arg(leftObj["stance time"].toDouble())
                                          .arg(rightObj["stance time"].toDouble()));
            ui->docDetail->append(QString("Swing Time(left, right): %1, %2")
                                          .arg(leftObj["swing time"].toDouble())
                                          .arg(rightObj["swing time"].toDouble()));
            ui->docDetail->append(QString("Speed(left, right): %1, %2")
                                          .arg(leftObj["speed"].toDouble())
                                          .arg(rightObj["speed"].toDouble()));
            ui->docDetail->append(QString("AnglePF(left, right): %1, %2")
                                          .arg(leftObj["anglePF"].toDouble())
                                          .arg(rightObj["anglePF"].toDouble()));
            ui->docDetail->append(QString("AngleDF(left, right): %1, %2")
                                          .arg(leftObj["angleDF"].toDouble())
                                          .arg(rightObj["angleDF"].toDouble()));
        }

        // 解析 "others"
        if (imuParaObj.contains("others") && imuParaObj["others"].isObject()) {
            QJsonObject othersObj = imuParaObj["others"].toObject();
            ui->docDetail->append(QString("Cadence: %1").arg(othersObj["cadence"].toDouble()));
            ui->docDetail->append(QString("SI of Swing Time: %1").arg(othersObj["si_sw"].toDouble()));
            ui->docDetail->append(QString("SI of Stride Length: %1").arg(othersObj["si_sl"].toDouble()));
            ui->docDetail->append(QString("CV of Double Support: %1").arg(othersObj["cv_ds"].toDouble()));

            // 解析数组 "cv_sw"
            if (othersObj.contains("cv_sw") && othersObj["cv_sw"].isArray()) {
                QJsonArray cvSwArray = othersObj["cv_sw"].toArray();
                ui->docDetail->append(QString("CV of Swing Time(left, right): %1, %2")
                                              .arg(cvSwArray[0].toDouble())
                                              .arg(cvSwArray[1].toDouble()));
            }

            // 解析数组 "cv_sl"
            if (othersObj.contains("cv_sl") && othersObj["cv_sl"].isArray()) {
                QJsonArray cvSlArray = othersObj["cv_sl"].toArray();
                ui->docDetail->append(QString("CV of Stride Length(left, right): %1, %2")
                                              .arg(cvSlArray[0].toDouble())
                                              .arg(cvSlArray[1].toDouble()));
            }

            // 解析数组 "cv_cd"
            if (othersObj.contains("cv_cd") && othersObj["cv_cd"].isArray()) {
                QJsonArray cvCdArray = othersObj["cv_cd"].toArray();
                ui->docDetail->append(QString("CV of Stride Time(left, right): %1, %2")
                                              .arg(cvCdArray[0].toDouble())
                                              .arg(cvCdArray[1].toDouble()));
            }
        }
    }

    // 6. 解析 "fsr para"
    if (rootObj.contains("fsr para") && rootObj["fsr para"].isObject()) {
        QJsonObject fsrParaObj = rootObj["fsr para"].toObject();
        ui->docDetail_2->clear();
        ui->docDetail_2->append("FSR Result: \n");
        // 解析 "left"
        if (fsrParaObj.contains("left") && fsrParaObj["left"].isObject()) {
            QJsonObject leftObj = fsrParaObj["left"].toObject();

            // 解析 "max_pressure"
            if (leftObj.contains("max_pressure") && leftObj["max_pressure"].isObject()
            && leftObj.contains("duration") && leftObj["duration"].isObject()
            && leftObj.contains("pressure integral") && leftObj["pressure integral"].isObject()) {
                QJsonObject maxPressureObj = leftObj["max_pressure"].toObject();
                QJsonObject durationObj = leftObj["duration"].toObject();
                QJsonObject pressureIntegralObj = leftObj["pressure integral"].toObject();
                for(int i=1;i<9;i++){
                    ui->docDetail_2->append(QString("Left FSR%1 (Max Pressure, Duration,Pressure Integral):"
                                                  "%2, %3, %4")
                                                  .arg(i)
                                                  .arg(maxPressureObj[QString("fsr%1").arg(i)].toDouble())
                                                  .arg(durationObj[QString("fsr%1").arg(i)].toDouble())
                                                  .arg(pressureIntegralObj[QString("fsr%1").arg(i)].toDouble()));
                }
            }
        }

        // 解析 "right"
        if (fsrParaObj.contains("right") && fsrParaObj["right"].isObject()) {
            QJsonObject rightObj = fsrParaObj["right"].toObject();

            // 解析 "max_pressure"
            if (rightObj.contains("max_pressure") && rightObj["max_pressure"].isObject()
                && rightObj.contains("duration") && rightObj["duration"].isObject()
                && rightObj.contains("pressure integral") && rightObj["pressure integral"].isObject()) {
                QJsonObject maxPressureObj = rightObj["max_pressure"].toObject();
                QJsonObject durationObj = rightObj["duration"].toObject();
                QJsonObject pressureIntegralObj = rightObj["pressure integral"].toObject();
                for(int i=1;i<9;i++){
                    ui->docDetail_2->append(QString("Right FSR%1 (Max Pressure, Duration,Pressure Integral):"
                                                  "%2, %3, %4")
                                                  .arg(i)
                                                  .arg(maxPressureObj[QString("fsr%1").arg(i)].toDouble())
                                                  .arg(durationObj[QString("fsr%1").arg(i)].toDouble())
                                                  .arg(pressureIntegralObj[QString("fsr%1").arg(i)].toDouble()));
                }
            }
        }
    }
}


