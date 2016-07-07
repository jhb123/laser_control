#include "camera.h"

Camera::Camera(QVideoWidget *display)
{

    capture = new QCamera();
    viewfinder = new QCameraViewfinder();
    capture->setViewfinder(display);
}

void Camera::start(){
    capture->start();
}
void Camera::stop(){
    capture->stop();
}
