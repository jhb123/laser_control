#ifndef CAMERA_H
#define CAMERA_H

#include <QCameraInfo>
#include <QCamera>
#include <QCameraViewfinder>

class Camera
{
public:
    Camera(QVideoWidget*);
    void start();
    void stop();

private:
    QCamera *capture;
    QCameraViewfinder *viewfinder;

};

#endif // CAMERA_H
