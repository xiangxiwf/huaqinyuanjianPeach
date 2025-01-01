#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QTimer>
#include <QtMqtt/qmqttclient.h>
#include <vector>
class PushThread;
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public:
    QPixmap getscaledPixmap();
    int getIndex();
    void setPeach();
    void pushRod();
    void setIndex(int index);
    void setEnginePath(QString enginePath);
    void setSubtopic(QString subtopic);
    void setPubtopic(QString pubtopic);
    void setMqttClient(QMqttClient *mqttClient);
    void setTimes(int times);
    QString getTextEditContent();
    QMqttClient *getMqttClient();
private:
    PushThread *firstSwitchThread;
    PushThread *secondSwitchThread;
    PushThread *thirdSwitchThread;
    int model;
    QString enginePath;
    QMqttClient *mqttClient = new QMqttClient(this);
    QString subtopic;
    QString pubtopic;
    int index;
    QPixmap scaledPixmap;
    int times=0;
    std::vector<std::vector<int> > peach = std::vector<std::vector<int> >(3);
    std::vector<int> rod = std::vector<int>(3);
    signals:
        void textContentChanged(const QString &content); // 自定义信号，用于传递 textEdit 内容

private slots:
    void on_pushButton_run_clicked(bool checked);

    void on_horizontalSlider_X_valueChanged(int value);

    void on_horizontalSlider_Y_valueChanged(int value);

    void on_radioButton_dxgi_clicked(bool checked);

    void on_radioButton_win_clicked(bool checked);

    void on_pushButton_ptToOnnx_clicked();

    void on_pushButton_onnxToEngine_clicked();

    void on_pushButton_engine_clicked();

    void on_pushButton_save_clicked();

    void on_pushButton_send_clicked(bool checked);

    void on_pushButton_conveyor_clicked(bool checked);

    void on_pushButton_rodFirst_clicked(bool checked);

    void on_pushButton_rodSecond_clicked(bool checked);

    void on_pushButton_rodThird_clicked(bool checked);

    void on_pushButton_rodFourth_clicked(bool checked);

    void on_pushButton_rodAll_clicked(bool checked);

    void on_textEditImg_textChanged();

    void pushRodSecond();

    void pushRodThird();

    void pushRodFourth();

    void pullRodSecond();

    void pullRodThird();

    void pullRodFourth();
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
