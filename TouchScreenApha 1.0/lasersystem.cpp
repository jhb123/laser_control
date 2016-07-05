#include "lasersystem.h"
#include<QDebug>
#include<QDir>

LaserSystem::LaserSystem(QObject *parent) : QObject(parent)
{
    SeedLaser = new Laser(this);
    Edfa = new edfa(this);

    connect(SeedLaser->interbus, SIGNAL(comportOpened(QString,bool)), this, SLOT(updateLaserComport(QString,bool)));
    connect(Edfa, SIGNAL(EDFAComportOpen(bool)),this, SLOT(updateEdfaComport(bool)));

    setupComports();

    comFails_ = 0;
    updateCounter_ = 0;
}

LaserSystem::~LaserSystem(){
}

//Function to automatically connect EDFA and laser. Returns true if success.
bool LaserSystem::setupComports(){
    QStringList availableComports = SeedLaser->interbus->getAvailableComports();
    int noOfComports = availableComports.size();

    bool isLaserFound = false;
    bool isEdfaFound = false;
    bool isConnectSuccess = true;

    if(!isLaserComOpen_){
        int comportCounter = 0;
        while((comportCounter < noOfComports) && (isLaserFound == false)){
            SeedLaser -> setUpPort(availableComports[comportCounter]);

            SeedLaser ->setAcknowlage(true);

            if(SeedLaser->checkComs()){
                isLaserFound = true;
                isLaserComOpen_ = true;
                laserComName_ = availableComports[comportCounter];
                qDebug() << "Laser found at port " << laserComName_;
            }
            else{
                SeedLaser->closePort();
            }
            comportCounter++;
        }
    }else{isLaserFound = true;}

    if(!isEdfaComOpen_){
        for(int i=0; i< noOfComports; i++){
            if(availableComports[i] != laserComName_){
                Edfa->setUpPort(availableComports[i]);

                if(Edfa->replyCheck()){
                    isEdfaFound = true;
                    isEdfaComOpen_ = true;
                    edfaComName_ = availableComports[i];
                    qDebug() << "EDFA found at port " << edfaComName_;
                }
                else{
                    Edfa->closeComport();
                }
            }
        }
    }else{isEdfaFound=true;}

    if(isLaserFound == false){
        qDebug() << "Laser NOT found";
        isLaserComOpen_ = false;
        isLaserOn_ = false; //Stops buttons from trying to write if one is not open. Should probably just disable buttons based on com ports
        isConnectSuccess = false;
    }

    if(isEdfaFound == false){
        qDebug() << "EDFA NOT found";
        isEdfaComOpen_ = false;
        isEdfaOn_ = false;
        isConnectSuccess = false;
    }

    if(isConnectSuccess){
        qDebug()<< "Loading settings from ini";
        loadSettings();
    }

    return isConnectSuccess;
}

void LaserSystem::updateLaserComport(QString portName, bool isOpen){
    Q_UNUSED(portName);

    isLaserComOpen_=isOpen;
    emit laserComSignal(isOpen);
}

void LaserSystem::updateEdfaComport(bool isOpen){
    isEdfaComOpen_= isOpen;
    emit edfaComSignal(isOpen);
}

bool LaserSystem::getLaserComOpen(bool shouldUpdate){
    if(shouldUpdate){
        if(!laserRecieveCheck()){
            SeedLaser->closePort();
        }
    }
    return isLaserComOpen_;
}

bool LaserSystem::getEdfaComOpen(bool shouldUpdate){
    if(shouldUpdate){
        if(!edfaRecieveCheck()){
            Edfa->closeComport();
        }
    }
    return isEdfaComOpen_;
}


bool LaserSystem::laserRecieveCheck(){
    if(isLaserComOpen_){
        return SeedLaser->checkComs();
    }else{return false;}
}

bool LaserSystem::edfaRecieveCheck(){
    if(isEdfaComOpen_){
        return Edfa->replyCheck();
    }else{return false;}
}

void LaserSystem::closeLaserComport(){
    qDebug() << "Close Laser com";
    setEdfaEmission(false);
    setLaserEmission(false);
    SeedLaser->closePort();
}

void LaserSystem::closeEdfaComport(){
    qDebug() << "Close EDFA com";
    setEdfaEmission(false);
    Edfa->closeComport();
}

bool LaserSystem::setSystemEmission(bool isOn){
    bool success = false;
    qDebug() << "Attempting to set emission to " << isOn;

    //Looks the same but the ordering makes sure they turn on and off in the correct order.
    if(isOn){
        if((setLaserEmission(isOn) == true) && (setEdfaEmission(isOn)== true)){
            success = true;
        }
    }else{

        if((setEdfaEmission(isOn)== true) && (setLaserEmission(isOn) == true) ){
            success = true;
        }
    }
    return success;
}

bool LaserSystem::getSystemEmission(bool shouldUpdateEdfa, bool shouldUpdateLaser){
    bool isOn = true;

    if(!getLaserEmission(shouldUpdateLaser)){
        isOn = false;
    }

    if(!getEdfaEmission(shouldUpdateEdfa)){
        isOn = false;
    }
    qDebug() << "System on " << isOn;
    emit systemEmissionSignal(isOn);
    return isOn;
}

bool LaserSystem::setLaserEmission(bool isOn){
    qDebug() << "Laser change emission to " << isOn;
    if(isLaserComOpen_){
        if(!((isEdfaOn_ == true) && (isOn == false))){
            SeedLaser->setEmission(isOn);
        }
            getSystemEmission(false, true);
    }
    if(isLaserOn_ != isOn){
        comFails_++;
        return false;
    }
    return true;
}

bool LaserSystem::getLaserEmission(bool shouldUpdate){
    if(isLaserComOpen_){
        if(shouldUpdate){
            isLaserOn_ = SeedLaser -> getEmission();
            qDebug() << "Laser emission " << isLaserOn_;
            emit laserEmissionSignal(isLaserOn_);
        }
    }
    return isLaserOn_;
}

bool LaserSystem::setEdfaEmission(bool isOn){
   qDebug() << "Setting EDFA emission to " << isOn;
   if(isEdfaComOpen_){
       if(!((isLaserOn_ == false) && (isOn == true))){
           qDebug() << "In edfa if" ;
           Edfa->setEmission(isOn);
       }
       getSystemEmission(true,false);
   }

   if(isEdfaOn_ != isOn){
       comFails_++;
       return false;
   }
   return true;
}

bool LaserSystem::getEdfaEmission(bool shouldUpdate){
    if(isEdfaComOpen_){
        if(shouldUpdate){
        isEdfaOn_ = Edfa->getEmission();
        qDebug() << "EDFA emission " << isEdfaOn_;
        emit edfaEmissionSignal(isEdfaOn_);
        }
    }
    return isEdfaOn_;
}

bool LaserSystem::setEdfaPower(float power){
    qDebug() << "Setting EDFA power to " << power;
    if(isEdfaComOpen_){
        Edfa->setPower(power);
        getEdfaPowerSetpoint(true);
    }

    if(edfaPowerSetpoint_ != power){
        comFails_++;
        return false;
    }
    return true;
}

float LaserSystem::getEdfaPowerReadout(bool shouldUpdate){
    if(isEdfaComOpen_){
        if(shouldUpdate){
            edfaPower_ = Edfa->getPh2();
            qDebug() << "EDFA setpoint " << edfaPower_;
            emit edfaPowerReadoutSignal(edfaPower_);
        }
    }
    return edfaPower_;
}

float LaserSystem::getEdfaPowerSetpoint(bool shouldUpdate){
    if(isEdfaComOpen_){
        if(shouldUpdate){
            edfaPowerSetpoint_ = Edfa->getPower();
            qDebug() << "EDFA setpoint " << edfaPowerSetpoint_;
        }
    }
   return edfaPowerSetpoint_;
}

bool LaserSystem::setLaserWavelength(float wavelength){
    if(isLaserComOpen_){
        qDebug() << "setting wavelength (system) to " << wavelength;
        SeedLaser -> setWavelength(wavelength);
        getLaserWavelengthSetpoint(true);
    }
    if(laserWavelengthSetpoint_ != wavelength){
        comFails_++;
        return false;
    }
    return true;
}

float LaserSystem::getLaserWavelengthReadout(bool shouldUpdate){
    if(isLaserComOpen_){
        if(shouldUpdate){
            laserWavelength_ = SeedLaser -> getWavelength();
            qDebug()<< "Laser wavelength " << laserWavelength_;
            emit laserWavelengthReadoutSignal(laserWavelength_);
        }
    }
    return laserWavelength_;
}

float LaserSystem::getLaserWavelengthSetpoint(bool shouldUpdate){
    if(isLaserComOpen_){
        if(shouldUpdate){
            laserWavelengthSetpoint_ = SeedLaser -> getWavelengthSetpoint();
            qDebug()<< "Wavelength setpoint " << laserWavelengthSetpoint_;
        }
    }
    return laserWavelengthSetpoint_;
}

bool LaserSystem::getStable(bool shouldUpdate){
    bool stable = true;
    if(getLaserWavelengthReadout(shouldUpdate)!=getLaserWavelengthSetpoint(shouldUpdate)){
        stable = false;
    }
    if(getEdfaPowerReadout(shouldUpdate)< (getEdfaPowerSetpoint(shouldUpdate)-0.05) || getEdfaPowerReadout(false) > (getEdfaPowerSetpoint(false)+0.05)){
        stable = false;
    }

    if(!isEdfaOn_ || !isLaserOn_ || !isEdfaComOpen_ || !isLaserComOpen_){
        stable=false;
    }

    emit stableCheckSignal(stable);
    return stable;
}

void LaserSystem::updateSystemState(bool shouldUpdateAll){
getLaserWavelengthReadout(true);
getEdfaPowerReadout(true);

    if(comFails_>5 || updateCounter_ > 10 || shouldUpdateAll){
        getSystemEmission(true, true);
        getEdfaComOpen(true);
        getLaserComOpen(true);
        comFails_=0;
        updateCounter_ = 0;
    }
    getStable(false);
    updateCounter_++;
}

void LaserSystem::loadSettings(){
    settings = new QSettings(QDir::currentPath() + "/system_settings.ini", QSettings::IniFormat);
    if(QFile::exists(QDir::currentPath() + "/system_settings.ini")){
        if(isLaserComOpen_){
            settings -> beginGroup("Laser");
            setLaserEmission(settings->value("initialEmission").toBool());
            setLaserWavelength(settings->value("wavelength").toFloat());
            SeedLaser->setPowerMode(settings->value("power/current mode").toBool());
            SeedLaser->setPowerSetpoint(settings->value("power setpoint").toInt()); //Not sure if this will convert properly
            SeedLaser->setRINSuppression(settings->value("RIN suppression").toBool());
            SeedLaser->setTWMode(settings->value("temperature/wavelength mode").toBool());
            SeedLaser->setPiezoModulation(settings->value("piezo enabled").toBool());
            SeedLaser->setTemperatureCompensation(settings->value("temperature compensation").toBool());
            settings -> endGroup();
            qDebug() << "Loaded old ini" ;
        }
        if(isEdfaComOpen_){
            settings -> beginGroup("EDFA");
            setEdfaEmission(settings->value("initialEmission").toBool());
            setEdfaPower(settings->value("power").toFloat());
            edfaPowerSetpoint_ = settings->value("power").toFloat();
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
        settings->setValue("power", 25.0);
        settings->endGroup();
        delete settings;
        loadSettings();
        qDebug() << "Loaded new ini" ;
    }
}

void LaserSystem::saveSettings(float wavelength, float power){
    settings->beginGroup("Laser");
    settings->setValue("wavelength", wavelength);
    settings->endGroup();
    settings->beginGroup("EDFA");
    settings->setValue("power", power);
    settings->endGroup();
}
