#include "timerbtn.h"
#include <QMouseEvent>
#include <QDebug>

TimerBtn::TimerBtn(QWidget *parent):QPushButton(parent),_counter(max)
{
    _timer = new QTimer(this);
    setText("获取");

    _timer -> setInterval(1000);//每秒触发一次

    //倒计时更新
    connect(_timer, &QTimer::timeout, this, &TimerBtn::onTimeout);

    connect(this, &QPushButton::clicked, this,[this](){
        emit startRequest();//发送信号开始计时
        this -> setEnabled(false);
        this -> setText(QString::number(_counter));
        _timer -> start(1000);

    });
    //时间结束
    connect(_timer, &QTimer::timeout, this, [this](){
        _counter--;
        if (_counter <= 0) {
            _timer->stop();
            _counter = 30;
            this->setText("获取");
            this->setEnabled(true);
            return;
        }
        this->setText(QString::number(_counter));
    });

}

void TimerBtn::reset()
{
    if (_timer->isActive()) {
        _timer->stop();
    }

    _counter = max;
    setText(tr("获取"));
    setEnabled(true);
}

TimerBtn::~TimerBtn()
{
    _timer -> stop();
}

void TimerBtn::onTimeout()
{
    _counter--;
    if (_counter <= 0) {
        _timer->stop();
        _counter = 30;
        this->setText("获取");
        this->setEnabled(true);
        return;
    }
    setText(QString::number(_counter));
}
