#include "lasersystem.h"
#include<QDebug>
#include<QString>
#include<QDir>

LaserSystem::LaserSystem(QObject *parent) : QObject(parent)
{
    SeedLaser = new Laser(this);
    EDFA = new edfa(this);


    setUpComports();
    loadSettings();
}

LaserSystem::~LaserSystem(){
}

//Function to automatically connect EDFA and laser. Returns true if success.
bool LaserSystem::setUpComports(){
    QStringList availableComports = SeedLaser->interbus->getAvailableComports();

    bool laserFound = false;
    bool EDFAFound = false;
    bool connectSuccess = true;

    int comportCounter = 0;
    int noOfComports = availableComports.size();

    while((comportCounter < noOfComports) && (laserFound == false)){
        SeedLaser -> setUpPort(availableComports[comportCounter]);

        SeedLaser ->setAcknowlage(true);

        if(SeedLaser->checkComs()){
            laserFound = true;
            laserComOpen = true;
            laserCom = availableComports[comportCounter];
            qDebug() << "Laser found at port " << laserCom;
        }
        else{
            SeedLaser->closePort();

        }

        comportCounter++;

    }

    for(int i=0; i< noOfComports; i++){
        if(availableComports[i] != laserCom){
            EDFA->setUpPort(availableComports[i]);

            if(EDFA->replyCheck()){
                EDFAFound = true;
                EDFAComOpen = true;
                EDFACom = availableComports[i];
                qDebug() << "EDFA found at port " << EDFACom;
            }
            else{
                EDFA->closeComport();
            }
        }
    }

    if(laserFound == false){
        qDebug() << "Laser NOT found";
        laserComOpen = false;
        laserOn = false; //Stops buttons from trying to write if one is not open. Should probably just disable buttons based on com ports
        connectSuccess = false;
    }

    if(EDFAFound == false){
        qDebug() << "EDFA NOT found";
        EDFAComOpen = false;
        EDFAOn = false;
      connectSuccess = false;

    }

    return connectSuccess;

}


bool LaserSystem::setEmission(bool state){
    bool success = false;
    qDebug() << "Attempting to set emission to " << state;

    //Looks the same but the ordering makes sure they turn off in the correct order.
    if(state){
        if((setLaserEmission(state) == true) && (setEDFAEmission(state)== true)){
            success = true;
        }
    }else{

        if((setEDFAEmission(state)== true) && (setLaserEmission(state) == true) ){
            success = true;
        }
    }




    getEmission(false,false);
    return success;
}

bool LaserSystem::getEmission(bool updateEDFA, bool updateLaser){
    bool emission = true;

    if(!getLaserEmission(updateLaser)){
        emission = false;
    }

    if(!getEDFAEmission(updateEDFA)){
        emission = false;
    }
    qDebug() << "System on " << emission;


    emit systemEmission(emission);
    return emission;
}

bool LaserSystem::setLaserEmission(bool state){
    qDebug() << "Laser change emission to " << state;
    if(!((EDFAOn == true) && (state == false))){
        SeedLaser->setEmission(state);
    }
    getEmission(false, true);
    return (getLaserEmission(false) == state) ?  true : false;
}

bool LaserSystem::getLaserEmission(bool update){
    if(laserComOpen){
        if(update){
            laserOn = SeedLaser -> getEmission();
            qDebug() << "Laser emission " << laserOn;
        }
        emit laserEmission(laserOn);
    }
    return laserOn;
}

bool LaserSystem::setEDFAEmission(bool state){
   qDebug() << "Setting EDFA emission to " << state;
   if(!((laserOn == false) && (state == true))){
       qDebug() << "In edfa if" ;
       EDFA->setEmission(state);
   }
   getEmission(true,false);
   return (getEDFAEmission(false) == state) ?  true : false;
}


bool LaserSystem::getEDFAEmission(bool update){
    if(EDFAComOpen){
        if(update){
        EDFAOn = EDFA->getEmission();
        qDebug() << "EDFA emission " << EDFAOn;
        emit EDFAEmission(EDFAOn);
        }

 return EDFAOn;
}
}

bool LaserSystem::setPower(int power){
    return true;
}

bool LaserSystem::setWavelength(float wavelength){
    SeedLaser -> setWavelength(wavelength);
    getWavelengthSetpoint(true);

    return (getWavelengthSetpoint(false) == wavelength) ?  true : false;
}



float LaserSystem::getPower(bool update){
    EDFAPower = 30.0; //testing;

}

float LaserSystem::getWavelength(bool update){
    if(update){
        laserWavelength = SeedLaser -> getWavelength();
        qDebug()<< "Laser wavelength " << laserWavelength;
    }

    emit laserWavelengthUpdate(laserWavelength);
    return laserWavelength;
}

float LaserSystem::getWavelengthSetpoint(bool update){
    if(update){
        laserWavelengthSetpoint = SeedLaser -> getWavelengthSetpoint();
    }
    return laserWavelengthSetpoint;
}


void LaserSystem::updateSystemState(){
getEmission(true, true);
getWavelength(true);
}

void LaserSystem::loadSettings(){
    settings = new QSettings(QDir::currentPath() + "/system_settings.ini", QSettings::IniFormat);
    if(QFile::exists(QDir::currentPath() + "/system_settings.ini")){
        if(laserComOpen){
            settings -> beginGroup("Laser");
            setLaserEmission(settings->value("initialEmission").toBool());
            setWavelength(settings->value("wavelength").toFloat());
            SeedLaser->setPowerMode(settings->value("power/current mode").toBool());
            SeedLaser->setPowerSetpoint(settings->value("power setpoint").toInt()); //Not sure if this will convert properly
            SeedLaser->setRINSuppression(settings->value("RIN suppression").toBool());
            SeedLaser->setTWMode(settings->value("temperature/wavelength mode").toBool());
            SeedLaser->setPiezoModulation(settings->value("piezo enabled").toBool());
            SeedLaser->setTemperatureCompensation(settings->value("temperature compensation").toBool());
            settings -> endGroup();
            qDebug() << "Loaded old ini" ;
        }
        if(EDFAComOpen){
            settings -> beginGroup("EDFA");
            setEDFAEmission(settings->value("initialEmission").toBool());
            setPower(settings->value("power").toFloat());
        settings->endGroup();
        }

    }
    else{
        settings->beginGroup("Laser");
        settings->setValue("initialEmission", false);
        settings->setValue("wavelength", 1560.000);
        settings->setValue("power/current mode", true);
        settings->setValue("power setpoint", 30000);
        settings->setValue("RIN suppression", true);
        settings->setValue("temperature/wavelength mode", true);
        settings->setValue("piezo enabled", true);
        settings->setValue("temperature compensation", true);
        settings->endGroup();
        settings->beginGroup("EDFA");
        settings->setValue("initialEmission", false);
        settings->setValue("power", 25);
        settings->endGroup();
        delete settings;
        loadSettings();
        qDebug() << "Loaded new ini" ;
    }
}

void LaserSystem::saveSettings(){

}
