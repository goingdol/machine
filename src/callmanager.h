
#ifndef CALLMANAGER_H
#define CALLMANAGER_H

#include <QObject>
#include <QVariantMap>    // For passing data with events/states
#include <array>          // For fixed-size array of call lines
#include <map>            // For state machine transitions
#include <functional>     // For std::function (alternative to raw function pointers)
#include <QTimer>         // For simulating call durations, timeouts, etc.
#include <QQmlEngine>     // For QML_ELEMENT

// Manages call states and logic for multiple lines
class CallManager : public QObject {
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_ELEMENT
#endif

public:
    // States a call line can be in
    enum class LineState { IDLE, DIALING, RINGING_INCOMING, CONVERSATION, HANGING_UP };
    Q_ENUM(LineState) // Expose enum to Qt's meta-object system

    // Events that can occur for a call line
    enum class LineEvent {
        USER_INITIATE_DIAL,     // User presses dial button
        SYSTEM_INCOMING_CALL,   // Hardware signals an incoming call
        USER_ANSWER_CALL,       // User presses answer button
        SYSTEM_CALL_CONNECTED,  // Call is successfully established (outgoing or answered incoming)
        USER_HANGUP,            // User presses hangup/reject button
        SYSTEM_REMOTE_HANGUP,   // Remote party hangs up
        SYSTEM_CALL_FAILED,     // Dialing failed (busy, no answer, network error)
        SYSTEM_ACTION_COMPLETE  // Internal event, e.g., hangup process finished
    };
    Q_ENUM(LineEvent) // Expose enum to Qt's meta-object system

    explicit CallManager(QObject *parent = nullptr);

    // Q_INVOKABLE method for QML to trigger call events
    Q_INVOKABLE void processCallEvent(int lineIndex, LineEvent event, const QVariantMap& data = {});

signals:
    // Signal emitted when a call line's state changes, providing details to QML
    void callLineStateChanged(int lineIndex, LineState newState, const QVariantMap& details);

private:
    // Structure to hold data for each call line
    struct CallLine {
        LineState currentState = LineState::IDLE;
        QString contactInfo;    // Phone number or caller ID
        QString statusMessage;  // Display message for QML (e.g., "Dialing...", "Connected")
        QTimer* actionTimer = nullptr; // Timer for simulating call progress, timeouts
    };
    std::array<CallLine, 4> m_lines; // Supports 4 call lines

    // Type alias for a pointer to a member function handling a state event
    using EventHandler = void (CallManager::*)(int lineIndex, const QVariantMap& data);
    // State machine: maps (CurrentState, Event) to a handler function
    std::map<LineState, std::map<LineEvent, EventHandler>> m_stateMachine;

    void initializeStateMachine(); // Sets up the m_stateMachine map
    // Helper to change state and emit signal
    void setState(int lineIndex, LineState newState, const QVariantMap& details = {});

    // --- State Handler Function Declarations ---
    // These functions implement the logic for each (State, Event) pair.
    void onIdle_UserInitiateDial(int lineIndex, const QVariantMap& data);
    void onIdle_SystemIncomingCall(int lineIndex, const QVariantMap& data);

    void onDialing_SystemCallConnected(int lineIndex, const QVariantMap& data);
    void onDialing_UserHangup(int lineIndex, const QVariantMap& data);
    void onDialing_SystemCallFailed(int lineIndex, const QVariantMap& data);

    void onRingingIncoming_UserAnswer(int lineIndex, const QVariantMap& data);
    void onRingingIncoming_UserHangup(int lineIndex, const QVariantMap& data); // Handles call rejection
    void onRingingIncoming_SystemRemoteHangup(int lineIndex, const QVariantMap& data); // Caller hangs up before answer

    void onConversation_UserHangup(int lineIndex, const QVariantMap& data);
    void onConversation_SystemRemoteHangup(int lineIndex, const QVariantMap& data);

    void onHangingUp_SystemActionComplete(int lineIndex, const QVariantMap& data); // Finalizes hangup
};

#endif // CALLMANAGER_H