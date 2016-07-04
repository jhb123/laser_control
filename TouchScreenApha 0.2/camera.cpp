#include "camera.h"

camera::camera(QVideoWidget *display)
{
    capture = new QCamera();
    viewfinder = new QCameraViewfinder();
    capture->setViewfinder(display);
}

void camera::start(){
    capture->start();
}
void camera::stop(){
    capture->stop();
}
