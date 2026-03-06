#include "ClickableSlider.h"
#include <QStyle>
#include <QStyleOptionSlider>
#include <QToolTip>

ClickableSlider::ClickableSlider(QWidget* parent)
    : QSlider(parent)
{
    setOrientation(Qt::Horizontal);
    setMouseTracking(true);
}

void ClickableSlider::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        QSlider::mousePressEvent(event);
        return;
    }

    QStyleOptionSlider opt;
    initStyleOption(&opt);

    // 핸들 영역 얻기
    const QRect handleRect = style()->subControlRect(
        QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this
    );

    // 1) 핸들을 눌렀으면: 기본 동작(드래그) 그대로
    if (handleRect.contains(event->pos()))
    {
        QSlider::mousePressEvent(event);
        return;
    }

    // 2) 바(그루브) 눌렀으면: 클릭 위치로 점프
    const int x = event->pos().x();
    const int w = width();

    const int value = QStyle::sliderValueFromPosition(
        minimum(), maximum(), x, w
    );

    setValue(value);

    // 3) 이후 드래그도 가능하게 기본 동작으로 넘김
    QSlider::mousePressEvent(event);
}

void ClickableSlider::mouseMoveEvent(QMouseEvent* event)
{
    if (orientation() == Qt::Horizontal)
    {
        int value = QStyle::sliderValueFromPosition(
            minimum(),
            maximum(),
            event->pos().x(),
            width()
        );

        int minutes = value / 60000;
        int seconds = (value % 60000) / 1000;
        int centiseconds = (value % 1000) / 10;

        QString time = QString("%1:%2.%3")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'))
            .arg(centiseconds, 2, 10, QChar('0'));

        QToolTip::showText(event->globalPosition().toPoint(), time, this);
    }

    QSlider::mouseMoveEvent(event);
}