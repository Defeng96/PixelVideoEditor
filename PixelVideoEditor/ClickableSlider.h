#pragma once
#include <QSlider>
#include <QMouseEvent>

class ClickableSlider : public QSlider
{
    Q_OBJECT
public:
    explicit ClickableSlider(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
};