/* Laser implimentation file
 *
 * This code is for the Kohoras Basik E15 laser. It makes use of the interbus provided by NKT photonics.
 * The interbus provides the necessary functions for communicating with any NKT laser system. This class
 * uses those functions to provide communication with the E15.
 *
 * For information on the register addresses of the laser and command structure, consult the SDK, avaiable at
 * http://www.nktphotonics.com/support/software-drivers/
 * */

#include "Laser.h"

Laser::Laser(QObject *parent) : QObject(parent)
{
    interbus = new IBHandler(this); //Create an interbus object.
}

Laser::~Laser(){
        qDebug()<<"Laser destructor";
        closePort();
}

//Private functions to check the status of read and write
bool Laser::regStatusCheck(RegSetStatusTypes regStatus){
    bool statusSuccess = false;

    //Switch for possible errors.
    switch (regStatus)
        {
        case GetSuccess:
            statusSuccess = true;
            break;
        default:
            statusSuccess = false;
        break;

        }
    return statusSuccess;
}

bool Laser::regStatusCheck(RegGetStatusTypes regStatus){
    bool statusSuccess = false;
    switch (regStatus)
        {
        case GetSuccess:
            statusSuccess = true;
            break;
    default:
        statusSuccess = false;
    break;
        }
    return statusSuccess;
}

//Opens the comport.
bool Laser::setUpPort(QString name){
    return interbus -> openComport(name);
}

//closes the connected comport.
void Laser::closePort(){
    interbus -> closeComport();
}

//Reads the emission register and checks to see if there is a reply, coms are working.
bool Laser::checkComs(){
    QByteArray readData;
    return regStatusCheck(interbus->readRegister(devAddress_, 0x30, readData));
}

//Method to check the emission state of the laser.
bool Laser::getEmission(){
    QByteArray readData;
    if(regStatusCheck(interbus->readRegister(devAddress_, 0x30, readData))){ //0x30 is the emission register.
        quint8 emissionData = interbus -> GetInt8(readData, 0);
        if(emissionData == 1){
            return true;
        }
        else if(emissionData == 0){
            return false;
        }

    }
    return false;
}

bool Laser::setEmission(bool isOn){
    return boolBuffer(0x30, isOn);
}

bool Laser::setAcknowlage(bool acknowlageState){
    return boolBuffer(0x36, acknowlageState);
}
//Returns the actual wavelength.
float Laser::getWavelength(){
    QByteArray readData;
    if(regStatusCheck(interbus->readRegister(devAddress_, 0x10, readData))){
        return wavelengthOffset_ + (getIndex16(interbus->GetHexString(readData, 0),12)/1000.0); //Wavelength needs to be converted (see SDK).
    }else{
        return 0.0; //Returns 0 if read failed
    }
}
//Returns the fibrelaser setpoint.
float Laser::getWavelengthSetpoint(){
    QByteArray readData;
    if(regStatusCheck(interbus->readRegister(devAddress_, 0x25, readData))){
        return wavelengthOffset_ + (interbus->GetInt16(readData, 0)/1000.0);
    }else{
        return 0.0;
    }
}
//Sets the fibrelaser setpoint to a value which corresponds to the inputted wavelength.
bool Laser::setWavelength(float wavelength){
    QByteArray writeData;
    ushort shortValue = (wavelength - 1559.000)*1000;
    interbus->SetInt16(writeData,shortValue,0);
    return regStatusCheck(interbus->writeRegister(devAddress_, 0x25, writeData));
}
//Wavelength information is given in a hex string which represents an array of ushort values. This function
//converts a certain index to a ushort value. Each element is also backwards so it needs to be flipped.
ushort Laser::getIndex16(QString inString, int index){
    QStringList indexList = inString.split(" ");
    QString outString;
    if(index <= ((indexList.length()-1)*2)){
        outString = indexList[(index*2) + 1] + indexList[(index*2)];
        outString.replace("h", "");

    }else{
        outString = "FFFF"; //Error catch.
    }
    bool ok;
    ushort outInt = outString.toUShort(&ok, 16);
    return outInt;
}

//Function to make sending bool states to the laser easier. As true and false needs to be converted to 0x01 and 0x00.
bool Laser::boolBuffer(quint8 writeReg, bool hasState){
    QByteArray writeData;
    if(hasState){
        interbus->SetInt8(writeData, 0x01, 0); //Sets buffer.
    }
    else{
        interbus->SetInt8(writeData, 0x00, 0); //Sets buffer.
        qDebug() << "Set laser final off";
    }
    return regStatusCheck(interbus->writeRegister(devAddress_, writeReg, writeData));
}

//These methods are only used for initial setup and change settings which are unused by the UI.
//They are set initially to make sure the laser is in the correct operating mode.
bool Laser::setPowerMode(bool isPowerMode){
    return boolBuffer(0x31, isPowerMode);
}

bool Laser::setPowerSetpoint(ushort power){
    QByteArray writeData;
    interbus->SetInt16(writeData,power,0);
    return regStatusCheck(interbus->writeRegister(devAddress_, 0x23, writeData));
}

bool Laser::setPiezoModulation(bool isOn){
    return boolBuffer(0x32, isOn);
}

bool Laser::setRINSuppression(bool isOn){
    return boolBuffer(0x33, isOn);
}

bool Laser::setTWMode(bool isWavelengthMode){
    return boolBuffer(0x34, isWavelengthMode);
}

bool Laser::setTemperatureCompensation(bool isOn){
    return boolBuffer(0x35, isOn);
}
