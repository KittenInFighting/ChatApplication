#ifndef ADDBTN_H
#define ADDBTN_H
#include <QPushButton>

class AddBtn : public QPushButton
{
    Q_OBJECT
public:
    explicit AddBtn(QWidget *parent = nullptr);

    //设置图标路径
    void setPlusIcon(const QString &iconPath);
    void setBtnSize(int w, int h);

private:
    void applyStyle();
};


#endif // ADDBTN_H
