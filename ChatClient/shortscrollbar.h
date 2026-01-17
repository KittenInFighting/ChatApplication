#ifndef SHORTSCROLLBAR_H
#define SHORTSCROLLBAR_H

#include <QScrollBar>
#include <QColor>

class ShortScrollBar : public QScrollBar {
    Q_OBJECT
    Q_PROPERTY(QColor handleColor READ handleColor WRITE setHandleColor)
    Q_PROPERTY(QColor handleHoverColor READ handleHoverColor WRITE setHandleHoverColor)
    Q_PROPERTY(QColor handlePressedColor READ handlePressedColor WRITE setHandlePressedColor)
    Q_PROPERTY(int handleLength READ handleLength WRITE setHandleLength)
    Q_PROPERTY(int barWidth READ barWidth WRITE setBarWidth)
    Q_PROPERTY(int radius READ radius WRITE setRadius)

public:
    explicit ShortScrollBar(Qt::Orientation ori, QWidget *parentůnullptr);

    QColor handleColor() const { return m_handle; }
    QColor handleHoverColor() const { return m_hover; }
    QColor handlePressedColor() const { return m_pressed; }
    int handleLength() const { return m_len; }
    int barWidth() const { return m_w; }
    int radius() const { return m_r; }

    void setHandleColor(const QColor &c) { m_handle = c; update(); }
    void setHandleHoverColor(const QColor &c) { m_hover = c; update(); }
    void setHandlePressedColor(const QColor &c) { m_pressed = c; update(); }
    void setHandleLength(int v) { m_len = qMax(6, v); update(); }
    void setBarWidth(int v) { m_w = qMax(4, v); updateGeometry(); update(); }
    void setRadius(int v) { m_r = qMax(0, v); update(); }

protected:
    void paintEvent(QPaintEvent *e) override;
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    QSize sizeHint() const override;

private:
    QRect grooveRect() const;
    QRect handleRect() const;
    int valueFromPos(int y) const;

private:
    QColor m_handle = QColor(0,0,0,75);
    QColor m_hover = QColor(0,0,0,110);
    QColor m_pressed = QColor(0,0,0,140);
    int m_len = 18;     // 固定短滑块长度
    int m_w   = 6;      // 滚动条宽度
    int m_r   = 3;      // 圆角
    bool m_hovering = false;
    bool m_dragging = false;
    int  m_dragOffset = 0;
};

#endif // SHORTSCROLLBAR_H
