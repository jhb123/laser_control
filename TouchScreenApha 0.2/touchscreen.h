#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include <QDialog>
#include "lasersystem.h"
#include "camera.h"
#include <QTimer>


namespace Ui {
class TouchScreen;
}

class TouchScreen : public QDialog
{
    Q_OBJECT

private slots:
    void EDFAEmissionLight(bool emissionOn);
    void laserEmissionLight(bool emissionOn);
    void mainButtonState(bool emissionOn);

    void mainButton();
    void EDFAOnOff();
    void laserOnOff();

    void updatePower(float power);
    void updateWavelength(float wavelength);
    void setNewWavelength();
    void tabTest();

public:
    explicit TouchScreen(QWidget *parent = 0);
    ~TouchScreen();


private:
    Ui::TouchScreen *ui;

    //System Object
    LaserSystem *mainSystem;

    QTimer *wavelengthTimer;
    QTimer *timer;
    camera *cam;

     quint16 prevWavelength;


     //Slider
     bool sliderMoving = false;


};

#endif // TOUCHSCREEN_H
