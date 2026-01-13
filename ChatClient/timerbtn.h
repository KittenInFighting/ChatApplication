#ifndef TIMERBTN_H
#define TIMERBTN_H
#include <QPushButton>
#include <QTimer>

const int max = 30;
class TimerBtn:public QPushButton
{
    Q_OBJECT
public:
    explicit TimerBtn(QWidget* parent = nullptr);
    void reset();
    ~TimerBtn();
private:

    QTimer* _timer;
    int _counter;
signals:
    void startRequest();
private slots:
    void onTimeout();
};

#endif // TIMERBTN_H
