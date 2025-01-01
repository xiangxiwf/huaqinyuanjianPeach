#include "pushThread.h"
#include "mainwindow.h"
#include <QApplication>

PushThread::PushThread(QObject *parent)
    : QThread(parent) {
}

void PushThread::run() {
    switch (order) {
        case 1: {
            while (state) {
                if (TextEditContent.contains("{\"first_switch\":true}")) {
                    for (int i = 0; i < peach_cooked.size(); i++) {
                        if (peach_cooked.at(i) == switchFirstTimes) {
                            ++switchFirstTimes;
                            peach_cooked.erase(peach_cooked.begin() + i);
                            pushFirstRod = true;
                            break;
                        }
                        if (peach_cooked.at(i) != switchFirstTimes) {
                            ++switchFirstTimes;
                            break;
                        }
                    }
                }
                if (TextEditContent.contains("{\"first_switch\":false}") && pushFirstRod) {
                    emit pushSecond();
                    pushFirstRod = false;
                    TextEditContent.clear();
                    QThread::msleep(400);
                    emit pullSecond();
                }
            }
            break;
        }
        case 2: {
            while (state) {
                if (TextEditContent.contains("{\"second_switch\":true}")) {
                    for (int i = 0; i < peach_halfcooked.size(); i++) {
                        if (peach_halfcooked.at(i) == switchSecondTimes) {
                            ++switchSecondTimes;
                            peach_halfcooked.erase(peach_halfcooked.begin() + i);
                            pushSecondRod = true;
                            break;
                        }
                        if (peach_halfcooked.at(i) != switchSecondTimes) {
                            ++switchSecondTimes;
                            break;
                        }
                    }
                }
                if (TextEditContent.contains("{\"second_switch\":false}") && pushSecondRod) {
                    emit pushThird();
                    pushSecondRod = false;
                    TextEditContent.clear();
                    QThread::msleep(400);
                    emit pullThird();
                }
            }
            break;
        }
        case 3: {
            while (state) {
                if (TextEditContent.contains("{\"third_switch\":true}")) {
                    for (int i = 0; i < peach_uncooked.size(); i++) {
                        if (peach_uncooked.at(i) == switchThirdTimes) {
                            ++switchThirdTimes;
                            peach_uncooked.erase(peach_uncooked.begin() + i);
                            pushThirdRod = true;
                            break;
                        }
                        if (peach_uncooked.at(i) != switchThirdTimes) {
                            ++switchThirdTimes;
                            break;
                        }
                    }
                }
                if (TextEditContent.contains("{\"third_switch\":false}") && pushThirdRod) {
                    emit pushFourth();
                    pushThirdRod = false;
                    TextEditContent.clear();
                    QThread::msleep(400);
                    emit pullFourth();
                }
            }
            break;
        }
    }
}

void PushThread::stop() {
    state = false;
}

void PushThread::updateTextEditContent(const QString &content) {
    TextEditContent = content; // 更新 TextEditContent
}

void PushThread::setOrder(int order) {
    this->order = order;
}
