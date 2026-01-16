#include "addbtn.h"
#include <QIcon>
#include <QSize>

AddBtn::AddBtn(QWidget *parent)
    : QPushButton(parent)
{
    // 固定大小30x30
    setFixedSize(30, 30);

    // 不显示文字，只显示图标
    setText("");

    // 鼠标手型、不要焦点框（更像 UI 按钮）
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);

    // 默认样式
    applyStyle();
}

void AddBtn::setPlusIcon(const QString &iconPath)
{
    setIcon(QIcon(iconPath));

    // 图标大小
    setIconSize(QSize(18, 18));
}

void AddBtn::setBtnSize(int w, int h)
{
    setFixedSize(w, h);

    // 圆角按较短边的一部分来
    applyStyle();
}

void AddBtn::applyStyle()
{
    const int radius = 8;

    setStyleSheet(QString(R"(
QPushButton{
    background:#F2F3F5;
    border:none;
    border-radius:%1px;
}
QPushButton:hover{
    background:#E9EAED;
}
QPushButton:pressed{
    background:#DFE1E6;
}
)").arg(radius));
}
