#ifndef INPUTTEXTEDIT_H
#define INPUTTEXTEDIT_H
#include <QTextEdit>

class InputTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    using QTextEdit::QTextEdit;

    void setOverlayMargins(int rightMargin, int bottomMargin)
    {
        // setViewportMargins 是 protected，只能在子类内部调用
        setViewportMargins(0, 0, rightMargin, bottomMargin);
    }

signals:

    //当 textEdit 尺寸变化时发出信号，连接此信号重新定位按钮
    void resized();

protected:

    //当控件大小变化时会被调用，让外面更新按钮位置
    void resizeEvent(QResizeEvent *e) override
    {
        QTextEdit::resizeEvent(e);
        emit resized();
    }
};

#endif // INPUTTEXTEDIT_H
