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
     //Need some way to throw errors
    /*
        case GetBusy:           ui->pteStatus->appendPlainText("Read busy"); break;  //Prints error to pte. Pte name may need changing
        case GetNacked:         ui->pteStatus->appendPlainText("Read nacked"); break;
        case GetCRCErr:         ui->pteStatus->appendPlainText("Read CRC error"); break;
        case GetTimeout:        ui->pteStatus->appendPlainText("Read timeout error"); break;
        case GetComError:       ui->pteStatus->appendPlainText("Read communication error"); break;
        case GetPortClosed:     ui->pteStatus->appendPlainText("Read port closed"); break;
        case GetPortNotFound:   ui->pteStatus->appendPlainText("Read port not found"); break;
        */
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
        /*
        case GetBusy:           ui->pteStatus->appendPlainText("Read busy"); break;  //Prints error to pte. Pte name may need changing
        case GetNacked:         ui->pteStatus->appendPlainText("Read nacked"); break;
        case GetCRCErr:         ui->pteStatus->appendPlainText("Read CRC error"); break;
        case GetTimeout:        ui->pteStatus->appendPlainText("Read timeout error"); break;
        case GetComError:       ui->pteStatus->appendPlainText("Read communication error"); break;
        case GetPortClosed:     ui->pteStatus->appendPlainText("Read port closed"); break;
        case GetPortNotFound:   ui->pteStatus->appendPlainText("Read port not found"); break;
        */
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
    return regStatusCheck(interbus->readRegister(devAddress, 0x30, readData));
}

//Method to check the emission state of the laser.
bool Laser::getEmission(){
    QByteArray readData;
    if(regStatusCheck(interbus->readRegister(devAddress, 0x30, readData))){
        quint8 emissionData = interbus -> GetInt8(readData, 0);
        if(emissionData == 1){
            return true;
        }
        else if(emissionData == 0){
            return false;
        }

    }
}

//May need to give some error here if neither of these two cases occur. At this time, the previous emission state would be returned.

bool Laser::setEmission(bool emission){
    return boolBuffer(0x30, emission);
}

bool Laser::setAcknowlage(bool acknowlageState){
    return boolBuffer(0x36, acknowlageState);
}

//Returns the actual wavelength.
float Laser::getWavelength(){
    QByteArray readData;
    if(regStatusCheck(interbus->readRegister(devAddress, 0x10, readData))){
        return wavelengthOffset + (getIndex16(interbus->GetHexString(readData, 0),12)/1000.0);
    }else{
        return 9999.999; //Rubbish way of debugging error read.
    }
}

//Returns the fibrelaser setpoint.
float Laser::getWavelengthSetpoint(){
    QByteArray readData;
    if(regStatusCheck(interbus->readRegister(devAddress, 0x25, readData))){
        return wavelengthOffset + (interbus->GetInt16(readData, 0)/1000.0);
    }else{
        return 9999.999;
    }
}
//Sets the fibrelaser setpoint to a value which corresponds to the inputted wavelength.
bool Laser::setWavelength(float wavelength){
    QByteArray writeData;
    ushort shortValue = (wavelength - 1559.000)*1000;
    interbus->SetInt16(writeData,shortValue,0);

    return regStatusCheck(interbus->writeRegister(devAddress, 0x25, writeData));
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

bool Laser::boolBuffer(quint8 writeReg, bool state){
    QByteArray writeData;
    if(state){
        interbus->SetInt8(writeData, 0x01, 0); //Sets buffer.
    }
    else{
        interbus->SetInt8(writeData, 0x00, 0); //Sets buffer.
        qDebug() << "Set laser final off";
    }

    return regStatusCheck(interbus->writeRegister(devAddress, writeReg, writeData));
}


bool Laser::setPowerMode(bool mode){
    return boolBuffer(0x31, mode);
}

bool Laser::setPowerSetpoint(ushort power){
    QByteArray writeData;
    interbus->SetInt16(writeData,power,0);

    return regStatusCheck(interbus->writeRegister(devAddress, 0x23, writeData));
}

bool Laser::setPiezoModulation(bool mode){
    return boolBuffer(0x32, mode);
}

bool Laser::setRINSuppression(bool mode){
    return boolBuffer(0x33, mode);
}

bool Laser::setTWMode(bool mode){
    return boolBuffer(0x34, mode);
}

bool Laser::setTemperatureCompensation(bool mode){
    return boolBuffer(0x35, mode);
}
