/* Laser Class
 * This class acts a connecting class between the interbus and the UI to reduce the amount of code in the UI source code
 * */

#ifndef LASER_H
#define LASER_H

#include "Interbus/ibhandler.h"
#include <QDebug>

class Laser: public QObject
{
    Q_OBJECT
public:
    explicit Laser(QObject *parent = 0);
    ~Laser();

    IBHandler *interbus;

    bool getEmission(); //Function to check emission status. Returns true if emission is on.
    bool setEmission(bool isOn);

    bool setAcknowlage(bool isOn);

    bool setWavelength(float wavelength);
    float getWavelength();
    float getWavelengthSetpoint();

    //Method to check if the laser is comunicating with serial.
    bool checkComs();

    bool setUpPort(QString name);
    void closePort();

    //Power mode, true for power mode, false for current.
    bool setPowerMode(bool isPowerMode);
    //Setpoint for power mA for current, uW for power.
    bool setPowerSetpoint(ushort power);
    //False disables piezo modulation, true enables.
    bool setPiezoModulation(bool isOn);
    bool setRINSuppression(bool isOn);
    //Set between temp or wavelength mode. true is wavelength false is temp.
    bool setTWMode(bool isWavelengthMode);
    bool setTemperatureCompensation(bool isOn);

private:
    bool regStatusCheck(RegGetStatusTypes regStatus); //Overloaded status check method
    bool regStatusCheck(RegSetStatusTypes regStatus);

    bool boolBuffer(quint8 writeReg, bool hasState);

    ushort getIndex16(QString inString, int index);

    const quint8 devAddress_ = 10;

    ushort wavelengthOffset_ = 1559.000; //System specific wavelength ofset.


};


#endif // LASER_H
