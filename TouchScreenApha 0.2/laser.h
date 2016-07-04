/* Laser Class
 * This class acts a connecting class between the interbus and the UI to reduce the amount of code in the UI source code
 *
 * First Created 29/06/2016 -SPG
 *
 * Chnage Log:
 *
 *
 * */

#ifndef LASER_H
#define LASER_H

#include "Interbus/ibhandler.h"

class Laser: public QObject
{
    Q_OBJECT
public:
    explicit Laser(QObject *parent = 0);
    ~Laser();

    IBHandler *interbus;

    bool getEmission(); //Function to check emission status. Returns true if emission is on.
    bool setEmission(bool emission);

    bool setAcknowlage(bool acknowlageState);

    bool setWavelength(float wavelength);
    float getWavelength();
    float getWavelengthSetpoint();

    //Method to check if the laser is comunicating with serial.
    bool checkComs();

    void updateLaserState();

    bool setUpPort(QString name);
    void closePort();

    //Other settings.

    //Power mode, true for power mode, false for current.
    bool setPowerMode(bool mode);
    //Setpoint for power mA for current, uW for power.
    bool setPowerSetpoint(ushort power);
    //False disables piezo modulation, true enables.
    bool setPiezoModulation(bool mode);
    bool setRINSuppression(bool mode);
    //Set between temp or wavelength mode. true is wavelength false is temp.
    bool setTWMode(bool mode);
    bool setTemperatureCompensation(bool mode);


public slots:

signals:
    void emmissionChanged(bool emission);

private slots:


private:

    bool regStatusCheck(RegGetStatusTypes regStatus); //Overloaded status check method
    bool regStatusCheck(RegSetStatusTypes regStatus);

    bool boolBuffer(quint8 writeReg, bool state);

    ushort getIndex16(QString inString, int index);

    bool emissionOn;
    quint16 wavelength;

    const quint8 devAddress = 10;

    ushort wavelengthOffset = 1559.000; //System specific wavelength ofset.

};


#endif // LASER_H
