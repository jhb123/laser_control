/* Header file for touchscreen.
 **/

#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include <QMainWindow>
#include "lasersystem.h"
#include "camera.h"
#include <QDoubleSpinBox>
#include <QPushButton>

namespace Ui {
class TouchScreen;
}

class TouchScreen : public QMainWindow
{
    Q_OBJECT

public:
    explicit TouchScreen(QWidget *parent = 0);
    ~TouchScreen();

private slots:
    void systemEmissionUpdate(bool isEmissionOn);
    void edfaEmissionUpdate(bool isEmissionOn);
    void laserEmissionUpdate(bool isEmissionOn);

    void stableLightUpdate(bool isStable);

    void systemButton();
    void edfaButton();
    void laserButton();

    void updateEdfaPowerReadout(float power);
    void setEdfaPower();

    void updateLaserWavelengthReadout(float wavelength);
    void setLaserWavelength();

    void edfaComportState(bool isOpen);
    void laserComportState(bool isOpen);

    void setupComports();

    void saveSettings();
    void loadDefaults();

    void updateSystemTimer();

    void keyboardButtonPressed();
    void showKeyboard(QWidget *oldWidget, QWidget *newWidget);

    void tabChanged(int tabIndex);

private:
    float addNumber(float previousValue, QString numberAppend);
    float deleteNumber(float previousValue);

    Ui::TouchScreen *ui;
    LaserSystem *MainSystem;
    QTimer *UpdateTimer;
    Camera *MotCamera;

    float defaultWavelength_;
    float defaultPower_;

    QDoubleSpinBox *pointerToActiveSpinBox_;

    QList<QPushButton*> buttonList_;

    int timeBetweenMeasurements_ = 1000;

    int currentTab_;

};

#endif // TOUCHSCREEN_H
