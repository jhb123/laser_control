/* Edfa header file
 *
 * */
#ifndef EDFA_H
#define EDFA_H

#include <QByteArray>
#include <QSerialPort>
#include<QDebug>
#include<QTimer>
#include<QTime>
#include<QCoreApplication>


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

    void setEmission(bool isOn);
    bool getEmission();

    bool replyCheck();

    float getPh1();
    float getPh2();

signals:
        void EDFAComportOpen(bool isOpen);
private:
    QSerialPort *serial;

    void delay(int millisecondsToWait);

    QByteArray readSerial();
    void writeSerial(QByteArray serialMsg);

    QString stripString(QString inputString, QString stringIdentifier);
};

#endif // EDFA_H
