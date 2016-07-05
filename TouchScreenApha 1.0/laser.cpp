#include "Laser.h"


Laser::Laser(QObject *parent) : QObject(parent)
{
    interbus = new IBHandler(this);

}

Laser::~Laser(){
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

bool Laser::setUpPort(QString name){
    return interbus -> openComport(name);
}

void Laser::closePort(){
    interbus -> closeComport();
}

bool Laser::checkComs(){
    QByteArray readData;
    return regStatusCheck(interbus->readRegister(devAddress_, 0x30, readData));
}

//Method to check the emission state of the laser.
bool Laser::getEmission(){
    QByteArray readData;
    if(regStatusCheck(interbus->readRegister(devAddress_, 0x30, readData))){
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
        return wavelengthOffset_ + (getIndex16(interbus->GetHexString(readData, 0),12)/1000.0);
    }else{
        return 0.0; //Rubbish way of debugging error read.
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

ushort Laser::getIndex16(QString inString, int index){
    QStringList indexList = inString.split(" ");
    QString outString;
    if(index <= ((indexList.length()-1)*2)){
        outString = indexList[(index*2) + 1] + indexList[(index*2)];
        outString.replace("h", "");

    }else{
        outString = "FFFF";
    }
    bool ok;
    ushort outInt = outString.toUShort(&ok, 16);
    return outInt;

}

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
