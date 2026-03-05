#include "PixelVideoEditor.h"
#include <QtWidgets/QApplication>
#include <opencv2/opencv.hpp>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    PixelVideoEditor window;
    window.show();
    return app.exec();
}
