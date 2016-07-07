/* edfa implimentation file.
 *
 * This class is for communicating with the NKT Photonics OEM Amplifier.
 *
 */

#include "edfa.h"

edfa::edfa(QObject *parent) : QObject(parent)
{
    serial = new QSerialPort(this); //Creates serial object for serial communication.
}

edfa::~edfa()
{
        qDebug()<<"EDFA destructor";
    closeComport();
}
//Setup the port. Returns success.
bool edfa::setUpPort(QString name){
    serial -> close(); //Make sure the port is closed before attempting to open.
    serial->setPortName(name);
    if(serial->open(QIODevice::ReadWrite)){ //If the serial can be opened then set it up with edfa settings.
        serial->setBaudRate(QSerialPort::Baud9600);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        emit EDFAComportOpen(serial-> isOpen()); //emit signal to say port has opened.
    return true;
    }
    closeComport(); //close the port if opening fails.
    return false;
}
//Close port function.
void edfa::closeComport(){
    serial -> close();
    emit EDFAComportOpen(serial -> isOpen()); //Signal to show state change.
}
//Sets the EDFA power setting.
void edfa::setPower(float value){
    qDebug() << "Setting power to " << value;
    QString valueQString;
    valueQString.setNum(value);
    QString command = "APC " + valueQString +"\r\n"; //concatinate the string.
    QByteArray commandByteArray = command.toLatin1(); //convert string to byte array.
    writeSerial(commandByteArray);
    readSerial();//The serial must be read after a write.
}
//Gets the setpoint power from the APC register.
float edfa::getPower(){
    QString message;
    writeSerial("APC\r\n");
    message = readSerial();
    qDebug() << "get EDFA power message " << message;
    if (message.contains("apc: ",Qt::CaseInsensitive)){
        message = stripString(message, "APC:");
        qDebug() << "message contains APC";
        qDebug() << "New message " << message;
    }
    return message.toFloat();
}
//Checks to see if the EDFA is replying to commands by sending giberish and getting an FWC reply.
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
//Set emission of EDFA. OM is the emission register.
void edfa::setEmission(bool isOn){
    qDebug() << "Set emission in EDFA object " << isOn;
    if(isOn){
        qDebug() << "Final EDFA write on";
        writeSerial("OM 2\r\n");
    }
    else{
        qDebug() << "Final EDFA write off";
        writeSerial("OM 0\r\n");
    }
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
    if(message == "O"){//Note that for some reason the message has a letter O instead of #0 when reading.
        emission = false;
    }
    if(message == "2"){
        emission = true;
    }

return emission;
}
//Input diode power read (Ph1 register).
float edfa::getPh1(){
    writeSerial("Ph 1\r\n");
    QString message=readSerial();
    if (message.contains("PDM1: ",Qt::CaseInsensitive)){
        message = stripString(message, "PDM1:");
    }
    return message.toFloat();
}
//Output diode power read (Ph2 register).
float edfa::getPh2(){
    writeSerial("Ph 2\r\n");
    QString message=readSerial();
    if (message.contains("PDM2: ",Qt::CaseInsensitive)){
        message = stripString(message, "PDM2:");
    }
    qDebug()<<"PH2 final message " << message;
    return message.toFloat();
}
//Delay function used to wait for serial to register in EDFA. http://stackoverflow.com/questions/3752742/how-do-i-create-a-pause-wait-function-using-qt.
void edfa::delay( int millisecondsToWait )
{
    QTime dieTime = QTime::currentTime().addMSecs( millisecondsToWait );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}
//General function to read the serial.
QByteArray edfa::readSerial(){
   QByteArray buf;
    while(!serial->atEnd()){
        buf = serial->read(100);
    }
    return buf;
}
//General function to write to serial. Note that a delay is required. Unsure of why but guessing that the EDFA needs time to process.
void edfa::writeSerial(QByteArray serialMsg){
    serial->clear();
    serial->write(serialMsg);
    serial->waitForBytesWritten(10000);
    qDebug() << "before delay";
    delay(50);
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
