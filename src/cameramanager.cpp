
#include "cameramanager.h"
#include <QDebug> // For logging

// Constructor: Initializes state, timer, and state machine
CameraManager::CameraManager(QObject *parent)
    : QObject(parent),
      m_currentState(State::IDLE),
      m_isRecording(false),
      m_recordingTime(0) {
    m_recordingTimer = new QTimer(this);
    // Connect timer to increment recordingTime and emit signal
    connect(m_recordingTimer, &QTimer::timeout, [this]() {
        if (m_isRecording) { // Should always be true if timer is running for this purpose
            m_recordingTime++;
            emit recordingTimeChanged();
        }
    });
    initializeStateMachine(); // Set up state transitions
    qDebug() << "CameraManager initialized.";
}

// Defines the state machine transitions and handlers
void CameraManager::initializeStateMachine() {
    // IDLE State
    m_stateMachine[State::IDLE][Event::START_PREVIEW] = &CameraManager::onIdle_StartPreview;

    // PREVIEW State
    m_stateMachine[State::PREVIEW][Event::START_RECORDING] = &CameraManager::onPreview_StartRecording;
    m_stateMachine[State::PREVIEW][Event::STOP_PREVIEW] = &CameraManager::onPreview_StopPreview;

    // RECORDING State
    m_stateMachine[State::RECORDING][Event::STOP_RECORDING] = &CameraManager::onRecording_StopRecording;
    // Note: From RECORDING, STOP_PREVIEW is not a direct event. STOP_RECORDING leads to PREVIEW.
    // If user wants to stop everything from RECORDING, it would be STOP_RECORDING then STOP_PREVIEW.
}

// Sets the current camera state and manages recording flag/timer accordingly
void CameraManager::setState(State newState) {
    if (m_currentState != newState) {
        m_currentState = newState;
        emit stateChanged(); // Notify QML of state change
        qDebug() << "CameraManager: State changed to" << static_cast<int>(m_currentState);

        // Manage isRecording flag and timer based on the new state
        bool newIsRecording = (newState == State::RECORDING);
        if (m_isRecording != newIsRecording) {
            m_isRecording = newIsRecording;
            emit isRecordingChanged(); // Notify QML of recording status change
            if (m_isRecording) {
                m_recordingTime = 0; // Reset recording time
                emit recordingTimeChanged(); // Notify QML of initial time (0)
                m_recordingTimer->start(1000); // Start timer to tick every second
            } else {
                m_recordingTimer->stop(); // Stop timer if not recording
                // m_recordingTime is preserved until next recording starts or explicitly reset
            }
        }
    }
}

// Processes an incoming event based on the current state
void CameraManager::processEvent(Event event) {
    qDebug() << "CameraManager: Processing event" << static_cast<int>(event)
             << "in state" << static_cast<int>(m_currentState);

    auto stateIter = m_stateMachine.find(m_currentState);
    if (stateIter != m_stateMachine.end()) { // Check if current state is valid
        auto eventIter = stateIter->second.find(event);
        if (eventIter != stateIter->second.end()) { // Check if event is valid for current state
            EventHandler handler = eventIter->second; // Get the function pointer
            (this->*handler)();                       // Call the handler function
        } else {
            qWarning() << "CameraManager: No handler for event" << static_cast<int>(event)
                       << "in state" << static_cast<int>(m_currentState);
        }
    } else {
        qWarning() << "CameraManager: Unknown current state" << static_cast<int>(m_currentState);
    }
}

// --- Event Handler Implementations ---

void CameraManager::onIdle_StartPreview() {
    qDebug() << "CameraManager: Action -> Start Preview";
    // In a real application: Initialize camera hardware, start capturing frames for preview.
    setState(State::PREVIEW);
}

void CameraManager::onPreview_StartRecording() {
    qDebug() << "CameraManager: Action -> Start Recording";
    // In a real application: Command hardware to start saving video stream.
    setState(State::RECORDING); // This will also set m_isRecording = true and start timer
}

void CameraManager::onPreview_StopPreview() {
    qDebug() << "CameraManager: Action -> Stop Preview";
    // In a real application: Release camera hardware, stop capturing frames.
    // Ensure recording is also stopped if it was active (though this event is from PREVIEW state)
    if (m_isRecording) { // Should not happen if in PREVIEW state, but as a safeguard
        qWarning() << "CameraManager: Stopping recording implicitly on STOP_PREVIEW from PREVIEW state.";
        onRecording_StopRecording(); // This will transition to PREVIEW, then we go to IDLE.
    }
    setState(State::IDLE);
}

void CameraManager::onRecording_StopRecording() {
    qDebug() << "CameraManager: Action -> Stop Recording";
    // In a real application: Command hardware to stop saving video, finalize file.
    // m_isRecording will be set to false by setState(State::PREVIEW)
    // m_recordingTimer will be stopped by setState(State::PREVIEW)
    setState(State::PREVIEW); // Transition back to PREVIEW state after recording stops
}