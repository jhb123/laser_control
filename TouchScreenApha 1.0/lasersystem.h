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

    bool setupComports();

    bool getEdfaComOpen(bool shouldUpdate);
    bool getLaserComOpen(bool shouldUpdate);

    bool laserRecieveCheck();
    bool edfaRecieveCheck();

    float getLaserWavelengthReadout(bool shouldUpdate);
    float getLaserWavelengthSetpoint(bool shouldUpdate);
    bool setLaserWavelength(float wavelength);

    bool getSystemEmission(bool shouldUpdateEdfa, bool shouldUpdateLaser);
    bool setSystemEmission(bool isOn);

    bool getLaserEmission(bool shouldUpdate);
    bool setLaserEmission(bool isOn);

    bool getEdfaEmission(bool shouldUpdate);
    bool setEdfaEmission(bool isOn);

    float getEdfaPowerReadout(bool shouldUpdate);
    float getEdfaPowerSetpoint(bool update);
    bool setEdfaPower(float power);

    void saveSettings(float wavelength, float power);

    bool getStable(bool shouldUpdate);

public slots:    
    void updateSystemState(bool shouldUpdateAll);

    void closeLaserComport();
    void closeEdfaComport();

private slots:
    void updateLaserComport(QString portName, bool isOpen);
    void updateEdfaComport(bool isOpen);

signals:
    void laserEmissionSignal(bool isOn);
    void edfaEmissionSignal(bool isOn);
    void systemEmissionSignal(bool isOn);

    void laserWavelengthReadoutSignal(float wavelength);
    void edfaPowerReadoutSignal(float power);

    void laserComSignal(bool isOpen);
    void edfaComSignal(bool isOpen);

    void stableCheckSignal(bool isStable);

private:
    QSettings *settings;

    edfa *Edfa;
    Laser *SeedLaser;

    void loadSettings();

    bool isEdfaOn_;
    bool isLaserOn_;
    bool isSystemOn_;

    bool isLaserComOpen_;
    bool isEdfaComOpen_;

    int comFails_;
    int updateCounter_;


    float edfaPower_;
    float edfaPowerSetpoint_;

    float laserWavelength_;
    float laserWavelengthSetpoint_;

    QString laserComName_;
    QString edfaComName_;

};

#endif // LASERSYSTEM_H
