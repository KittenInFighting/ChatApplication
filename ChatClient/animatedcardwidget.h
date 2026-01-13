#ifndef ANIMATEDCARDWIDGET_H
#define ANIMATEDCARDWIDGET_H

#include <QWidget>
#include <QVariantAnimation>
#include <QPainter>
#include <QtMath>
#include <QImage>
#include <QColor>
#include <QVector>

class AnimatedCardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AnimatedCardWidget(QWidget *parent = nullptr);

    void setRadius(int r) { m_radius = r; update(); }
    void setBorderWidth(int w) { m_borderWidth = w; update(); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct KeyFrame {
        qreal t;                 // 0~1
        QColor tl, tr, bl, br;   // 四角
        QColor glowL, glowR;     // 左/右柔光色
    };

    static QColor lerpColor(const QColor& a, const QColor& b, qreal k);
    static void renderBilinear(QImage &img, const QColor& tl, const QColor& tr,
                               const QColor& bl, const QColor& br);

    void sampleQQKeyframes(); // 你可以在这里继续微调关键帧

private:
    qreal m_phase = 0.0;
    int m_radius = 10;
    int m_borderWidth = 2;

    QVariantAnimation *m_anim = nullptr;
    QVector<KeyFrame> m_keys;
    QImage m_cached;  // 缓存当前尺寸的背景图
};

#endif // ANIMATEDCARDWIDGET_H
