#include "ClickableSlider.h"

ClickableSlider::ClickableSlider(QWidget* parent)
    : QSlider(parent)
{
    // UI에서 orientation 지정해도 되지만,
    // 안전하게 기본을 수평으로 고정
    setOrientation(Qt::Horizontal);
}

void ClickableSlider::mousePressEvent(QMouseEvent* event)
{
    if (orientation() == Qt::Horizontal)
    {
        int value = minimum() + ((maximum() - minimum()) * event->pos().x()) / width();
        setValue(value);

        // 클릭하자마자 이동 + 기존 로직(sliderMoved 연결) 재사용
        emit sliderMoved(value);
        emit sliderPressed();
        emit sliderReleased();
    }

    QSlider::mousePressEvent(event);
}