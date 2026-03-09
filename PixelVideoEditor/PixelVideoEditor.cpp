#include "PixelVideoEditor.h"
#include "ClickableSlider.h"

#include <QFileDialog>


PixelVideoEditor::PixelVideoEditor(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    mediaPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);

    mediaPlayer->setAudioOutput(audioOutput);

    videoWidget = new QVideoWidget(ui.centralWidget);
    mediaPlayer->setVideoOutput(videoWidget);

    videoWidget->installEventFilter(this);

    videoWidget->show();

    ui.PlayPauseButton->setEnabled(false);
    ui.PlayPauseButton->setFocusPolicy(Qt::NoFocus);
    ui.FullscreenButton->setFocusPolicy(Qt::NoFocus);

    clickTimer = new QTimer(this);
    clickTimer->setSingleShot(true);

    timeLabel = new QLabel(ui.centralWidget);
    timeLabel->setText("00:00.00 / 00:00.00");
    timeLabel->setAlignment(Qt::AlignCenter);
    timeLabel->setStyleSheet("color: black;");
    timeLabel->show();

    rewindButton = new QPushButton("<<", ui.centralWidget);
    forwardButton = new QPushButton(">>", ui.centralWidget);

    seekStepSpin = new QDoubleSpinBox(ui.centralWidget);
    seekStepSpin->setDecimals(2);
    seekStepSpin->setRange(0.01, 600.0);
    seekStepSpin->setValue(5.00);
    seekStepSpin->setSingleStep(0.01);

    connect(clickTimer, &QTimer::timeout, this, [this]()
        {
            if (!pendingSingleClick)
                return;

            pendingSingleClick = false;
            togglePlayPauseInternal();
        });

    connect(ui.FullscreenButton, &QPushButton::clicked,
        this, &PixelVideoEditor::toggleFullscreen);

    connect(ui.PlayPauseButton,
        &QPushButton::clicked,
        this,
        &PixelVideoEditor::togglePlayPauseInternal);

    connect(ui.actionOpen,
        &QAction::triggered,
        this,
        &PixelVideoEditor::openFile);

    connect(mediaPlayer, &QMediaPlayer::playbackStateChanged,
        this, &PixelVideoEditor::updatePlayButtonUI);

    connect(ui.SeekSlider, &QSlider::sliderPressed,
        this, &PixelVideoEditor::onSliderPressed);

    connect(ui.SeekSlider, &QSlider::valueChanged,
        this, &PixelVideoEditor::onSliderMoved);

    connect(ui.SeekSlider, &QSlider::sliderReleased,
        this, &PixelVideoEditor::onSliderReleased);

    connect(mediaPlayer, &QMediaPlayer::positionChanged,
        this, &PixelVideoEditor::updateSliderPosition);

    connect(mediaPlayer, &QMediaPlayer::durationChanged,
        this, &PixelVideoEditor::updateSliderRange);

    connect(rewindButton,
        &QPushButton::clicked,
        this,
        &PixelVideoEditor::seekBackward);

    connect(forwardButton,
        &QPushButton::clicked,
        this,
        &PixelVideoEditor::seekForward);

    connect(seekStepSpin,
        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this,
        [this](double v)
        {
            seekStepSeconds = v;
        });
}

PixelVideoEditor::~PixelVideoEditor()
{}

void PixelVideoEditor::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    QWidget* area = ui.centralWidget;

    int margin = 50;

    int availableWidth = area->width() - margin * 2;
    int availableHeight = area->height() - margin * 2;

    int videoWidth = availableWidth;
    int videoHeight = videoWidth * 9 / 16;

    if (videoHeight > availableHeight)
    {
        videoHeight = availableHeight;
        videoWidth = videoHeight * 16 / 9;
    }

    int x = margin;
    int y = margin;

    videoWidget->setGeometry(x, y, videoWidth, videoHeight);

    int controlSize = 30;
    int spacing = 10;

    int timeW = 120;

    int controlY = y + videoHeight + 10;

    int rewindW = controlSize;
    int forwardW = controlSize;
    int spinW = 70;

    // ▶/⏸ 버튼 (왼쪽)
    ui.PlayPauseButton->setGeometry(
        x,
        controlY,
        controlSize,
        controlSize
    );

    rewindButton->setGeometry(
        x + controlSize + spacing,
        controlY,
        rewindW,
        controlSize
    );

    seekStepSpin->setGeometry(
        x + controlSize + rewindW + spacing * 2,
        controlY,
        spinW,
        controlSize
    );

    forwardButton->setGeometry(
        x + controlSize + rewindW + spinW + spacing * 3,
        controlY,
        forwardW,
        controlSize
    );

    timeLabel->setGeometry(
        x + controlSize + rewindW + spinW + forwardW + spacing * 4,
        controlY,
        timeW,
        controlSize
    );

    // ⛶ 전체화면 버튼 (오른쪽)
    ui.FullscreenButton->setGeometry(
        x + videoWidth - controlSize,
        controlY,
        controlSize,
        controlSize
    );

    // 슬라이더 (가운데)
    ui.SeekSlider->setGeometry(
        x + controlSize + rewindW + spinW + forwardW + timeW + spacing * 5,
        controlY,
        videoWidth - (controlSize + rewindW + spinW + forwardW + timeW + spacing * 5) - (controlSize + spacing),
        controlSize
    );

    // (겹침 방지용)
    ui.PlayPauseButton->raise();
    ui.SeekSlider->raise();
    ui.FullscreenButton->raise();
}

void PixelVideoEditor::toggleFullscreen()
{
    isFullscreen = !isFullscreen;

    if (isFullscreen)
    {
        // 현재 창 상태 전체 저장
        previousWindowState = windowState();

        showFullScreen();

        if (ui.menuBar) ui.menuBar->hide();
        if (ui.statusBar) ui.statusBar->hide();
        if (ui.mainToolBar) ui.mainToolBar->hide();

        ui.PlayPauseButton->hide();
        ui.SeekSlider->hide();
        ui.FullscreenButton->hide();
        timeLabel->hide();

        videoWidget->setGeometry(0, 0, width(), height());
    }
    else
    {
        // 창 상태 복원
        setWindowState(previousWindowState);

        if (ui.menuBar) ui.menuBar->show();
        if (ui.statusBar) ui.statusBar->show();
        if (ui.mainToolBar) ui.mainToolBar->show();

        ui.PlayPauseButton->show();
        ui.SeekSlider->show();
        ui.FullscreenButton->show();
        timeLabel->show();

        resizeEvent(nullptr);
    }
}

void PixelVideoEditor::togglePlayPauseInternal()
{
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState)
    {
        mediaPlayer->pause();
    }
    else
    {
        mediaPlayer->play();
    }

    updatePlayButtonUI();
}

void PixelVideoEditor::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Space:
        togglePlayPauseInternal();
        return;

    case Qt::Key_F:
        toggleFullscreen();
        return;

    case Qt::Key_Escape:
        if (isFullscreen)
        {
            toggleFullscreen();
            return;
        }
        break;
    }

    QMainWindow::keyPressEvent(event);
}

bool PixelVideoEditor::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == videoWidget)
    {
        if (event->type() == QEvent::MouseButtonDblClick)
        {
            pendingSingleClick = false;
            clickTimer->stop();

            toggleFullscreen();
            return true;
        }

        if (event->type() == QEvent::MouseButtonPress)
        {
            auto* me = static_cast<QMouseEvent*>(event);

            if (me->button() == Qt::LeftButton)
            {
                pendingSingleClick = true;
                clickTimer->start(200);
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void PixelVideoEditor::updatePlayButtonUI()
{
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState)
        ui.PlayPauseButton->setText("⏸");
    else
        ui.PlayPauseButton->setText("▶");
}

void PixelVideoEditor::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "영상 열기",
        "",
        "Video Files (*.mp4 *.avi *.mkv)"
    );

    if (fileName.isEmpty())
        return;

    mediaPlayer->setSource(QUrl::fromLocalFile(fileName));
    mediaPlayer->play();

    ui.PlayPauseButton->setEnabled(true);
}

void PixelVideoEditor::updateSliderRange(qint64 duration)
{
    ui.SeekSlider->setRange(0, duration);
}

QString formatTime(qint64 ms)
{
    int minutes = ms / 60000;
    int seconds = (ms % 60000) / 1000;
    int centiseconds = (ms % 1000) / 10; // 소수점 두자리

    return QString("%1:%2.%3")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
        .arg(centiseconds, 2, 10, QChar('0'));
}

void PixelVideoEditor::updateSliderPosition(qint64 position)
{
    if (!ui.SeekSlider->isSliderDown())
    {
        ui.SeekSlider->setValue(position);
    }

    qint64 duration = mediaPlayer->duration();

    QString current = formatTime(position);
    QString total = formatTime(duration);

    timeLabel->setText(current + " / " + total);
}

void PixelVideoEditor::onSliderPressed()
{
    wasPlayingBeforeDrag =
        (mediaPlayer->playbackState() == QMediaPlayer::PlayingState);

    mediaPlayer->pause();
}

void PixelVideoEditor::onSliderMoved(int position)
{
    if (ui.SeekSlider->isSliderDown())
    {
        mediaPlayer->setPosition(position);
    }
}

void PixelVideoEditor::onSliderReleased()
{
    mediaPlayer->setPosition(ui.SeekSlider->value());

    if (wasPlayingBeforeDrag)
        mediaPlayer->play();
}

void PixelVideoEditor::seekForward()
{
    qint64 step = seekStepSeconds * 1000.0;
    qint64 newPos = mediaPlayer->position() + step;

    if (newPos > mediaPlayer->duration())
        newPos = mediaPlayer->duration();

    mediaPlayer->setPosition(newPos);
}

void PixelVideoEditor::seekBackward()
{
    qint64 step = seekStepSeconds * 1000.0;
    qint64 newPos = mediaPlayer->position() - step;

    if (newPos < 0)
        newPos = 0;

    mediaPlayer->setPosition(newPos);
}

