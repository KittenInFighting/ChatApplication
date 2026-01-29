#ifndef WHITEPAGE_H
#define WHITEPAGE_H

#include <QWidget>

namespace Ui {
class WhitePage;
}

class WhitePage : public QWidget
{
    Q_OBJECT

public:
    explicit WhitePage(QWidget *parent = nullptr);
    ~WhitePage();

private:
    Ui::WhitePage *ui;
};

#endif // WHITEPAGE_H
