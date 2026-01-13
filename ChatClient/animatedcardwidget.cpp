#include "animatedcardwidget.h"
#include <QPainterPath>

static qreal smootherstep(qreal x) {
    x = qBound<qreal>(0.0, x, 1.0);
    return x*x*x*(x*(x*6 - 15) + 10);
}

AnimatedCardWidget::AnimatedCardWidget(QWidget *parent)
    : QWidget(parent)
{
    sampleQQKeyframes();

    m_anim = new QVariantAnimation(this);
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(1.0);
    m_anim->setDuration(4000);                 // 接近你视频长度（约9秒）
    m_anim->setLoopCount(-1);
    m_anim->setEasingCurve(QEasingCurve::InOutSine);

    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v){
        m_phase = v.toReal();
        update();
    });
    m_anim->start();
}

void AnimatedCardWidget::sampleQQKeyframes()
{
    // 4帧真实QQ截图取样得到的近似颜色
    // 顺序按你发图顺序：1->2->3->4->回到1（闭环）
    m_keys = {
              {0.00, QColor("#FFFFFF"), QColor("#FBF6FF"), QColor("#BBE2FC"), QColor("#C6DCF4"),
               QColor("#FFFAFF"), QColor("#DEF0FF")},

              {0.25, QColor("#F3F2FF"), QColor("#E0F4FF"), QColor("#C1E1FC"), QColor("#CEE0F4"),
               QColor("#EDF0FF"), QColor("#E4F6FF")},

              {0.50, QColor("#DAF1FF"), QColor("#DDF2FF"), QColor("#D4E6FC"), QColor("#C0DCF4"),
               QColor("#F1F1FF"), QColor("#EEF0FF")},

              {0.75, QColor("#F0F1FF"), QColor("#DAF2FF"), QColor("#BEE4FC"), QColor("#D0DFF4"),
               QColor("#DBF1FF"), QColor("#EBF0FF")},

              // 闭环
              {1.00, QColor("#FFFFFF"), QColor("#FBF6FF"), QColor("#BBE2FC"), QColor("#C6DCF4"),
               QColor("#FFFAFF"), QColor("#DEF0FF")},
              };
}

QColor AnimatedCardWidget::lerpColor(const QColor& a, const QColor& b, qreal k)
{
    auto lerp = [&](int x, int y) { return int(x + (y - x) * k + 0.5); };
    return QColor(lerp(a.red(), b.red()),
                  lerp(a.green(), b.green()),
                  lerp(a.blue(), b.blue()),
                  lerp(a.alpha(), b.alpha()));
}

void AnimatedCardWidget::renderBilinear(QImage &img, const QColor& tl, const QColor& tr,
                                        const QColor& bl, const QColor& br)
{
    const int w = img.width();
    const int h = img.height();
    if (w <= 1 || h <= 1) return;

    for (int y = 0; y < h; ++y) {
        const qreal fy = qreal(y) / qreal(h - 1);
        const QColor left  = lerpColor(tl, bl, fy);
        const QColor right = lerpColor(tr, br, fy);

        QRgb *line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < w; ++x) {
            const qreal fx = qreal(x) / qreal(w - 1);
            const QColor c = lerpColor(left, right, fx);
            line[x] = c.rgba();
        }
    }
}

void AnimatedCardWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // 圆角路径
    QRectF r = rect();
    r.adjust(m_borderWidth/2.0, m_borderWidth/2.0,
             -m_borderWidth/2.0, -m_borderWidth/2.0);

    QPainterPath path;
    path.addRoundedRect(r, m_radius, m_radius);

    // 选出当前关键帧段
    if (m_keys.size() < 2) return;

    qreal t = m_phase;
    int idx = 0;
    while (idx + 1 < m_keys.size() && !(t >= m_keys[idx].t && t <= m_keys[idx+1].t)) {
        idx++;
    }
    if (idx + 1 >= m_keys.size()) idx = m_keys.size() - 2;

    const auto &a = m_keys[idx];
    const auto &b = m_keys[idx + 1];
     const qreal span = (b.t - a.t);
    qreal k = (span <= 0.0) ? 0.0 : (t - a.t) / span;
    k = smootherstep(k);

    const QColor tl = lerpColor(a.tl, b.tl, k);
    const QColor tr = lerpColor(a.tr, b.tr, k);
    const QColor bl = lerpColor(a.bl, b.bl, k);
    const QColor br = lerpColor(a.br, b.br, k);
    const QColor glowL = lerpColor(a.glowL, b.glowL, k);
    const QColor glowR = lerpColor(a.glowR, b.glowR, k);

    // 用小图缓存（更柔和），再平滑放大铺满
    QSize smallSize(qMax(1, width()/4), qMax(1, height()/4));   // 320x460 -> 80x115
    if (m_cached.size() != smallSize || m_cached.format() != QImage::Format_ARGB32_Premultiplied) {
        m_cached = QImage(smallSize, QImage::Format_ARGB32_Premultiplied);
    }

    // 四角插值生成背景（画到小图上）
    renderBilinear(m_cached, tl, tr, bl, br);

    // 裁剪到圆角后绘制（把小图平滑放大到整个 card）
    p.setClipPath(path);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.drawImage(rect(), m_cached);

    // 叠加左右柔光（保持裁剪，才能不溢出圆角）
    auto drawGlow = [&](const QPointF &center, const QColor &base, qreal radius, int alphaCenter) {
        QRadialGradient rg(center, radius);

        QColor c0 = base; c0.setAlpha(alphaCenter); // 中心更亮
        QColor c1 = base; c1.setAlpha(0);           // 外圈透明

        rg.setColorAt(0.0, c0);
        rg.setColorAt(1.0, c1);

        p.fillRect(rect(), rg);
    };

    // 左侧偏粉紫柔光（位置/半径按 320x460 调过）
    drawGlow(QPointF(width()*0.20, height()*0.45), glowL, width()*1.05, 170);

    // 右侧偏蓝柔光
    drawGlow(QPointF(width()*0.88, height()*0.22), glowR, width()*1.10, 180);
    // 画边框
    p.setClipping(false);
    QPen pen(QColor(255, 255, 255, 40), 1);   // 白色很淡的线
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);

}
