#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PixelVideoEditor.h"

#include <opencv2/opencv.hpp>
#include <QKeyEvent>

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QTimer>
#include <QLabel>

class PixelVideoEditor : public QMainWindow
{
    Q_OBJECT

public:
    PixelVideoEditor(QWidget *parent = nullptr);
    ~PixelVideoEditor();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    Ui::PixelVideoEditorClass ui;
    
    cv::VideoCapture cap;
    QMediaPlayer* mediaPlayer = nullptr;
    QAudioOutput* audioOutput = nullptr;
    QVideoWidget* videoWidget = nullptr;
    QTimer* clickTimer;
    QLabel* timeLabel;
    
    cv::Mat currentFrame;
    int timerInterval = 30;
    bool isFullscreen = false;
    bool wasPlayingBeforeDrag = false;
    bool pendingSingleClick = false;


private slots:
    void togglePlayPauseInternal();
    void updatePlayButtonUI();
    void toggleFullscreen();

    void updateSliderPosition(qint64 position);
    void updateSliderRange(qint64 duration);
    void onSliderPressed();
    void onSliderMoved(int position);
    void onSliderReleased();

    void openFile();
};

