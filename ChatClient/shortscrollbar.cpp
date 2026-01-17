#include "shortscrollbar.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>

ShortScrollBar::ShortScrollBar(Qt::Orientation ori, QWidget *parent)
    : QScrollBar(ori, parent)
{
    setMouseTracking(true);
    setCursor(Qt::ArrowCursor);
    connect(this, &QScrollBar::valueChanged, this, QOverload<>::of(&ShortScrollBar::update));
    connect(this, &QScrollBar::rangeChanged, this, QOverload<>::of(&ShortScrollBar::update));
}

QSize ShortScrollBar::sizeHint() const
{
    if (orientation() == Qt::Vertical) return QSize(m_w, 0);
    return QSize(0, m_w);
}

QRect ShortScrollBar::grooveRect() const
{
    // 贴右边，并留出一点上下margin
    if (orientation() == Qt::Vertical) {
        const int topBottom = 8;
        return QRect(width() - m_w, topBottom, m_w, height() - 2*topBottom);
    } else {
        const int leftRight = 8;
        return QRect(leftRight, height() - m_w, width() - 2*leftRight, m_w);
    }
}

QRect ShortScrollBar::handleRect() const
{
    QRect g = grooveRect();
    if (maximum() <= minimum() || g.height() <= m_len) {
        // 没内容可滚动就不画
        return QRect();
    }

    if (orientation() == Qt::Vertical) {
        const int track = g.height() - m_len;
        const double t = double(value() - minimum()) / double(maximum() - minimum());
        const int y = g.top() + int(qRound(t * track));
        return QRect(g.left(), y, g.width(), m_len);
    } else {
        const int track = g.width() - m_len;
        const double t = double(value() - minimum()) / double(maximum() - minimum());
        const int x = g.left() + int(qRound(t * track));
        return QRect(x, g.top(), m_len, g.height());
    }
}

int ShortScrollBar::valueFromPos(int y) const
{
    QRect g = grooveRect();
    if (maximum() <= minimum() || g.height() <= m_len) return minimum();

    const int track = g.height() - m_len;
    int pos = y - g.top() - m_dragOffset;
    pos = qBound(0, pos, track);

    const double t = double(pos) / double(track);
    return minimum() + int(qRound(t * (maximum() - minimum())));
}

void ShortScrollBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // 轨道
    QRect g = grooveRect();
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0,0,0,0)); // 轨道透明
    // p.setBrush(QColor(0,0,0,15));
    p.drawRoundedRect(g, m_r, m_r);

    // 没有可滚动范围，就不画滑块
    if (maximum() <= minimum()) return;

    QRect h = handleRect();
    if (h.isNull()) return;

    QColor c = m_handle;
    if (m_dragging) c = m_pressed;
    else if (m_hovering) c = m_hover;

    p.setBrush(c);
    p.drawRoundedRect(h, m_r, m_r);
}

void ShortScrollBar::enterEvent(QEnterEvent *e)
{
    QScrollBar::enterEvent(e);
    m_hovering = true;
    update();
}

void ShortScrollBar::leaveEvent(QEvent *e)
{
    QScrollBar::leaveEvent(e);
    m_hovering = false;
    update();
}

void ShortScrollBar::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        QScrollBar::mousePressEvent(e);
        return;
    }

    QRect h = handleRect();
    if (h.contains(e->pos())) {
        m_dragging = true;
        m_dragOffset = e->pos().y() - h.top();
        e->accept();
        update();
        return;
    }

    // 点击轨道：把滑块中心跳过去
    m_dragging = true;
    m_dragOffset = m_len / 2;
    setValue(valueFromPos(e->pos().y()));
    e->accept();
    update();
}

void ShortScrollBar::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_dragging) {
        // hover 命中
        m_hovering = handleRect().contains(e->pos());
        update();
        QScrollBar::mouseMoveEvent(e);
        return;
    }

    setValue(valueFromPos(e->pos().y()));
    e->accept();
}

void ShortScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragging = false;
        m_dragOffset = 0;
        update();
        e->accept();
        return;
    }
    QScrollBar::mouseReleaseEvent(e);
}
