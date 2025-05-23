
#include "maincontroller.h"
#include "channelmodel.h"      // Definition of ChannelModel
#include "hardwaremanager.h"   // Definition of HardwareManager
#include "callmanager.h"       // Definition of CallManager
#include "messagemanager.h"    // Definition of MessageManager
#include "cameramanager.h"     // Definition of CameraManager
#include "videoplayermanager.h"// Definition of VideoPlayerManager
#include <QDebug>              // For logging

// Constructor: Initializes all member managers and connects signals/slots
MainController::MainController(QObject *parent)
    : QObject(parent),
      m_currentMode(AppMode::Home) { // Default mode is Home

    // Instantiate all manager classes, setting 'this' as parent for auto-deletion
    m_channelModel = new ChannelModel(this);
    m_hardwareManager = new HardwareManager(this);
    m_callManager = new CallManager(this);
    m_messageManager = new MessageManager(this);
    m_cameraManager = new CameraManager(this);
    m_videoPlayerManager = new VideoPlayerManager(this);

    // Connect signals from HardwareManager to appropriate slots or models
    connect(m_hardwareManager, &HardwareManager::channelStateFromHardware,
            this, &MainController::onHardwareChannelStateChanged);

    // Connect ChannelModel's request to toggle hardware to HardwareManager
    connect(m_channelModel, &ChannelModel::commandToggleChannel,
            m_hardwareManager, &HardwareManager::setHardwareChannelState);


    // Initialize channel model with some dummy channels for demonstration
    for (int i = 0; i < 16; ++i) {
        m_channelModel->addChannel({i, QString("Channel %1").arg(i + 1), false});
    }

    m_hardwareManager->startSimulation(); // Start hardware simulation for demo purposes
    qDebug() << "MainController initialized.";
}

// Destructor: Qt's parent-child mechanism handles deletion of member QObjects
MainController::~MainController() {
    qDebug() << "MainController destroyed.";
}

// Getter for currentMode property
MainController::AppMode MainController::currentMode() const {
    return m_currentMode;
}

// Setter for currentMode property; emits signal if mode changes
void MainController::setCurrentMode(AppMode mode) {
    if (m_currentMode != mode) {
        m_currentMode = mode;
        qDebug() << "MainController: Mode changed to" << static_cast<int>(mode);
        emit currentModeChanged(m_currentMode); // Notify QML and other C++ parts
    }
}

// Getter for ChannelModel
ChannelModel* MainController::channelModel() const {
    return m_channelModel;
}

// Getter for CallManager
CallManager* MainController::callManager() const {
    return m_callManager;
}

// Getter for MessageManager
MessageManager* MainController::messageManager() const {
    return m_messageManager;
}

// Getter for CameraManager
CameraManager* MainController::cameraManager() const {
    return m_cameraManager;
}

// Getter for VideoPlayerManager
VideoPlayerManager* MainController::videoPlayerManager() const {
    return m_videoPlayerManager;
}


// Q_INVOKABLE method for QML to change the mode
void MainController::selectMode(AppMode mode) {
    setCurrentMode(mode);
}

// Slot implementation for hardware channel state changes
void MainController::onHardwareChannelStateChanged(int channelId, bool isOn) {
    qDebug() << "MainController: Received hardware update for channel" << channelId << "to" << isOn;
    m_channelModel->updateChannelState(channelId, isOn); // Update the model, which notifies QML
}