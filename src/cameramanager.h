
#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <QObject>
#include <QTimer>       // For recording timer
#include <map>          // For state machine
#include <functional>   // For std::function
#include <QQmlEngine>   // For QML_ELEMENT

// Manages camera preview and recording states
class CameraManager : public QObject {
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_ELEMENT
#endif

    // Properties exposed to QML for UI binding
    Q_PROPERTY(State currentState READ currentState NOTIFY stateChanged)
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY isRecordingChanged)
    Q_PROPERTY(int recordingTime READ recordingTime NOTIFY recordingTimeChanged) // In seconds

public:
    // Camera operational states
    enum class State { IDLE, PREVIEW, RECORDING };
    Q_ENUM(State)

    // Events that can trigger state changes
    enum class Event { START_PREVIEW, STOP_PREVIEW, START_RECORDING, STOP_RECORDING };
    Q_ENUM(Event)

    explicit CameraManager(QObject *parent = nullptr);

    // Accessors for Q_PROPERTY
    State currentState() const { return m_currentState; }
    bool isRecording() const { return m_isRecording; }
    int recordingTime() const { return m_recordingTime; }

    // Q_INVOKABLE method for QML to trigger camera events
    Q_INVOKABLE void processEvent(Event event);

signals:
    void stateChanged();         // Emitted when camera state (IDLE, PREVIEW, RECORDING) changes
    void isRecordingChanged();   // Emitted when recording status (true/false) changes
    void recordingTimeChanged(); // Emitted every second during recording

private:
    // Type alias for state event handler function pointer
    using EventHandler = void (CameraManager::*)();
    // State machine: maps (CurrentState, Event) to a handler function
    std::map<State, std::map<Event, EventHandler>> m_stateMachine;

    void initializeStateMachine(); // Sets up the state machine
    void setState(State newState); // Helper to change state and emit signals

    // --- State Handler Function Declarations ---
    void onIdle_StartPreview();
    void onPreview_StartRecording();
    void onPreview_StopPreview();       // Also called if recording stops and goes back to preview
    void onRecording_StopRecording();

    State m_currentState;       // Current operational state
    bool m_isRecording;         // Flag indicating if currently recording
    int m_recordingTime;        // Duration of current recording in seconds
    QTimer* m_recordingTimer;   // Timer to update recordingTime every second
};

#endif // CAMERAMANAGER_H