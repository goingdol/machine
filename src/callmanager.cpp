
#include "callmanager.h"
#include <QDebug> // For logging
#include <QRandomGenerator> // For simulation

// Constructor: Initializes state machine and call lines
CallManager::CallManager(QObject *parent) : QObject(parent) {
    initializeStateMachine(); // Populate the state transition map
    for (int i = 0; i < 4; ++i) { // Initialize each of the 4 lines
        m_lines[i].currentState = LineState::IDLE;
        m_lines[i].statusMessage = "Idle";
        // Timers (m_lines[i].actionTimer) are created on demand
    }
    qDebug() << "CallManager initialized.";
}

// Defines the state machine transitions and their handlers
void CallManager::initializeStateMachine() {
    // --- IDLE State Transitions ---
    m_stateMachine[LineState::IDLE][LineEvent::USER_INITIATE_DIAL] = &CallManager::onIdle_UserInitiateDial;
    m_stateMachine[LineState::IDLE][LineEvent::SYSTEM_INCOMING_CALL] = &CallManager::onIdle_SystemIncomingCall;

    // --- DIALING State Transitions ---
    m_stateMachine[LineState::DIALING][LineEvent::SYSTEM_CALL_CONNECTED] = &CallManager::onDialing_SystemCallConnected;
    m_stateMachine[LineState::DIALING][LineEvent::USER_HANGUP] = &CallManager::onDialing_UserHangup; // User cancels dialing
    m_stateMachine[LineState::DIALING][LineEvent::SYSTEM_CALL_FAILED] = &CallManager::onDialing_SystemCallFailed;

    // --- RINGING_INCOMING State Transitions ---
    m_stateMachine[LineState::RINGING_INCOMING][LineEvent::USER_ANSWER_CALL] = &CallManager::onRingingIncoming_UserAnswer;
    m_stateMachine[LineState::RINGING_INCOMING][LineEvent::USER_HANGUP] = &CallManager::onRingingIncoming_UserHangup; // User rejects call
    m_stateMachine[LineState::RINGING_INCOMING][LineEvent::SYSTEM_REMOTE_HANGUP] = &CallManager::onRingingIncoming_SystemRemoteHangup; // Caller hangs up

    // --- CONVERSATION State Transitions ---
    m_stateMachine[LineState::CONVERSATION][LineEvent::USER_HANGUP] = &CallManager::onConversation_UserHangup;
    m_stateMachine[LineState::CONVERSATION][LineEvent::SYSTEM_REMOTE_HANGUP] = &CallManager::onConversation_SystemRemoteHangup;

    // --- HANGING_UP State Transitions ---
    m_stateMachine[LineState::HANGING_UP][LineEvent::SYSTEM_ACTION_COMPLETE] = &CallManager::onHangingUp_SystemActionComplete;
}

// Processes an incoming event for a specific call line
void CallManager::processCallEvent(int lineIndex, LineEvent event, const QVariantMap& data) {
    if (lineIndex < 0 || lineIndex >= static_cast<int>(m_lines.size())) {
        qWarning() << "CallManager: Invalid lineIndex" << lineIndex;
        return;
    }

    CallLine& line = m_lines[lineIndex]; // Get reference to the specific line
    qDebug() << "CallManager: Processing event" << static_cast<int>(event)
             << "for line" << lineIndex << "in state" << static_cast<int>(line.currentState)
             << "with data" << data;

    auto stateIter = m_stateMachine.find(line.currentState);
    if (stateIter != m_stateMachine.end()) { // Check if current state is valid
        auto eventIter = stateIter->second.find(event);
        if (eventIter != stateIter->second.end()) { // Check if event is valid for current state
            EventHandler handler = eventIter->second; // Get the function pointer
            (this->*handler)(lineIndex, data);       // Call the handler function
        } else {
            qWarning() << "CallManager: Line" << lineIndex << "- No handler for event" << static_cast<int>(event)
                       << "in state" << static_cast<int>(line.currentState);
        }
    } else {
        qWarning() << "CallManager: Line" << lineIndex << "- Unknown current state" << static_cast<int>(line.currentState);
    }
}

// Helper function to change the state of a line and emit the notification signal
void CallManager::setState(int lineIndex, LineState newState, const QVariantMap& details) {
    if (lineIndex < 0 || lineIndex >= static_cast<int>(m_lines.size())) return;

    CallLine& line = m_lines[lineIndex];
    // Update state and details only if there's a change or new status message
    if (line.currentState != newState || line.statusMessage != details.value("statusMessage").toString()) {
        line.currentState = newState;
        // Update line's data from details map, keeping old values if not provided
        line.statusMessage = details.value("statusMessage", line.statusMessage).toString();
        line.contactInfo = details.value("contactInfo", line.contactInfo).toString();

        // Ensure the emitted details map contains all current relevant info
        QVariantMap fullDetails = details;
        fullDetails.insert("statusMessage", line.statusMessage);
        fullDetails.insert("contactInfo", line.contactInfo);
        fullDetails.insert("lineState", static_cast<int>(newState)); // Also include the new state itself

        emit callLineStateChanged(lineIndex, newState, fullDetails);
        qDebug() << "CallManager: Line" << lineIndex << "state changed to" << static_cast<int>(newState)
                 << "Status:" << line.statusMessage << "Contact:" << line.contactInfo;
    }
}

// --- Event Handler Implementations ---

void CallManager::onIdle_UserInitiateDial(int lineIndex, const QVariantMap& data) {
    QString numberToDial = data.value("phoneNumber", "Unknown").toString();
    setState(lineIndex, LineState::DIALING, {
        {"statusMessage", "Dialing " + numberToDial + "..."},
        {"contactInfo", numberToDial}
    });

    // Simulate call connection or failure
    if (m_lines[lineIndex].actionTimer) m_lines[lineIndex].actionTimer->deleteLater();
    m_lines[lineIndex].actionTimer = new QTimer(this);
    m_lines[lineIndex].actionTimer->setSingleShot(true);
    connect(m_lines[lineIndex].actionTimer, &QTimer::timeout, [this, lineIndex, numberToDial]() {
        if (QRandomGenerator::global()->bounded(10) < 2 || numberToDial == "911") { // ~20% chance of failure or specific number
             processCallEvent(lineIndex, LineEvent::SYSTEM_CALL_FAILED, {{"reason", "Line Busy"}});
        } else {
             processCallEvent(lineIndex, LineEvent::SYSTEM_CALL_CONNECTED, {});
        }
    });
    m_lines[lineIndex].actionTimer->start(2000 + QRandomGenerator::global()->bounded(3000)); // 2-5 seconds to connect/fail
}

void CallManager::onIdle_SystemIncomingCall(int lineIndex, const QVariantMap& data) {
    QString callerId = data.value("callerId", QString("Caller %1").arg(QRandomGenerator::global()->bounded(1000,9999))).toString();
    setState(lineIndex, LineState::RINGING_INCOMING, {
        {"statusMessage", "Incoming Call: " + callerId},
        {"contactInfo", callerId}
    });

    // Simulate caller hanging up if not answered (missed call)
    if (m_lines[lineIndex].actionTimer) m_lines[lineIndex].actionTimer->deleteLater();
    m_lines[lineIndex].actionTimer = new QTimer(this);
    m_lines[lineIndex].actionTimer->setSingleShot(true);
    connect(m_lines[lineIndex].actionTimer, &QTimer::timeout, [this, lineIndex]() {
        if(m_lines[lineIndex].currentState == LineState::RINGING_INCOMING) { // If still ringing
            processCallEvent(lineIndex, LineEvent::SYSTEM_REMOTE_HANGUP, {{"reason", "Missed Call"}});
        }
    });
    m_lines[lineIndex].actionTimer->start(15000); // 15 seconds to answer
}

void CallManager::onDialing_SystemCallConnected(int lineIndex, const QVariantMap& data) {
    setState(lineIndex, LineState::CONVERSATION, {
        {"statusMessage", "Connected to " + m_lines[lineIndex].contactInfo}
    });
    // Simulate remote hangup after some conversation time
    if (m_lines[lineIndex].actionTimer) m_lines[lineIndex].actionTimer->deleteLater();
    m_lines[lineIndex].actionTimer = new QTimer(this);
    m_lines[lineIndex].actionTimer->setSingleShot(true);
    connect(m_lines[lineIndex].actionTimer, &QTimer::timeout, [this, lineIndex]() {
         if(m_lines[lineIndex].currentState == LineState::CONVERSATION) { // If still in conversation
            processCallEvent(lineIndex, LineEvent::SYSTEM_REMOTE_HANGUP, {{"reason", "Call ended by remote party"}});
         }
    });
    m_lines[lineIndex].actionTimer->start(10000 + QRandomGenerator::global()->bounded(20000)); // 10-30 second call
}

void CallManager::onDialing_UserHangup(int lineIndex, const QVariantMap& data) {
    if (m_lines[lineIndex].actionTimer) m_lines[lineIndex].actionTimer->stop(); // Stop any pending connection/failure timer
    setState(lineIndex, LineState::HANGING_UP, {{"statusMessage", "Canceling call..."}});
    // Simulate hangup process
    QTimer::singleShot(500, [this, lineIndex]() {
        processCallEvent(lineIndex, LineEvent::SYSTEM_ACTION_COMPLETE, {{"finalStatus", "Call Canceled"}});
    });
}

void CallManager::onDialing_SystemCallFailed(int lineIndex, const QVariantMap& data) {
    QString reason = data.value("reason", "Call Failed").toString();
    setState(lineIndex, LineState::IDLE, {
        {"statusMessage", reason},
        {"contactInfo", ""} // Clear contact info on failure
    });
}

void CallManager::onRingingIncoming_UserAnswer(int lineIndex, const QVariantMap& data) {
    if (m_lines[lineIndex].actionTimer) m_lines[lineIndex].actionTimer->stop(); // Stop missed call timer
    setState(lineIndex, LineState::CONVERSATION, {
        {"statusMessage", "Connected to " + m_lines[lineIndex].contactInfo}
    });
    // Simulate remote hangup (same as for outgoing connected call)
    if (m_lines[lineIndex].actionTimer) m_lines[lineIndex].actionTimer->deleteLater();
    m_lines[lineIndex].actionTimer = new QTimer(this);
    m_lines[lineIndex].actionTimer->setSingleShot(true);
    connect(m_lines[lineIndex].actionTimer, &QTimer::timeout, [this, lineIndex]() {
         if(m_lines[lineIndex].currentState == LineState::CONVERSATION) {
            processCallEvent(lineIndex, LineEvent::SYSTEM_REMOTE_HANGUP, {{"reason", "Call ended by remote party"}});
         }
    });
    m_lines[lineIndex].actionTimer->start(10000 + QRandomGenerator::global()->bounded(20000));
}

void CallManager::onRingingIncoming_UserHangup(int lineIndex, const QVariantMap& data) { // User rejects call
    if (m_lines[lineIndex].actionTimer) m_lines[lineIndex].actionTimer->stop(); // Stop missed call timer
    setState(lineIndex, LineState::HANGING_UP, {{"statusMessage", "Rejecting call..."}});
    QTimer::singleShot(500, [this, lineIndex]() {
        processCallEvent(lineIndex, LineEvent::SYSTEM_ACTION_COMPLETE, {{"finalStatus", "Call Rejected"}});
    });
}

void CallManager::onRingingIncoming_SystemRemoteHangup(int lineIndex, const QVariantMap& data) { // Missed call
    if (m_lines[lineIndex].actionTimer) m_lines[lineIndex].actionTimer->stop();
    setState(lineIndex, LineState::IDLE, {
        {"statusMessage", data.value("reason", "Missed Call").toString()},
        {"contactInfo", ""}
    });
}

void CallManager::onConversation_UserHangup(int lineIndex, const QVariantMap& data) {
    if (m_lines[lineIndex].actionTimer) m_lines[lineIndex].actionTimer->stop(); // Stop any remote hangup timer
    setState(lineIndex, LineState::HANGING_UP, {{"statusMessage", "Hanging up..."}});
    QTimer::singleShot(500, [this, lineIndex]() {
        processCallEvent(lineIndex, LineEvent::SYSTEM_ACTION_COMPLETE, {{"finalStatus", "Call Ended"}});
    });
}

void CallManager::onConversation_SystemRemoteHangup(int lineIndex, const QVariantMap& data) {
    if (m_lines[lineIndex].actionTimer) m_lines[lineIndex].actionTimer->stop();
    setState(lineIndex, LineState::IDLE, {
        {"statusMessage", data.value("reason", "Call Ended by Remote").toString()},
        {"contactInfo", ""}
    });
}

void CallManager::onHangingUp_SystemActionComplete(int lineIndex, const QVariantMap& data) {
    setState(lineIndex, LineState::IDLE, {
        {"statusMessage", data.value("finalStatus", "Idle").toString()},
        {"contactInfo", ""} // Clear contact info
    });
}