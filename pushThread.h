#ifndef PUSHTHREAD_H
#define PUSHTHREAD_H

#include <QThread>
#include <QString>
#include <QObject>
#include <QtMqtt/qmqttclient.h>
class MainWindow;

class PushThread : public QThread
{
    Q_OBJECT

public:
    explicit PushThread(QObject *parent = nullptr);
    void stop();
    void setOrder(int order);
    std::vector<int> peach_cooked;
    std::vector<int> peach_halfcooked;
    std::vector<int> peach_uncooked;
private:
    bool pushFirstRod=false;
    bool pushSecondRod=false;
    bool pushThirdRod=false;
    int switchFirstTimes=1;
    int switchSecondTimes=1;
    int switchThirdTimes=1;
    int order;
    int switchNumber;
    QString TextEditContent; // 保存文本内容
    QMqttClient *mqttClient;
    bool state = true;

    signals:
    void pushSecond();
    void pushThird();
    void pushFourth();

    void pullSecond();
    void pullThird();
    void pullFourth();
    public slots:
        void updateTextEditContent(const QString &content); // 定义槽函数以更新内容

protected:
    void run() override;
};

#endif // PUSHTHREAD_H