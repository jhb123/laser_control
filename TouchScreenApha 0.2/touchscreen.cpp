#include "touchscreen.h"
#include "ui_touchscreen.h"

#include <QTimer>
#include <QDebug>
#include <QDebug>

TouchScreen::TouchScreen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TouchScreen)
{
    mainSystem = new LaserSystem(this);

    ui->setupUi(this);

    //Setup slots for widgets
    connect(mainSystem, SIGNAL(EDFAEmission(bool)), this, SLOT(EDFAEmissionLight(bool)));
    connect(mainSystem, SIGNAL(laserEmission(bool)), this, SLOT(laserEmissionLight(bool)));
    connect(mainSystem, SIGNAL(systemEmission(bool)), this, SLOT(mainButtonState(bool)));

    connect(ui->EDFAOn, SIGNAL(pressed()), this, SLOT(EDFAOnOff()));
    connect(ui->EDFAOff, SIGNAL(pressed()), this, SLOT(EDFAOnOff()));


    connect(ui->TurnOnBtn, SIGNAL(pressed()), this, SLOT(mainButton()));
    connect(ui->TurnOffBtn, SIGNAL(pressed()), this, SLOT(mainButton()));

    connect(ui->LaserQuestOff, SIGNAL(pressed()), this, SLOT(laserOnOff()));
    connect(ui->LaserQuestOn, SIGNAL(pressed()), this, SLOT(laserOnOff()));

    //Connects a system wavelength update to a slot which updates the text in the UI.
    connect(mainSystem, SIGNAL(laserWavelengthUpdate(float)), this, SLOT(updateWavelength(float)));

    //Updates all of the variables in the system.
    mainSystem -> updateSystemState();

    ui->wavelengthSb->setValue(mainSystem->getWavelength(false)); //This probably isnt needed as wavelength should be automatically updated.
    connect(ui->wavelengthSb, SIGNAL(editingFinished()), this, SLOT(setNewWavelength()));

    //Not yet implimented
    ui->StableOff->show();
    ui->StableOn->hide();

    //Change these timers so that all of the system variables are checked at intervals
    QTimer *wavelengthTimer = new QTimer(this);
    connect(wavelengthTimer, SIGNAL(timeout()),this, SLOT(wavelength = ui->hsWavelength->value()));

    //Setup similar system to origional interface where the button type is automatically updated uing signals.

    //Update timer. Every second, system variables will be checked to update the UI to the current settings.
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()),mainSystem,SLOT(updateSystemState()));
    timer->start(1000);

    //camera and viewfinder
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(tabTest()));
    cam = new camera(ui->viewfinder);
    if (ui->tabWidget->currentIndex() == 1 ){
        cam->start();
    }
}

TouchScreen::~TouchScreen()
{
    delete ui;
}


//Series of slots to keep the colour of the buttons up to date.
void TouchScreen::EDFAEmissionLight(bool emissionOn){
    qDebug() << "Updated EDFA button";
    if(emissionOn){
        ui->EDFAOff->hide();
        ui->EDFAOn->show();
    }
    else{
        ui->EDFAOff->show();
        ui->EDFAOn->hide();
    }
}

void TouchScreen::laserEmissionLight(bool emissionOn){
    qDebug() << "Updated laser button";
    if(emissionOn){
        ui->LaserQuestOff->hide();
        ui->LaserQuestOn->show();
    }
    else{
        ui->LaserQuestOff->show();
        ui->LaserQuestOn->hide();
    }
}

void TouchScreen::mainButtonState(bool emissionOn){
    if(emissionOn){
        ui->TurnOnBtn->show();
        ui->TurnOffBtn-> hide();
    }else{
        ui->TurnOnBtn->hide();
        ui->TurnOffBtn->show();
    }

}
//Probably change all these to just be slots in
void TouchScreen::mainButton(){
        qDebug() << "Main button press";
    mainSystem -> setEmission(ui->TurnOnBtn->isHidden());

}

void TouchScreen::EDFAOnOff(){
        qDebug() << "EDFA Button";
    mainSystem -> setEDFAEmission(ui->EDFAOn->isHidden());

}

void TouchScreen::laserOnOff(){
    qDebug() << "Laser Button";
    bool hidden = ui->LaserQuestOn->isHidden();
    qDebug() << "Red on button is hidden " << hidden;
    mainSystem -> setLaserEmission(hidden);

}

void TouchScreen::setNewWavelength(){
    qDebug() << "SB value change";
    mainSystem -> setWavelength(ui->wavelengthSb->value());
}

void TouchScreen::updatePower(float power){
    ui->powerDb->setValue(power);
}

void TouchScreen::updateWavelength(float wavelength){
    ui->wavelengthDb->setValue(wavelength);
}

void TouchScreen::tabTest(){
    qDebug() << ui->tabWidget->currentIndex();

    if (ui->tabWidget->currentIndex() == 1 ){
        cam->start();
    }
    else{
        cam->stop();
    }
}

