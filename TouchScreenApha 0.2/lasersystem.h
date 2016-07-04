#ifndef LASERSYSTEM_H
#define LASERSYSTEM_H

#include "laser.h"
#include "edfa.h"
#include <QSettings>

class LaserSystem: public QObject
{
    Q_OBJECT
public:
    explicit LaserSystem(QObject *parent = 0);
    ~LaserSystem();

    edfa *EDFA;
    Laser *SeedLaser;

    bool setUpComports();




    float getWavelength(bool update);
    float getWavelengthSetpoint(bool update);


    bool getEmission(bool updateEDFA, bool updateLaser);
    bool getLaserEmission(bool update);
    bool getEDFAEmission(bool update);

    float getPower(bool update);





public slots:
    void updateSystemState();
    void saveSettings();

    bool setEmission(bool state);
    bool setLaserEmission(bool state);
    bool setEDFAEmission(bool state);
    bool setWavelength(float wavelength);
    bool setPower(int power);

signals:
    void laserComportOpen(bool open);

    void laserEmission(bool emission);
    void EDFAEmission(bool emission);
    void systemEmission(bool emission);

    void laserWavelengthUpdate(float wavelength);
    void EDFAPowerUpdate(float power);


private:

    bool EDFAOn;
    bool laserOn;
    bool systemOn;

    bool laserComOpen;
    bool EDFAComOpen;


    float EDFAPower;

    float laserWavelength;
    float laserWavelengthSetpoint;

    QString laserCom;
    QString EDFACom;

    void loadSettings();

    QSettings *settings;







};

#endif // LASERSYSTEM_H
