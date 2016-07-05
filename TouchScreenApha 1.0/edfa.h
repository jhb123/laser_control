#ifndef EDFA_H
#define EDFA_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QSerialPort>


class edfa: public QObject
{
    Q_OBJECT
public:
    explicit edfa(QObject *parent = 0);
    ~edfa();
    bool setUpPort(QString name);
    void closeComport();

    void setPower(float value);
    float getPower();

    //Added to make similar to laser - SPG 29/06
    void setEmission(bool isOn);
    bool getEmission();

    bool replyCheck();

    float getPh1();
    float getPh2();

signals:
        void EDFAComportOpen(bool isOpen);
private:
    QSerialPort *serial;

    QByteArray buf; //stores data read from the edfa

    void EDFAOn();
    void EDFAOff();

    void delay(int millisecondsToWait);

    QByteArray readSerial();
    void writeSerial(QByteArray serialMsg);

    QString stripString(QString inputString, QString stringIdentifier);



};

#endif // EDFA_H
