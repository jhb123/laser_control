#include "touchscreen.h"
#include "ui_touchscreen.h"

TouchScreen::TouchScreen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TouchScreen)
{
    MainSystem = new LaserSystem(this);

    ui->setupUi(this);

    //Setup slots for widgets
    connect(MainSystem, SIGNAL(edfaEmissionSignal(bool)), this, SLOT(edfaEmissionUpdate(bool)));
    connect(MainSystem, SIGNAL(laserEmissionSignal(bool)), this, SLOT(laserEmissionUpdate(bool)));
    connect(MainSystem, SIGNAL(systemEmissionSignal(bool)), this, SLOT(systemEmissionUpdate(bool)));

    connect(MainSystem, SIGNAL(laserComSignal(bool)), this, SLOT(laserComportState(bool)));
    connect(MainSystem, SIGNAL(edfaComSignal(bool)), this, SLOT(edfaComportState(bool)));
    laserComportState(MainSystem->getLaserComOpen(false));
    edfaComportState(MainSystem->getEdfaComOpen(false));

    connect(ui->laserPortOffPushButton, SIGNAL(pressed()), this, SLOT(setupComports()));
    connect(ui->edfaPortOffPushButton, SIGNAL(pressed()), this, SLOT(setupComports()));

    connect(ui->laserPortOnPushButton, SIGNAL(pressed()), MainSystem, SLOT(closeLaserComport()));
    connect(ui->edfaPortOnPushButton, SIGNAL(pressed()), MainSystem, SLOT(closeEdfaComport()));

    connect(ui->edfaOnPushButton, SIGNAL(pressed()), this, SLOT(edfaButton()));
    connect(ui->edfaOffPushButton, SIGNAL(pressed()), this, SLOT(edfaButton()));

    connect(ui->systemOnPushButton, SIGNAL(pressed()), this, SLOT(systemButton()));
    connect(ui->systemOffPushButton, SIGNAL(pressed()), this, SLOT(systemButton()));

    connect(ui->laserOnPushButton, SIGNAL(pressed()), this, SLOT(laserButton()));
    connect(ui->laserOffPushButton, SIGNAL(pressed()), this, SLOT(laserButton()));

    //Connects a system wavelength update to a slot which updates the text in the UI.
    connect(MainSystem, SIGNAL(laserWavelengthReadoutSignal(float)), this, SLOT(updateLaserWavelengthReadout(float)));
    connect(MainSystem, SIGNAL(edfaPowerReadoutSignal(float)), this, SLOT(updateEdfaPowerReadout(float)));

    connect(MainSystem, SIGNAL(stableCheckSignal(bool)), this, SLOT(stableLightUpdate(bool)));

    defaultWavelength_ = MainSystem->getLaserWavelengthSetpoint(false);
    ui->wavelengthSetpointSpinBox->setValue(defaultWavelength_);
    defaultPower_ = MainSystem->getEdfaPowerSetpoint(false);
    ui->powerSetpointSpinBox->setValue(defaultPower_);
    connect(ui->wavelengthSetpointSpinBox, SIGNAL(editingFinished()), this, SLOT(setLaserWavelength())); //May need to change the editingFinished signal to something else.
    connect(ui->powerSetpointSpinBox, SIGNAL(editingFinished()), this, SLOT(setEdfaPower()));

    connect(ui->savePushButton, SIGNAL(pressed()), this, SLOT(saveSettings()));
    connect(ui->loadDefaultPushButton, SIGNAL(pressed()), SLOT(loadDefaults()));

    //Updates all of the variables in the system.
    MainSystem -> updateSystemState(true);

    //Update timer. Every second, system variables will be checked to update the UI to the current settings.
    UpdateTimer = new QTimer(this);
    connect(UpdateTimer, SIGNAL(timeout()), this ,SLOT(updateSystemTimer()));
    UpdateTimer->start(1000);
}

TouchScreen::~TouchScreen()
{
    delete ui;
}

void TouchScreen::systemEmissionUpdate(bool isEmissionOn){
    if(isEmissionOn){
        ui->systemOffPushButton->hide();
        ui->systemOnPushButton->show();
    }else{
        ui->systemOffPushButton->show();
        ui->systemOnPushButton-> hide();
    }
}

//Series of slots to keep the colour of the buttons up to date.
void TouchScreen::edfaEmissionUpdate(bool isEmissionOn){
    qDebug() << "Updated EDFA button";
    if(isEmissionOn){
        ui->edfaOffPushButton->hide();
        ui->edfaOnPushButton->show();
    }
    else{
        ui->edfaOffPushButton->show();
        ui->edfaOnPushButton->hide();
    }
}

void TouchScreen::laserEmissionUpdate(bool isEmissionOn){
    qDebug() << "Updated laser button";
    if(isEmissionOn){
        ui->laserOffPushButton->hide();
        ui->laserOnPushButton->show();
    }
    else{
        ui->laserOffPushButton->show();
        ui->laserOnPushButton->hide();
    }
}

void TouchScreen::stableLightUpdate(bool isStable){
    if(isStable){
        ui->stableOffPushButton->hide();
        ui->stableOnPushButton->show();
    }
    else{
        ui->stableOffPushButton->show();
        ui->stableOnPushButton->hide();
    }
}

void TouchScreen::systemButton(){
        qDebug() << "Main button press";
    MainSystem -> setSystemEmission(ui->systemOnPushButton->isHidden());
}

void TouchScreen::edfaButton(){
        qDebug() << "EDFA Button";
    MainSystem -> setEdfaEmission(ui->edfaOnPushButton->isHidden());
}

void TouchScreen::laserButton(){
    MainSystem -> setLaserEmission(ui->laserOnPushButton->isHidden());
}

void TouchScreen::laserComportState(bool isOpen){
    qDebug() << "EDFA com " << isOpen;
    if(isOpen){
        ui->laserPortOffPushButton->hide();
        ui->laserPortOnPushButton->show();
    }
    else{
        ui->laserPortOffPushButton->show();
        ui->laserPortOnPushButton->hide();
    }
}

void TouchScreen::edfaComportState(bool isOpen){
    qDebug() << "EDFA com " << isOpen;
    if(isOpen){
        ui->edfaPortOffPushButton->hide();
        ui->edfaPortOnPushButton->show();
    }
    else{
        ui->edfaPortOffPushButton->show();
        ui->edfaPortOnPushButton->hide();
    }
}

void TouchScreen::setupComports(){
    if(ui->edfaPortOnPushButton->isHidden() || ui->laserPortOnPushButton->isHidden()){
        qDebug()<< "Attempting to setup comport";
        UpdateTimer->stop();
        MainSystem->setupComports(); //Not sure if this is the best way of doing it. May want to setup the comports indivdually?
        UpdateTimer->start(1000);
    }else{
        qDebug() << "Comports already setup!";
    }
}

void TouchScreen::setLaserWavelength(){
    qDebug() << "SB value change";
    MainSystem -> setLaserWavelength(ui->wavelengthSetpointSpinBox->value());
}

void TouchScreen::updateLaserWavelengthReadout(float wavelength){
     qDebug()<< "Updating wavelength spin box to " << wavelength;
     ui->wavelengthReadoutSpinBox->setValue(wavelength);
     qDebug()<< "Wavelength db is " << ui->wavelengthReadoutSpinBox->value();
}

void TouchScreen::updateEdfaPowerReadout(float power){
    qDebug()<< "Updating power spin box to " << power;
    ui->powerReadoutSpinBox->setValue(power);
}

void TouchScreen::setEdfaPower(){
        qDebug() << "power SB value change";
    MainSystem->setEdfaPower(ui->powerSetpointSpinBox->value());
}

void TouchScreen::updateSystemTimer(){
    MainSystem->updateSystemState(false);
}

void TouchScreen::saveSettings(){
    defaultWavelength_ = ui->wavelengthSetpointSpinBox->value();
    defaultPower_ = ui->powerSetpointSpinBox->value();
    MainSystem->saveSettings(defaultWavelength_, defaultPower_);
}

void TouchScreen::loadDefaults(){
    ui->wavelengthSetpointSpinBox->setValue(defaultWavelength_);
    ui->powerSetpointSpinBox->setValue(defaultPower_);
    setLaserWavelength();
    setEdfaPower();
}




