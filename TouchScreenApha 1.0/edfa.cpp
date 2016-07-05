/*
 * Todos for this class:
 * -Impliment a more fail safe way of reading data.
 * -Create method to read the replies of commands to ensure they have been read.
 *
 */

#include "edfa.h"
#include<QDebug>
#include<QTimer>
#include <QThread>
#include<QTime>
#include<QCoreApplication>

edfa::edfa(QObject *parent) : QObject(parent)
{
    serial = new QSerialPort(this);
}

edfa::~edfa()
{
    serial->close();
}
 //Changed this a bit to make it more error proof. SPG 29/06

bool edfa::setUpPort(QString name){
    serial -> close();
    serial->setPortName(name);
    if(serial->open(QIODevice::ReadWrite)){
        serial->setBaudRate(QSerialPort::Baud9600);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        emit EDFAComportOpen(serial-> isOpen());
    return true;
    }
    closeComport();
    return false;
}

void edfa::closeComport(){
    serial -> close();
    emit EDFAComportOpen(serial -> isOpen());
}

void edfa::setPower(float value){
    qDebug() << "Setting power to " << value;
    QString valueQString;
    valueQString.setNum(value);
    QString command = "APC " + valueQString +"\r\n"; //concatinate the string
    QByteArray commandByteArray = command.toLatin1(); //convert string to byte array
    writeSerial(commandByteArray);
    readSerial();
}

float edfa::getPower(){
    QString message;
    writeSerial("APC\r\n");
    message = readSerial();
    qDebug() << "get EDFA power message " << message;
    if (message.contains("APC ",Qt::CaseInsensitive)){
        message = stripString(message, "APC");
        qDebug() << "message contains APC";
        qDebug() << "New message " << message;
    }
    return message.toFloat();
}

bool edfa::replyCheck(){
    qDebug() << "In reply check";
    qDebug() << "In reply check ready pass";
    writeSerial("giberish\r\n"); //send some giberish to get FWC response.
    QString message = readSerial();
    qDebug() << message;
    if (message.contains("FWC",Qt::CaseInsensitive)){
        return true;
    }
    return false;
}

void edfa::setEmission(bool isOn){
    qDebug() << "Set emission in EDFA object " << isOn;
    if(isOn){

        EDFAOn();
    }
    else{
        EDFAOff();
    }

    qDebug() << readSerial(); //Needed for some reason. Think it is to do with delays.

}

void edfa::EDFAOn(){
    qDebug() << "Final EDFA write on";
    writeSerial("OM 2\r\n");
    readSerial();
}

void edfa::EDFAOff(){
    qDebug() << "Final EDFA write off";
    writeSerial("OM 0\r\n");
    readSerial();
}

bool edfa::getEmission(){
    bool emission = false;
    QString message;
    writeSerial("OM\r\n");
    message = readSerial();
    qDebug() << "get EDFA emission message " << message;
    if (message.contains("OM ",Qt::CaseInsensitive)){
        message = stripString(message, "OM");
        qDebug() << "message contains OM";
        qDebug() << "New message " << message;
    }
    if(message == "O"){
        emission = false;
    }
    if(message == "2"){
        emission = true;
    }

return emission;
}

float edfa::getPh1(){
    writeSerial("Ph 1\r\n");
    QString message=readSerial();
    if (message.contains("PDM1: ",Qt::CaseInsensitive)){
        message = stripString(message, "PDM1:");
    }
    return message.toFloat();
}

float edfa::getPh2(){
    writeSerial("Ph 2\r\n");
    QString message=readSerial();
    if (message.contains("PDM2: ",Qt::CaseInsensitive)){
        message = stripString(message, "PDM2:");
    }
    qDebug()<<"PH2 final message " << message;
    return message.toFloat();
}

void edfa::delay( int millisecondsToWait )
{
    QTime dieTime = QTime::currentTime().addMSecs( millisecondsToWait );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}

QByteArray edfa::readSerial(){
   QByteArray buf;
    while(!serial->atEnd()){
        buf = serial->read(100);
    }
    return buf;
}

void edfa::writeSerial(QByteArray serialMsg){
    serial->clear();
    serial->write(serialMsg);
    serial->waitForBytesWritten(10000);
    delay(200);
}

//Function to strip the EDFA string of the carridge return and starting value.
QString edfa::stripString(QString inputString, QString stringIdentifier){
    inputString = inputString.replace("\r", "");
    inputString = inputString.replace("\n", "");
    inputString = inputString.replace("dBm", "");
    inputString = inputString.replace(stringIdentifier, "");
    inputString = inputString.replace(" ", "");

    return inputString;
}
