/*Touchscreen.cpp
 *
 * This implimentation file contains all of the button handles UI events. It stores no data about the laser system and is written for the specific .ui file.
 *
 * */


#include "touchscreen.h"
#include "ui_touchscreen.h"


TouchScreen::TouchScreen(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TouchScreen)
{
    //Create objects needed for UI
    MainSystem = new LaserSystem(this);
    ui->setupUi(this); //Generate UI

    //Checks if camera is enabled in settings and sets up ui acordingly.
    if(MainSystem->getCameraEnabled()){
    MotCamera = new Camera(ui->displayBoxCameraViewFinder);
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    }else{
        ui->tabWidget->removeTab(2);
    }

    ui->keyboardWidgetBox->hide(); //Set the touch keyboard to be hidden initially.

    if(MainSystem->getWindowFrameless()){//Sets window frameless based on config file.
        this->setWindowFlags(Qt::FramelessWindowHint);
    }

    if(MainSystem->getWindowFullscreen()){//Sets window fullscreen based on settings file.
        this->showMaximized();
    }

    //Setup slots for widgets
    connect(MainSystem, SIGNAL(edfaEmissionSignal(bool)), this, SLOT(edfaEmissionUpdate(bool)));
    connect(MainSystem, SIGNAL(laserEmissionSignal(bool)), this, SLOT(laserEmissionUpdate(bool)));
    connect(MainSystem, SIGNAL(systemEmissionSignal(bool)), this, SLOT(systemEmissionUpdate(bool)));

    connect(MainSystem, SIGNAL(laserComSignal(bool)), this, SLOT(laserComportState(bool)));
    connect(MainSystem, SIGNAL(edfaComSignal(bool)), this, SLOT(edfaComportState(bool)));
    laserComportState(MainSystem->getLaserComOpen(false)); //Check for initial COM state of laser and EDFA. Doing this will update the buttons.
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

    defaultWavelength_ = MainSystem->getLaserWavelengthSetpoint(false); //Sets the default wavelength. This is the value before any changes.
    qDebug() << "Default wavelenth " << defaultWavelength_;
    ui->wavelengthSetpointSpinBox->setValue(defaultWavelength_); //Sets the spinbox to the initial setpoint value.
    defaultPower_ = MainSystem->getEdfaPowerSetpoint(false);
    ui->powerSetpointSpinBox->setValue(defaultPower_);
    //Connect slots for setpoint spinboxes. Note that editingFinished() is only emitted when the spinbox loses focus or enter is pressed
    connect(ui->wavelengthSetpointSpinBox, SIGNAL(editingFinished()), this, SLOT(setLaserWavelength()));
    connect(ui->powerSetpointSpinBox, SIGNAL(editingFinished()), this, SLOT(setEdfaPower()));

    connect(ui->savePushButton, SIGNAL(pressed()), this, SLOT(saveSettings()));
    connect(ui->loadDefaultPushButton, SIGNAL(pressed()), SLOT(loadDefaults()));

    if(!MainSystem->getKeyboardDisabled()){ //Only connects the keyboard if the keyboard is enabled in the settings.
        //Add keyboard buttons to a list.
        buttonList_.append(ui->Button_00);
        buttonList_.append(ui->Button_01);
        buttonList_.append(ui->Button_02);
        buttonList_.append(ui->Button_03);
        buttonList_.append(ui->Button_04);
        buttonList_.append(ui->Button_05);
        buttonList_.append(ui->Button_06);
        buttonList_.append(ui->Button_07);
        buttonList_.append(ui->Button_08);
        buttonList_.append(ui->Button_09);
        buttonList_.append(ui->Button_Del);
        buttonList_.append(ui->Button_Enter);
        //Loop to connect buttons to the keyboard button slot.
        for(int i = 0; i < 12; i++){
            connect(buttonList_[i], SIGNAL(pressed()),this, SLOT(keyboardButtonPressed()));
        }
        //This allows the program to know when one of the spinboxes gets focus. The "qApp" macro is a pointer to the application widget created by Qt.
        connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(showKeyboard(QWidget*,QWidget*)));
    }

    //Updates all of the variables in the system.
    MainSystem -> updateSystemState(true);

    //Update timer. Every second, system variables will be checked to update the UI to the current settings.
    UpdateTimer = new QTimer(this);
    connect(UpdateTimer, SIGNAL(timeout()), this ,SLOT(updateSystemTimer()));
    UpdateTimer->start(timeBetweenMeasurements_);
}
//Destructor.
TouchScreen::~TouchScreen()
{
    qDebug()<<"Touchscreen destructor";
    delete ui;
}

//The next few methods are slots for the state buttons on the UI. They change the colour of the buttons by hiding or showing
//red or green buttons which have the same slot.
void TouchScreen::systemEmissionUpdate(bool isEmissionOn){
    if(isEmissionOn){
        ui->systemStackedWidget->setCurrentIndex(0);
    }else{
        ui->systemStackedWidget->setCurrentIndex(1);
    }
}

void TouchScreen::edfaEmissionUpdate(bool isEmissionOn){
    qDebug() << "Updated EDFA button";
    if(isEmissionOn){
        ui->edfaEmitStackedWidget->setCurrentIndex(0);
    }
    else{
        ui->edfaEmitStackedWidget->setCurrentIndex(1);
    }
}

void TouchScreen::laserEmissionUpdate(bool isEmissionOn){
    qDebug() << "Updated laser button";
    if(isEmissionOn){
        ui->laserEmitStackedWidget->setCurrentIndex(0);
    }
    else{
        ui->laserEmitStackedWidget->setCurrentIndex(1);
    }
}

void TouchScreen::stableLightUpdate(bool isStable){
    if(isStable){
        ui->stableStackedWidget->setCurrentIndex(0);
    }
    else{
        ui->stableStackedWidget->setCurrentIndex(1);
    }
}

void TouchScreen::laserComportState(bool isOpen){
    qDebug() << "EDFA com " << isOpen;
    if(isOpen){
        ui->laserPortStackedWidget->setCurrentIndex(0);
    }
    else{
        ui->laserPortStackedWidget->setCurrentIndex(1);
    }
}

void TouchScreen::edfaComportState(bool isOpen){
    qDebug() << "EDFA com " << isOpen;
    if(isOpen){
        ui->edfaPortStackedWidget->setCurrentIndex(0);
    }
    else{
        ui->edfaPortStackedWidget->setCurrentIndex(1);
    }
}

//These methods call functions in the lasersystem class based on button presses. They are all emission buttons and the new emission
//state is decided by which button is hidden.
void TouchScreen::systemButton(){
        qDebug() << "Main button press";
    MainSystem -> setSystemEmission((ui->systemStackedWidget->currentIndex()==1) ? true:false);
}

void TouchScreen::edfaButton(){
        qDebug() << "EDFA Button";
    MainSystem -> setEdfaEmission((ui->edfaEmitStackedWidget->currentIndex()==1) ? true:false);
}

void TouchScreen::laserButton(){
    MainSystem -> setLaserEmission((ui->laserEmitStackedWidget->currentIndex()==1) ? true:false);
}

void TouchScreen::setupComports(){
    //The comports will be setup if either of the comports are off. In lasersystem, the setup comports function will only attempt to
    //setup ports which are not already open.
    if((ui->edfaPortStackedWidget->currentIndex() == 1) || (ui->laserPortStackedWidget->currentIndex()==1)){
        qDebug()<< "Attempting to setup comport";
        UpdateTimer->stop(); //Stops the update timer as this process takes some time. Don't want to try and update settings while the devices are not connected.
        MainSystem->setupComports(); //Call to setup com ports.
        UpdateTimer->start(timeBetweenMeasurements_); //Start the timer again.
    }
}
//Slot to set the laser wavelength to the value in the setpoint spinbox.
void TouchScreen::setLaserWavelength(){
    qDebug() << "SB value change";
    MainSystem -> setLaserWavelength(ui->wavelengthSetpointSpinBox->value());
}
//Same as above but for EDFA power.
void TouchScreen::setEdfaPower(){
        qDebug() << "power SB value change";
    MainSystem->setEdfaPower(ui->powerSetpointSpinBox->value());
}
//Updates the readout box.
void TouchScreen::updateLaserWavelengthReadout(float wavelength){
     qDebug()<< "Updating wavelength spin box to " << wavelength;
     ui->wavelengthReadoutSpinBox->setValue(wavelength);
     qDebug()<< "Wavelength db is " << ui->wavelengthReadoutSpinBox->value();
}

void TouchScreen::updateEdfaPowerReadout(float power){
    qDebug()<< "Updating power spin box to " << power;
    ui->powerReadoutSpinBox->setValue(power);
}
//Slot which the timer calls to update the system state. Full update is set to false as updating the emission and comport state
//every second is unnescessary.
void TouchScreen::updateSystemTimer(){
    MainSystem->updateSystemState(false);
}
//Slot for saveing the current setpoints. Also updates the default values.
void TouchScreen::saveSettings(){
    defaultWavelength_ = ui->wavelengthSetpointSpinBox->value();
    defaultPower_ = ui->powerSetpointSpinBox->value();
    MainSystem->saveSettings(defaultWavelength_, defaultPower_);
}
//Loads default values into spinboxes and then sets the power and wavelength manually as this will not trigger the
//editing finished signal.
void TouchScreen::loadDefaults(){
    ui->wavelengthSetpointSpinBox->setValue(defaultWavelength_);
    ui->powerSetpointSpinBox->setValue(defaultPower_);
    setLaserWavelength();
    setEdfaPower();
}
//Method for handling keyboard presses.
void TouchScreen::keyboardButtonPressed(){
    qDebug() << "Keyboard button pressed";
    int buttonNo = buttonList_.indexOf((QPushButton*)QObject::sender()); //Gets the index of the list where the pressed button is stored.
    if(buttonNo<10){ //First 10 buttons are just numvbers which match the index.
        pointerToActiveSpinBox_->setValue(addNumber(pointerToActiveSpinBox_->value(), QString::number(buttonNo))); //Add pressed number.
    }else if(buttonNo == 10){ //Handle delete
        pointerToActiveSpinBox_->setValue(deleteNumber(pointerToActiveSpinBox_->value()));
    }else if(buttonNo == 11){ //Handle enter.
        pointerToActiveSpinBox_->clearFocus();
    }
}
//Function to add number to spinbox. Uses string operations to manipulate the spinbox.
float TouchScreen::addNumber(float previousValue, QString numberAppend){
    QString outString = QString::number(previousValue);
    if(pointerToActiveSpinBox_==ui->wavelengthSetpointSpinBox){
        if(outString.length()!=4){
            outString = outString + numberAppend;
            return outString.toFloat();
        }
        else{
            outString = outString + "." + numberAppend;
            return outString.toFloat();
        }
    }else{
        if(outString.length()!=2){
            outString = outString + numberAppend;
            return outString.toFloat();
        }
        else{
            outString = outString + "." + numberAppend;
            return outString.toFloat();
        }
    }
}
//Removes the last number in the spinbox.
float TouchScreen::deleteNumber(float previousValue){
    QString outString = QString::number(previousValue);
    outString.chop(1);
    return outString.toFloat();
}
//Slot called when the focus has changed.
void TouchScreen::showKeyboard(QWidget*OldWidget,QWidget*NewWidget)
{
    Q_UNUSED(OldWidget);
    //Only interested in events from the spinbox.
    if((NewWidget==ui->wavelengthSetpointSpinBox)||(NewWidget==ui->powerSetpointSpinBox))
    {
        pointerToActiveSpinBox_ = (QDoubleSpinBox*)NewWidget; //Sets private reference to the active spinbox.
        ui->keyboardWidgetBox->show(); //Shows the keyboard.
    }
    else if((OldWidget==ui->wavelengthSetpointSpinBox)||(OldWidget==ui->powerSetpointSpinBox))
    {
        ui->keyboardWidgetBox->hide(); //Hides the keyboard if focus is given to any other object.
    }
}
//Slot which opens camera if the user selects the camera tab.
void TouchScreen::tabChanged(int tabIndex){
    if (tabIndex == 2){
        MotCamera->start();
    }
    else if(currentTab_== 2){ //Only stops camera if user moves off current tab to prevent calling stop when the camera is not on.
        MotCamera->stop();
    }

    currentTab_ = tabIndex;
}





