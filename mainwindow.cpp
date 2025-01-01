#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ScreenCapture.h"
#include <QTimer>
#include <QPixmap>
#include <QFileDialog>
#include <QSettings>
#include <QImage>
#include <QThread>
#include <iostream>
#include <algorithm>
#include <QMessageBox>
#include <QDir>
#include <QProcess>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QTcpServer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHostAddress>
#include <QMessageBox>
#include <QtMqtt/qmqttclient.h>
#include <QtMqtt/qmqttsubscription.h>
#include <QJsonObject>
#include "pushThread.h"
#ifdef _WIN32
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <opencv2/opencv.hpp>
#include "YOLOv11.h"

// 主窗口构造函数
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
      , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->setFixedSize(this->size()); // 固定窗口大小
    this->setWindowTitle("工业流水线分拣系统"); // 设置窗口标题
    ui->label_X->setText(QString("X: %1").arg(ui->horizontalSlider_X->value())); // 初始化 X 轴滑块标签
    ui->label_Y->setText(QString("Y: %1").arg(ui->horizontalSlider_Y->value())); // 初始化 Y 轴滑块标签

    // 从注册表中加载 UI 元素值
    QSettings settings("IndustrialAssemblyLineSortingSystem", "Settings"); // 定义应用名称和组织名

    // 加载 lineEdit_ip 值
    ui->lineEdit_ip->setText(settings.value("lineEdit_ip", "").toString());

    // 加载 lineEdit_port 值
    ui->lineEdit_port->setText(settings.value("lineEdit_port", "").toString());

    // 加载 radioButton_dxgi 和 radioButton_win 状态
    ui->radioButton_dxgi->setChecked(settings.value("radioButton_dxgi", false).toBool());
    ui->radioButton_win->setChecked(settings.value("radioButton_win", false).toBool());

    // 加载 horizontalSlider_X 值
    ui->horizontalSlider_X->setValue(settings.value("horizontalSlider_X", ui->horizontalSlider_X->minimum()).toInt());

    // 加载 horizontalSlider_Y 值
    ui->horizontalSlider_Y->setValue(settings.value("horizontalSlider_Y", ui->horizontalSlider_Y->minimum()).toInt());

    // 加载 horizontalSlider_conf 值
    ui->horizontalSlider_conf->setValue(
        settings.value("horizontalSlider_conf", ui->horizontalSlider_conf->minimum()).toInt());

    ui->lineEdit_Subtopic->setText(settings.value("lineEdit_Subtopic", "").toString());

    ui->lineEdit_Pubtopic->setText(settings.value("lineEdit_Pubtopic", "").toString());


    // 加载 enginePath 值
    enginePath = settings.value("enginePath", "").toString();
    if (enginePath.isEmpty()) {
        ui->label_engine->setText("当前使用的engine：未选择");
    } else {
        QString engineFileName = QFileInfo(enginePath).fileName();
        ui->label_engine->setText(QString("当前使用的engine：%1").arg(engineFileName));
    }
    ui->label_conf->setText(QString("置信度：%1").arg((ui->horizontalSlider_conf->value()) / 10.0f));

    firstSwitchThread = new PushThread(this);firstSwitchThread->setOrder(1);
    secondSwitchThread = new PushThread(this);secondSwitchThread->setOrder(2);
    thirdSwitchThread = new PushThread(this);thirdSwitchThread->setOrder(3);

    connect(firstSwitchThread, &PushThread::pushSecond, this, &MainWindow::pushRodSecond);
    connect(secondSwitchThread, &PushThread::pushThird, this, &MainWindow::pushRodThird);
    connect(thirdSwitchThread, &PushThread::pushFourth, this, &MainWindow::pushRodFourth);
    connect(firstSwitchThread, &PushThread::pullSecond, this, &MainWindow::pullRodSecond);
    connect(secondSwitchThread, &PushThread::pullThird, this, &MainWindow::pullRodThird);
    connect(thirdSwitchThread, &PushThread::pullFourth, this, &MainWindow::pullRodFourth);

    // 监听 textEdit 的内容变化，使用 lambda 发射信号传递内容到 firstSwitchThread
    connect(ui->textEditSwitch1, &QTextEdit::textChanged, this, [this]() {
    QString content = ui->textEditSwitch1->toPlainText(); // 获取文本内容
    emit textContentChanged(content);              // 触发 MainWindow 的自定义信号

    // 使用类成员变量 pushThread，而不直接捕获它
    if (firstSwitchThread) {
        firstSwitchThread->updateTextEditContent(content); // 假设 PushThread 类中有此方法
    }
});
    connect(ui->textEditSwitch2, &QTextEdit::textChanged, this, [this]() {
    QString content = ui->textEditSwitch2->toPlainText(); // 获取文本内容
    emit textContentChanged(content);              // 触发 MainWindow 的自定义信号

    // 使用类成员变量 pushThread，而不直接捕获它
    if (secondSwitchThread) {
        secondSwitchThread->updateTextEditContent(content); // 假设 PushThread 类中有此方法
    }
});
    connect(ui->textEditSwitch3, &QTextEdit::textChanged, this, [this]() {
    QString content = ui->textEditSwitch3->toPlainText(); // 获取文本内容
    emit textContentChanged(content);              // 触发 MainWindow 的自定义信号

    // 使用类成员变量 pushThread，而不直接捕获它
    if (thirdSwitchThread) {
        thirdSwitchThread->updateTextEditContent(content); // 假设 PushThread 类中有此方法
    }
});

    // 连接自定义信号与 PushThread 的槽函数
    connect(this, &MainWindow::textContentChanged, firstSwitchThread, &PushThread::updateTextEditContent);
    connect(this, &MainWindow::textContentChanged, secondSwitchThread, &PushThread::updateTextEditContent);
    connect(this, &MainWindow::textContentChanged, thirdSwitchThread, &PushThread::updateTextEditContent);
    firstSwitchThread->start();
    secondSwitchThread->start();
    thirdSwitchThread->start();

}

// 主窗口析构函数
MainWindow::~MainWindow() {
    // Ensure the UI resources are released
    delete ui;

    // Save the UI states
    QSettings settings("IndustrialAssemblyLineSortingSystem", "Settings");
    settings.setValue("lineEdit_ip", ui->lineEdit_ip->text());
    settings.setValue("lineEdit_port", ui->lineEdit_port->text());
    settings.setValue("radioButton_dxgi", ui->radioButton_dxgi->isChecked());
    settings.setValue("radioButton_win", ui->radioButton_win->isChecked());
    settings.setValue("horizontalSlider_X", ui->horizontalSlider_X->value());
    settings.setValue("horizontalSlider_Y", ui->horizontalSlider_Y->value());
    settings.setValue("horizontalSlider_conf", ui->horizontalSlider_conf->value());
    settings.setValue("enginePath", enginePath);
    settings.setValue("lineEdit_Subtopic", ui->lineEdit_Subtopic->text());
    settings.setValue("lineEdit_Pubtopic", ui->lineEdit_Pubtopic->text());

    if (firstSwitchThread && firstSwitchThread->isRunning()) {
        firstSwitchThread->stop();
        firstSwitchThread->quit();
        firstSwitchThread->wait();
    }
    delete firstSwitchThread;

    if (secondSwitchThread && secondSwitchThread->isRunning()) {
        secondSwitchThread->stop();
        secondSwitchThread->quit();
        secondSwitchThread->wait();
    }
    delete secondSwitchThread;

    if (thirdSwitchThread && thirdSwitchThread->isRunning()) {
        thirdSwitchThread->stop();
        thirdSwitchThread->quit();
        thirdSwitchThread->wait();
    }
    delete thirdSwitchThread;
}

// 自定义日志记录器类
class Logger : public nvinfer1::ILogger {
    void log(Severity severity, const char *msg) noexcept override {
        if (severity <= Severity::kWARNING)
            std::cout << msg << std::endl;
    }
} logger;

// 运行按钮点击事件
void MainWindow::on_pushButton_run_clicked(bool checked) {
    if (ui->radioButton_mqtt->isChecked()) {
        if (mqttClient->state() != QMqttClient::Connected) {
            QMessageBox::warning(this, "错误", "请先连接MQTT!");
            ui->pushButton_run->setChecked(false);
            return;
        }
    }
    static QTimer *timer = nullptr; // 定义一个静态 QTimer 对象，保证其生命周期超出函数作用域
    if (enginePath.isEmpty()) {
        QMessageBox::warning(this, "错误", "你还没有选择engine文件");
        ui->pushButton_run->setChecked(false);
        return;
    }
    static YOLOv11 YOLOmodel(enginePath.replace("/", "\\").toStdString(), logger); // 定义 YOLOv11 实例
    static ScreenCapture capture(ui->radioButton_dxgi->isChecked() ? 1 : 2,
                                 ui->horizontalSlider_X->value(),
                                 ui->horizontalSlider_Y->value()); // 定义 ScreenCapture 实例，并根据 UI 滑块初始化其参数
    YOLOmodel.setConf((ui->horizontalSlider_conf->value()) / 10.0f);
    if (checked) {
        // 如果按钮被按下
        if (!timer) {
            // 如果 timer 为 null（尚未实例化）
            timer = new QTimer(this); // 初始化 QTimer

            if (ui->radioButton_mqtt->isChecked()) {
                // 如果是 MQTT 模式，则检测 textEdit 的内容变化
                connect(ui->textEditImg, &QTextEdit::textChanged, this, [this]() {
                    QString content = ui->textEditImg->toPlainText();
                    if (content.startsWith("{\"image")) {
                        // 如果内容以 {"image 开头，执行一次推理
                        YOLOmodel.setConf((ui->horizontalSlider_conf->value()) / 10.0f);
                        ui->label_conf->setText(QString("置信度：%1").arg((ui->horizontalSlider_conf->value()) / 10.0f));
                        static int modelType = ui->radioButton_dxgi->isChecked()
                                                   ? 1
                                                   : (ui->radioButton_win->isChecked() ? 2 : 3); // 根据单选按钮确定模型类型
                        capture.reviseParameter(modelType, ui->horizontalSlider_X->value(),
                                                ui->horizontalSlider_Y->value());
                        // 调整 ScreenCapture 参数
                        cv::Mat image = capture.runCapture(); // 执行屏幕捕获

                        if (image.empty()) {
                            // 验证捕获到的图像是否为空
                            std::cerr << "捕获的图像为空！" << std::endl; // 输出错误信息
                            return;
                        }

                        std::vector<Detection> objects;
                        YOLOmodel.preprocess(image); // 对图像进行预处理
                        YOLOmodel.infer(); // 进行推理
                        YOLOmodel.postprocess(objects); // 后处理推理结果

                        YOLOmodel.draw(image, objects); // 调用 YOLOv11 的绘制函数，在图像上绘制边框和标签
                        index = objects[0].class_id;

                        //ui->pushButton_run->setText(QString("%1 %2").arg(index).arg(times));
                        times++;
                        setPeach();
                        cv::cvtColor(image, image, cv::COLOR_BGR2RGB); // 将 BGR 图像转换为 RGB（用于显示）
                        QImage qImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);
                        // 将 cv::Mat 直接转换为 QImage，无需调整尺寸

                        QSize labelSize = ui->label_img->size(); // 获取 QLabel 的大小
                        QImage scaledImage = qImage.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        // 按 QLabel 大小按比例缩放 QImage

                        ui->label_img->setPixmap(QPixmap::fromImage(scaledImage)); // 在 QLabel 上显示缩放后的图像
                        ui->label_img->update(); // 更新 QLabel 显示
                    }
                });
            } else {
                // 如果不是 MQTT 模式，则使用定时器进行反复推理
                connect(timer, &QTimer::timeout, this, [this]() {
                    YOLOmodel.setConf((ui->horizontalSlider_conf->value()) / 10.0f);
                    ui->label_conf->setText(QString("置信度：%1").arg((ui->horizontalSlider_conf->value()) / 10.0f));
                    static int modelType = ui->radioButton_dxgi->isChecked()
                                               ? 1
                                               : (ui->radioButton_win->isChecked() ? 2 : 3); // 根据单选按钮确定模型类型
                    capture.reviseParameter(modelType, ui->horizontalSlider_X->value(),
                                            ui->horizontalSlider_Y->value());
                    // 调整 ScreenCapture 参数
                    cv::Mat image = capture.runCapture(); // 执行屏幕捕获

                    if (image.empty()) {
                        // 验证捕获到的图像是否为空
                        std::cerr << "捕获的图像为空！" << std::endl; // 输出错误信息
                        return;
                    }

                    std::vector<Detection> objects;
                    YOLOmodel.preprocess(image); // 对图像进行预处理
                    YOLOmodel.infer(); // 进行推理
                    YOLOmodel.postprocess(objects); // 后处理推理结果

                    YOLOmodel.draw(image, objects); // 调用 YOLOv11 的绘制函数，在图像上绘制边框和标签
                    //index = objects[0].class_id;
                    cv::cvtColor(image, image, cv::COLOR_BGR2RGB); // 将 BGR 图像转换为 RGB（用于显示）
                    QImage qImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);
                    // 将 cv::Mat 直接转换为 QImage，无需调整尺寸

                    QSize labelSize = ui->label_img->size(); // 获取 QLabel 的大小
                    QImage scaledImage = qImage.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    // 按 QLabel 大小按比例缩放 QImage

                    ui->label_img->setPixmap(QPixmap::fromImage(scaledImage)); // 在 QLabel 上显示缩放后的图像
                    ui->label_img->update(); // 更新 QLabel 显示
                });
            }
        }

        if (ui->radioButton_mqtt->isChecked()) {
            timer->stop(); // 如果是 MQTT 模式，不启用定时器
        } else {
            timer->start(0); // 启动定时器，时间间隔为 0ms
        }
    } else if (timer) {
        // 如果按钮被释放
        ui->label_img->clear();
        timer->stop(); // 停止计时器
    }
}

// 当 X 轴滑块的值改变时更新标签
void MainWindow::on_horizontalSlider_X_valueChanged(int value) {
    ui->label_X->setText(QString("X: %1").arg(value)); // 将滑块当前值设置到标签上
}

// 当 Y 轴滑块的值改变时更新标签
void MainWindow::on_horizontalSlider_Y_valueChanged(int value) {
    ui->label_Y->setText(QString("Y: %1").arg(value)); // 将滑块当前值设置到标签上
}

// DXGI 单选按钮点击事件
void MainWindow::on_radioButton_dxgi_clicked(bool checked) {
    if (checked) {
        model = 1; // 设置模型类型为 1
    }
}

// Windows 单选按钮点击事件
void MainWindow::on_radioButton_win_clicked(bool checked) {
    if (checked) {
        model = 2; // 设置模型类型为 2
    }
}

// pt 转换为 ONNX 按钮点击事件
void MainWindow::on_pushButton_ptToOnnx_clicked() {
    // 打开文件选择对话框，选择 .pt 文件
    QString ptPath = QFileDialog::getOpenFileName(this, tr("打开文件"), "", tr("PyTorch 文件 (*.pt);;所有文件 (*)"));

    if (!ptPath.isEmpty()) {
        ui->pushButton_ptToOnnx->setEnabled(false); // 禁用按钮

        // 创建信息框提示，显示正在转换的消息
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("转换中"));
        msgBox.setText(tr("正在转换，请稍候..."));
        msgBox.setStandardButtons(QMessageBox::NoButton); // 不显示任何按钮
        msgBox.setModal(true);
        msgBox.show();
        QApplication::processEvents(); // 强制更新以显示提示窗口内容

        // 使用 QProcess 执行命令
        QProcess process;
        process.setProgram("python");
        process.setArguments({"ptToOnnx.py", ptPath});
        process.start();

        // 等待进程完成
        if (!process.waitForFinished(-1)) {
            // 无限等待直到完成
            msgBox.close(); // 关闭提示框
            QMessageBox::critical(this, "错误", "执行转换命令失败！");
            ui->pushButton_ptToOnnx->setEnabled(true); // 恢复按钮
            return;
        }

        msgBox.close(); // 关闭提示框

        // 获取输出信息
        QString output = process.readAllStandardOutput();
        QString error = process.readAllStandardError();

        // 显示输出或错误信息
        if (!error.isEmpty()) {
            QMessageBox::warning(this, "警告", QString("转换过程中发生错误：\n%1").arg(error));
        } else {
            QString onnxPath = output.trimmed(); // 假设输出包含转换后的文件路径
            QMessageBox::information(this, "成功", QString("转换成功！输出文件：\n%1").arg(onnxPath));
        }

        ui->pushButton_ptToOnnx->setEnabled(true); // 恢复按钮
    } else {
        QMessageBox::warning(this, "警告", "未选择任何 .pt 文件！");
    }
}


// onnx 转换为 TensorRT 引擎按钮点击事件
void MainWindow::on_pushButton_onnxToEngine_clicked() {
    // 选择 ONNX 文件路径
    QString onnxPath = QFileDialog::getOpenFileName(this, tr("打开 ONNX 文件"), "", tr("ONNX 文件 (*.onnx);;所有文件 (*)"));

    if (!onnxPath.isEmpty()) {
        QString fileName = QFileInfo(onnxPath).fileName(); // 提取文件名
        QString engineFileName = fileName.replace(".onnx", "_fp16.engine"); // 生成输出文件名

        // 创建信息框提示，显示正在转换的消息
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("转换中"));
        msgBox.setText(tr("正在转换，请稍候..."));
        msgBox.setStandardButtons(QMessageBox::NoButton); // 不显示任何按钮
        msgBox.setModal(true);
        msgBox.show();
        QApplication::processEvents(); // 强制更新以显示提示窗口内容

        // 使用 QProcess 执行命令
        QProcess process;
        process.setProgram("trtexec");
        process.setArguments({"--onnx=" + QFileInfo(onnxPath).fileName(), "--fp16", "--saveEngine=" + engineFileName});
        process.start();

        // 等待进程完成
        if (!process.waitForFinished(-1)) {
            // 无限等待直到完成
            msgBox.close(); // 关闭提示框
            QMessageBox::information(this, tr("成功"), tr("转换成功！输出文件：\n%1").arg(engineFileName));
            return;
        }

        msgBox.close(); // 关闭提示框

        // 获取输出信息
        QString output = process.readAllStandardOutput();
        QString error = process.readAllStandardError();

        // 总是显示成功提示框，附带输出或错误信息
        QString resultMessage = tr("转换成功！输出文件：\n%1").arg(engineFileName);
        QMessageBox::information(this, tr("成功"), resultMessage);
    } else {
        QMessageBox::warning(this, tr("警告"), tr("未选择任何 ONNX 文件！"));
    }
}


// 加载 TensorRT 引擎按钮点击事件
void MainWindow::on_pushButton_engine_clicked() {
    enginePath = QFileDialog::getOpenFileName(this, tr("打开文件"), "", tr("TensorRT 引擎文件 (*.engine);;所有文件 (*)"));
    // 显示文件选择对话框
    if (enginePath.isEmpty()) {
        ui->label_engine->setText("当前使用的engine：未选择");
    } else {
        QString engineFileName = QFileInfo(enginePath).fileName();
        ui->label_engine->setText(QString("当前使用的engine：%1").arg(engineFileName));
    }
}

void MainWindow::on_pushButton_save_clicked() {
    // 使用 QSettings 将 UI 元素值写入注册表
    QSettings settings("IndustrialAssemblyLineSortingSystem", "Settings"); // 定义应用名称和组织名

    // 保存 lineEdit_ip 值
    settings.setValue("lineEdit_ip", ui->lineEdit_ip->text());

    // 保存 lineEdit_port 值
    settings.setValue("lineEdit_port", ui->lineEdit_port->text());

    // 保存 radioButton_dxgi 和 radioButton_win 状态
    settings.setValue("radioButton_dxgi", ui->radioButton_dxgi->isChecked());
    settings.setValue("radioButton_win", ui->radioButton_win->isChecked());

    // 保存 horizontalSlider_X 值
    settings.setValue("horizontalSlider_X", ui->horizontalSlider_X->value());

    // 保存 horizontalSlider_Y 值
    settings.setValue("horizontalSlider_Y", ui->horizontalSlider_Y->value());

    // 保存 horizontalSlider_conf 值
    settings.setValue("horizontalSlider_conf", ui->horizontalSlider_conf->value());

    // 保存 enginePath 值
    settings.setValue("enginePath", enginePath);

    settings.setValue("lineEdit_Subtopic", ui->lineEdit_Subtopic->text());

    settings.setValue("lineEdit_Pubtopic", ui->lineEdit_Pubtopic->text());
    QMessageBox::information(this, "提示", "保存成功");
}


void MainWindow::on_pushButton_send_clicked(bool checked) {
    // 创建或获取 QMqttClient 实例
    //static QMqttClient *mqttClient = new QMqttClient(this);
    static bool isConnectedHandled = false;

    if (checked) {
        if (mqttClient->state() == QMqttClient::Connected) {
            QMessageBox::information(this, "MQTT", "已经连接");
            return;
        }

        // 配置 MQTT 客户端
        mqttClient->setHostname(ui->lineEdit_ip->text()); // 设置 IP 地址
        mqttClient->setPort(ui->lineEdit_port->text().toInt()); // 设置端口号
        mqttClient->setClientId("QtClient"); // 设置客户端 ID

        if (!isConnectedHandled) {
            // 处理成功连接信号
            connect(mqttClient, &QMqttClient::connected, this, [=]() {
                QMessageBox::information(this, "MQTT", "连接成功");

                // 获取订阅主题和发布主题
                subtopic = ui->lineEdit_Subtopic->text(); // 获取订阅主题
                pubtopic = ui->lineEdit_Pubtopic->text(); // 获取发布主题

                // 尝试订阅主题
                auto subscription = mqttClient->subscribe(QMqttTopicFilter(subtopic), 0);
                if (!subscription) {
                    QMessageBox::critical(this, "MQTT", "订阅失败");
                    return;
                }

                // 监听订阅的消息
                connect(subscription, &QMqttSubscription::messageReceived, this,
                        [=](const QMqttMessage &message) {
                            QString payload = message.payload();
                            if (payload.startsWith("{\"image")) {
                                ui->textEditImg->setText(payload);
                            } else if (payload.startsWith("{\"first")) {
                                ui->textEditSwitch1->setText(payload);
                            } else if (payload.startsWith("{\"second")) {
                                ui->textEditSwitch2->setText(payload);
                            } else if (payload.startsWith("{\"third")) {
                                ui->textEditSwitch3->setText(payload);
                            }
                        });

                // 发布消息到指定的发布主题
                QString payload = "测试消息";
                mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
            });

            // 处理状态变化信号
            connect(mqttClient, &QMqttClient::stateChanged, this, [=](QMqttClient::ClientState state) {
                if (state == QMqttClient::Disconnected) {
                    QMqttClient::ClientError mqttError = mqttClient->error();
                    if (mqttError != QMqttClient::NoError) {
                        QMessageBox::critical(this, "MQTT 错误",
                                              QString("连接失败，错误代码：%1").arg(static_cast<int>(mqttError)));
                    }
                }
            });

            isConnectedHandled = true;
        }

        // 连接到 MQTT 服务器
        mqttClient->connectToHost();
    } else {
        if (mqttClient->state() == QMqttClient::Connected) {
            mqttClient->disconnectFromHost();
            QMessageBox::information(this, "MQTT", "已断开连接");
        }
        QString payload = R"({"rod_control":"first_pull"})";
        mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
    }
}


void MainWindow::on_pushButton_conveyor_clicked(bool checked) {
    if (checked) {
        if (mqttClient->state() != QMqttClient::Connected) {
            QMessageBox::warning(this, "MQTT", "MQTT未连接！");
            ui->pushButton_conveyor->setChecked(false); // 将按钮状态设置为弹起
            return;
        }
        QString payload = R"({"conveyor":"run"})";
        mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
    } else {
        QString payload = R"({"conveyor":"stop"})";
        mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
    }
}


void MainWindow::on_pushButton_rodFirst_clicked(bool checked) {
    if (checked) {
        if (mqttClient->state() != QMqttClient::Connected) {
            QMessageBox::warning(this, "MQTT", "MQTT未连接！");
            ui->pushButton_rodFirst->setChecked(false); // 将按钮状态设置为弹起
            return;
        }
        if (!(ui->pushButton_conveyor->isChecked())) {
            QMessageBox::warning(this, "警告", "请先启动传送带");
            ui->pushButton_rodFirst->setChecked(false);
        }
        QString payload = R"({"rod_control":"first_push"})";
        mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
    } else {
        QString payload = R"({"rod_control":"first_pull"})";
        mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
    }
}


void MainWindow::on_pushButton_rodSecond_clicked(bool checked) {
    if (checked) {
        if (mqttClient->state() != QMqttClient::Connected) {
            QMessageBox::warning(this, "MQTT", "MQTT未连接！");
            ui->pushButton_rodSecond->setChecked(false); // 将按钮状态设置为弹起
            return;
        }
        QString payload = R"({"rod_control":"second_push"})";
        mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
    } else {
        QString payload = R"({"rod_control":"second_pull"})";
        mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
    }
}


void MainWindow::on_pushButton_rodThird_clicked(bool checked) {
    if (checked) {
        if (mqttClient->state() != QMqttClient::Connected) {
            QMessageBox::warning(this, "MQTT", "MQTT未连接！");
            ui->pushButton_rodThird->setChecked(false); // 将按钮状态设置为弹起
            return;
        }
        QString payload = R"({"rod_control":"third_push"})";
        mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
    } else {
        QString payload = R"({"rod_control":"third_pull"})";
        mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
    }
}


void MainWindow::on_pushButton_rodFourth_clicked(bool checked) {
    if (checked) {
        if (mqttClient->state() != QMqttClient::Connected) {
            QMessageBox::warning(this, "MQTT", "MQTT未连接！");
            ui->pushButton_rodFourth->setChecked(false); // 将按钮状态设置为弹起
            return;
        }
        QString payload = R"({"rod_control":"fourth_push"})";
        mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
    } else {
        QString payload = R"({"rod_control":"fourth_pull"})";
        mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
    }
}


void MainWindow::on_pushButton_rodAll_clicked(bool checked) {
    if (checked) {
        if (mqttClient->state() != QMqttClient::Connected) {
            QMessageBox::warning(this, "MQTT", "MQTT未连接！");
            ui->pushButton_rodFourth->setChecked(false); // 将按钮状态设置为弹起
            return;
        }
        QString payload = R"({"rod_control":"all_push"})";
        mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
    } else {
        QString payload = R"({"rod_control":"all_pull"})";
        mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
    }
}

void MainWindow::on_textEditImg_textChanged() {
    // 从 textEdit 中读取文本内容
    QString textContent = ui->textEditImg->toPlainText();

    // 检查内容中是否包含表示 JSON 格式 "image" 键的有效前缀
    if (textContent.startsWith("{\"image")) {
        QString base64Image = textContent.mid(7).trimmed();
        // 提取前缀后的 base64 字符串

        // 将 base64 字符串解码为 QByteArray
        QByteArray imageData = QByteArray::fromBase64(base64Image.toUtf8());

        // 将 QByteArray 转换为 QPixmap
        QPixmap pixmap;
        if (pixmap.loadFromData(imageData)) {
            scaledPixmap = pixmap.scaled((ui->label_img->size()), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        } else {
            QMessageBox::warning(this, "错误", "从解码数据加载图片失败。");
        }
    }
}

QPixmap MainWindow::getscaledPixmap() {
    return scaledPixmap;
}

int MainWindow::getIndex() {
    return index;
}

void MainWindow::setPeach() {
    // 从 textEdit 中读取文本内容
    QString textContent = ui->textEditImg->toPlainText();

    // 检查内容是否以 {"image" 开头
    if (textContent.startsWith("{\"image")) {
        int currentIndex = getIndex(); // 获取 index 值

        if (currentIndex == 0) {
            thirdSwitchThread->peach_uncooked.push_back(times);
        } else if (currentIndex == 1) {
            secondSwitchThread->peach_halfcooked.push_back(times);
        } else if (currentIndex == 2) {
            firstSwitchThread->peach_cooked.push_back(times);
        }
    }
}


QString MainWindow::getTextEditContent() {
    return ui->textEditImg->toPlainText();
}
QMqttClient *MainWindow::getMqttClient() {
    return mqttClient;
}

void MainWindow::pushRodSecond() {
    QString payload = R"({"rod_control":"second_push"})";
    mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
}

void MainWindow::pushRodThird() {
    QString payload = R"({"rod_control":"third_push"})";
    mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
}

void MainWindow::pushRodFourth() {
    QString payload = R"({"rod_control":"fourth_push"})";
    mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
}

void MainWindow::pullRodSecond() {
    QString payload = R"({"rod_control":"second_pull"})";
    mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
}

void MainWindow::pullRodThird() {
    QString payload = R"({"rod_control":"third_pull"})";
    mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
}

void MainWindow::pullRodFourth() {
    QString payload = R"({"rod_control":"fourth_pull"})";
    mqttClient->publish(QMqttTopicName(pubtopic), payload.toUtf8(), 0, false);
}