/*
 * created 4/7/16 by jhb
*/

#ifndef CAMERA_H
#define CAMERA_H

#include <QCameraInfo>
#include <QCamera>
#include <QCameraViewfinder>

class camera
{
public:
    camera(QVideoWidget*);
    void start();
    void stop();

private:
    QCamera *capture;
    QCameraViewfinder *viewfinder;

};


#endif // CAMERA_H
