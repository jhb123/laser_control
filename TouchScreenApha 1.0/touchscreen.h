#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include <QDialog>
#include "lasersystem.h"
#include <QTimer>

namespace Ui {
class TouchScreen;
}

class TouchScreen : public QDialog
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


private:
    Ui::TouchScreen *ui;
    LaserSystem *MainSystem;
    QTimer *UpdateTimer;

    float defaultWavelength_;
    float defaultPower_;


};

#endif // TOUCHSCREEN_H
