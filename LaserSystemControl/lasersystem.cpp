/* lasersystem implimentation file.
 *
 * This class deals with the laser and EDFA as a whole system. It has all the system protections to make sure state changes are not made when the
 * system is either not in a state to deal with them or they would be damaging to the system.
 *
 * This file can be used to control the Kohoras BasiK E15 laser and NKT photonics OEM amp.
 * */

#include "lasersystem.h"

LaserSystem::LaserSystem(QObject *parent) : QObject(parent)
{
    //Create edfa and laser objects.
    SeedLaser = new Laser(this);
    Edfa = new edfa(this);

    //Connect comport state change signals from laser and EDFA to relevant slots.
    connect(SeedLaser->interbus, SIGNAL(comportOpened(QString,bool)), this, SLOT(updateLaserComport(QString,bool)));
    connect(Edfa, SIGNAL(EDFAComportOpen(bool)),this, SLOT(updateEdfaComport(bool)));

    setupComports(); //Attempt to search and connect to the devices.

    if(!isLaserComOpen_ || !isEdfaComOpen_){
        loadSettings(); //Load default settings from the ini file if the comports haven't opened.
    }

    comFails_ = 0;
    updateCounter_ = 0;
}

LaserSystem::~LaserSystem(){
    qDebug()<<"LaserSystem destructor";
}

//Function to automatically connect EDFA and laser. Returns true if success.
bool LaserSystem::setupComports(){
    QStringList availableComports = SeedLaser->interbus->getAvailableComports(); //Gets a list of the available comports using the laser's interbus.
    int noOfComports = availableComports.size();
    //Bools for assessing the success of the connect.
    bool isLaserFound = false;
    bool isEdfaFound = false;
    bool isConnectSuccess = true;

    if(!isLaserComOpen_){ //Only tries to setup laser if it is not already connected.
        int comportCounter = 0; //Ceate counter
        while((comportCounter < noOfComports) && (isLaserFound == false)){ //Loops over available ports.
            SeedLaser -> setUpPort(availableComports[comportCounter]); //Opens the port on the current port.
            SeedLaser ->setAcknowlage(true); //Sets acknowlage to true.
            if(SeedLaser->checkComs()){ //If the laser replies to a info request it has been found.
                //set bools and store port name.
                isLaserFound = true;
                isLaserComOpen_ = true;
                laserComName_ = availableComports[comportCounter];
                qDebug() << "Laser found at port " << laserComName_;
            }
            else{
                SeedLaser->closePort(); //Close port if laser couldn't connect there.
            }
            comportCounter++;
        }
    }else{isLaserFound = true;}
    //Same steps but with EDFA
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
        loadSettings(); //Loads full settings on connection.
    }

    return isConnectSuccess;
}
//Updates class variables when laser port state changes.
void LaserSystem::updateLaserComport(QString portName, bool isOpen){
    Q_UNUSED(portName);
    isLaserComOpen_=isOpen;
    emit laserComSignal(isOpen);
}
//Updates class variables for EDFA when port state changes.
void LaserSystem::updateEdfaComport(bool isOpen){
    isEdfaComOpen_= isOpen;
    emit edfaComSignal(isOpen);
}
//Function to check the current state of the comport for the laser. Depending on bool argument, the program will either check the state of the port
//or just return the last stored value.
bool LaserSystem::getLaserComOpen(bool shouldUpdate){
    if(shouldUpdate){
        if(!laserRecieveCheck()){
            SeedLaser->closePort();
        }
    }
    return isLaserComOpen_;
}
//Same as above.
bool LaserSystem::getEdfaComOpen(bool shouldUpdate){
    if(shouldUpdate){
        if(!edfaRecieveCheck()){
            Edfa->closeComport();
        }
    }
    return isEdfaComOpen_;
}
//Checks whether the laser can communicate.
bool LaserSystem::laserRecieveCheck(){
    if(isLaserComOpen_){
        return SeedLaser->checkComs();
    }else{return false;}
}
//Same as above.
bool LaserSystem::edfaRecieveCheck(){
    if(isEdfaComOpen_){
        return Edfa->replyCheck();
    }else{return false;}
}
//Closes laser comport after switching the EDFA and then laser off to avoid damage to EDFA.
void LaserSystem::closeLaserComport(){
    qDebug() << "Close Laser com";
    setEdfaEmission(false);
    setLaserEmission(false);
    SeedLaser->closePort();
}
//Closes EDFA comport.
void LaserSystem::closeEdfaComport(){
    qDebug() << "Close EDFA com";
    setEdfaEmission(false);
    Edfa->closeComport();
}
//Sets the emission state of the system as a whole and returns whether the operation succeeded.
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
//Method to check whether the system is emitting. Bool arguments choose if the class variables ae updated or not.
bool LaserSystem::getSystemEmission(bool shouldUpdateEdfa, bool shouldUpdateLaser){
    bool isOn = true; //Initially set bool to true and then set it false if either object is off.
    if(!getLaserEmission(shouldUpdateLaser)){
        isOn = false;
    }

    if(!getEdfaEmission(shouldUpdateEdfa)){
        isOn = false;
    }
    qDebug() << "System on " << isOn;
    emit systemEmissionSignal(isOn); //Signal emitting new emission state.
    return isOn;
}
//Method to change laser emission.
bool LaserSystem::setLaserEmission(bool isOn){
    qDebug() << "Laser change emission to " << isOn;
    if(isLaserComOpen_){ //Only tries if laser is connected.
        if(!((isEdfaOn_ == true) && (isOn == false))){ //Prevents the laser being turned off if the EDFA is on.
            SeedLaser->setEmission(isOn);
        }
            getSystemEmission(false, true); //Update the state of the laser register.
    }
    if(isLaserOn_ != isOn){
        comFails_++; //Adds a comfail if the set value does not match the class variable to try to catch loss of coms.
        return false;
    }
    return true;
}
//Method to get the laser emission state.
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
//Same as for laser set emission.
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
//Same as get laser emission.
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

    if((edfaPowerSetpoint_ != power)){
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
            qDebug() << "EDFA setpoint from get power " << edfaPowerSetpoint_;
        }
    }
   return edfaPowerSetpoint_;
}

bool LaserSystem::setLaserWavelength(float wavelength){
    if(isLaserComOpen_){
        qDebug() << "setting wavelength (system) to " << (float)wavelength;
        if(wavelength<laserMinWavelength_){ //Prevents laser setting bellow min operating wavelength.
            wavelength = laserMinWavelength_;
        }
        if(wavelength>laserMaxWavelength_){
            wavelength=laserMaxWavelength_;
        }
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
//Checks values and compares them with setpoint. Returns bool, true if stable.
bool LaserSystem::getStable(bool shouldUpdate){
    bool stable = true;
    if(getLaserWavelengthReadout(shouldUpdate)!=getLaserWavelengthSetpoint(shouldUpdate)){
        stable = false;
    }
    if(getEdfaPowerReadout(shouldUpdate)< (getEdfaPowerSetpoint(shouldUpdate)-edfaStablePowerRange_) || getEdfaPowerReadout(false) > (getEdfaPowerSetpoint(false)+edfaStablePowerRange_)){
        stable = false;
    }

    if(!isEdfaOn_ || !isLaserOn_ || !isEdfaComOpen_ || !isLaserComOpen_){
        stable=false;
    }

    emit stableCheckSignal(stable);
    return stable;
}
//Method updates system variables.
void LaserSystem::updateSystemState(bool shouldUpdateAll){
getLaserWavelengthReadout(true);
getEdfaPowerReadout(true);
    //Only updates emission state and com state every times called or every 5 comFails.
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

//All functions bellow deal with loading from a settings file.

void LaserSystem::loadSettings(){
    settings = new QSettings(QDir::currentPath() + "/laser_system_settings.ini", QSettings::IniFormat); //Creates a settings object in current directory.
    if(QFile::exists(QDir::currentPath() + "/laser_system_settings.ini")){ //If the file exisits load the settings.
        settings -> beginGroup("Laser");
        if(isLaserComOpen_){ //Only sets values if laser is connected.
            setLaserEmission(settings->value("initialEmission").toBool());
            laserWavelengthSetpoint_ = settings->value("wavelength").toFloat();
            setLaserWavelength(laserWavelengthSetpoint_);
            //These are all values which cannot be controlled by the UI but are setup anyway to ensure laser is operating in disired state.
            SeedLaser->setPowerMode(settings->value("power/current mode").toBool());
            SeedLaser->setPowerSetpoint(settings->value("power setpoint").toInt());
            SeedLaser->setRINSuppression(settings->value("RIN suppression").toBool());
            SeedLaser->setTWMode(settings->value("temperature/wavelength mode").toBool());
            SeedLaser->setPiezoModulation(settings->value("piezo enabled").toBool());
            SeedLaser->setTemperatureCompensation(settings->value("temperature compensation").toBool());
            qDebug() << "Loaded old ini" ;
        }else{
            laserWavelengthSetpoint_ = settings->value("wavelength").toFloat(); //Loads the setpoint to class variable.
        }
        settings -> endGroup();

        settings -> beginGroup("EDFA");
        if(isEdfaComOpen_){
            setEdfaEmission(settings->value("initialEmission").toBool());
            edfaPowerSetpoint_ = settings->value("power").toFloat();
            setEdfaPower(edfaPowerSetpoint_);
            edfaStablePowerRange_ = settings->value("stable range").toFloat();
        }else{
            edfaPowerSetpoint_=settings->value("power").toFloat();
        }
        settings->endGroup();
        settings->beginGroup("UI settings");
        isKeyboardDisabled_ = settings->value("keyboard disabled").toBool();
        isWindowFrameless_ = settings->value("open frameless").toBool();
        isWindowMaximised_ = settings->value("open frameless").toBool();
        isCameraEnabled_=settings->value("camera enabled").toBool();
        settings->endGroup();
    }
    else{
        //Creates a settings file if it doesn't already exist.
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
        settings->setValue("stable range", 0.02);
        settings->endGroup();
        settings->beginGroup("UI settings");
        settings->setValue("keyboard disabled", false);
        settings->setValue("open fullscreen", true);
        settings->setValue("open frameless", true);
        settings->setValue("camera enabled", true);
        settings->endGroup();
        delete settings;
        loadSettings();
        qDebug() << "Loaded new ini" ;
    }
}
//Saves new wavelength and power setpoints.
void LaserSystem::saveSettings(float wavelength, float power){
    settings->beginGroup("Laser");
    qDebug()<<"Saveing wavelength " << wavelength;
    settings->setValue("wavelength", wavelength);
    settings->endGroup();
    settings->beginGroup("EDFA");
    settings->setValue("power", power);
    settings->endGroup();
}

bool LaserSystem::getKeyboardDisabled(){
    return isKeyboardDisabled_;
}

bool LaserSystem::getWindowFrameless(){
    return isWindowFrameless_;
}

bool LaserSystem::getWindowFullscreen(){
    return isWindowMaximised_;
}

bool LaserSystem::getCameraEnabled(){
    return isCameraEnabled_;
}
