#include "messagemanager.h"
#include <QDebug> // For logging

// Constructor: Initializes state, model, and sample messages
MessageManager::MessageManager(QObject *parent)
    : QObject(parent),
      m_currentState(State::VIEW_LIST),
      m_selectedMessageIndex(-1) {
    m_messageListModel = new QStringListModel(this);
    initializeStateMachine(); // Set up state transitions

    // Add some sample messages for demonstration
    m_messagesData.append(std::make_pair(QStringLiteral("Welcome!"), QStringLiteral("Hello and welcome to the messaging system.")));
    m_messagesData.append(std::make_pair(QStringLiteral("Meeting Reminder"), QStringLiteral("Team meeting tomorrow at 10 AM in Conference Room A.")));
    m_messagesData.append(std::make_pair(QStringLiteral("Project Update"), QStringLiteral("The latest project build has been deployed to staging.")));
    updateMessageListModel(); // Populate the QML model

    qDebug() << "MessageManager initialized.";
}

// Defines the state machine transitions and handlers
void MessageManager::initializeStateMachine() {
    // VIEW_LIST State
    m_stateMachine[State::VIEW_LIST][Event::SELECT_MESSAGE_FROM_LIST] = &MessageManager::onViewList_SelectMessage;
    m_stateMachine[State::VIEW_LIST][Event::CREATE_NEW_MESSAGE] = &MessageManager::onViewList_CreateNew;

    // VIEW_MESSAGE State
    m_stateMachine[State::VIEW_MESSAGE][Event::EDIT_SELECTED_MESSAGE] = &MessageManager::onViewMessage_EditSelected;
    m_stateMachine[State::VIEW_MESSAGE][Event::BACK_TO_LIST_FROM_VIEW] = &MessageManager::onViewMessage_BackToList;

    // EDIT_MESSAGE State
    m_stateMachine[State::EDIT_MESSAGE][Event::SAVE_DRAFT] = &MessageManager::onEditMessage_SaveDraft;
    m_stateMachine[State::EDIT_MESSAGE][Event::SEND_MESSAGE] = &MessageManager::onEditMessage_Send;
    m_stateMachine[State::EDIT_MESSAGE][Event::DISCARD_AND_BACK_TO_LIST] = &MessageManager::onEditMessage_DiscardAndBackToList;
}

// Sets the current UI state and emits a signal if it changes
void MessageManager::setState(State newState) {
    if (m_currentState != newState) {
        m_currentState = newState;
        emit stateChanged();
        qDebug() << "MessageManager: State changed to" << static_cast<int>(m_currentState);
    }
}

// Updates the QStringListModel with subjects from m_messagesData
void MessageManager::updateMessageListModel() {
    QStringList subjects;
    for(const auto& msgPair : m_messagesData) {
        subjects.append(msgPair.first); // Add subject to the list
    }
    m_messageListModel->setStringList(subjects); // Update the model for QML
}


void MessageManager::setCurrentMessageSubject(const QString& subject) {
    if (m_currentMessageSubject != subject) {
        m_currentMessageSubject = subject;
        emit currentMessageChanged();
    }
}

void MessageManager::setCurrentMessageBody(const QString& body) {
    if (m_currentMessageBody != body) {
        m_currentMessageBody = body;
        emit currentMessageChanged();
    }
}


// Processes an incoming event based on the current state
void MessageManager::processEvent(Event event, const QVariantMap& data) {
    qDebug() << "MessageManager: Processing event" << static_cast<int>(event)
             << "in state" << static_cast<int>(m_currentState) << "with data" << data;

    auto stateIter = m_stateMachine.find(m_currentState);
    if (stateIter != m_stateMachine.end()) { // Check if current state is valid
        auto eventIter = stateIter->second.find(event);
        if (eventIter != stateIter->second.end()) { // Check if event is valid for current state
            EventHandler handler = eventIter->second; // Get the function pointer
            (this->*handler)(data);                   // Call the handler function
        } else {
            qWarning() << "MessageManager: No handler for event" << static_cast<int>(event)
                       << "in state" << static_cast<int>(m_currentState);
        }
    } else {
        qWarning() << "MessageManager: Unknown current state" << static_cast<int>(m_currentState);
    }
}

// --- Event Handler Implementations ---

void MessageManager::onViewList_SelectMessage(const QVariantMap& data) {
    int index = data.value("index", -1).toInt();
    if (index >= 0 && index < m_messagesData.size()) {
        if(m_selectedMessageIndex != index) {
            m_selectedMessageIndex = index;
            emit selectedMessageIndexChanged();
        }
        setCurrentMessageSubject(m_messagesData[index].first);
        setCurrentMessageBody(m_messagesData[index].second);
        setState(State::VIEW_MESSAGE);
    } else {
        qWarning() << "MessageManager: Invalid index" << index << "for SELECT_MESSAGE_FROM_LIST";
    }
}

void MessageManager::onViewList_CreateNew(const QVariantMap& data) {
    if(m_selectedMessageIndex != -1) {
        m_selectedMessageIndex = -1; // Indicates a new message, not editing an existing one
        emit selectedMessageIndexChanged();
    }
    setCurrentMessageSubject("New Message"); // Default subject for new message
    setCurrentMessageBody("");              // Empty body for new message
    setState(State::EDIT_MESSAGE);
}

void MessageManager::onViewMessage_EditSelected(const QVariantMap& data) {
    if (m_selectedMessageIndex >= 0 && m_selectedMessageIndex < m_messagesData.size()) {
        // Subject and body are already loaded from when message was selected for viewing
        setState(State::EDIT_MESSAGE);
    } else {
        qWarning() << "MessageManager: No message selected to edit. Current index:" << m_selectedMessageIndex;
        // Optionally, could transition to CREATE_NEW_MESSAGE state or show an error
        onViewList_CreateNew({}); // Fallback to creating a new message
    }
}

void MessageManager::onViewMessage_BackToList(const QVariantMap& data) {
    if(m_selectedMessageIndex != -1) {
        m_selectedMessageIndex = -1; // Clear selection
        emit selectedMessageIndexChanged();
    }
    setCurrentMessageSubject(""); // Clear current message buffers
    setCurrentMessageBody("");
    setState(State::VIEW_LIST);
}

void MessageManager::onEditMessage_SaveDraft(const QVariantMap& data) {
    // Subject and body are taken from m_currentMessageSubject/Body,
    // which should be updated by QML text field bindings.
    QString subjectToSave = m_currentMessageSubject;
    QString bodyToSave = m_currentMessageBody;

    if (m_selectedMessageIndex >= 0 && m_selectedMessageIndex < m_messagesData.size()) {
        // Editing an existing message
        m_messagesData[m_selectedMessageIndex] = {subjectToSave, bodyToSave};
    } else {
        // Saving a new message as a draft
        m_messagesData.append({subjectToSave, bodyToSave});
        m_selectedMessageIndex = m_messagesData.size() - 1; // Select the newly saved draft
        emit selectedMessageIndexChanged();
    }
    updateMessageListModel(); // Refresh the list model
    qDebug() << "MessageManager: Draft saved/updated -" << subjectToSave;
    // After saving, transition to viewing the saved/edited message
    setState(State::VIEW_MESSAGE);
}

void MessageManager::onEditMessage_Send(const QVariantMap& data) {
    QString subjectToSend = m_currentMessageSubject;
    QString bodyToSend = m_currentMessageBody;

    qDebug() << "MessageManager: Attempting to send message -" << subjectToSend;
    // Simulate sending process
    bool sendSuccess = !subjectToSend.trimmed().isEmpty(); // Simple success condition: subject not empty

    if (sendSuccess) {
        // If it was a new message (not an existing draft being edited), add it to messagesData.
        // If it was an existing draft, it's already updated or was just saved.
        if (m_selectedMessageIndex == -1 ||
            (m_selectedMessageIndex >=0 && m_messagesData[m_selectedMessageIndex].first != subjectToSend) || // if it's a new subject for existing index
            (m_selectedMessageIndex >=0 && m_messagesData[m_selectedMessageIndex].second != bodyToSend) ) { // or new body

            // Check if this exact message (subject + body) already exists to avoid duplicates if user hits send multiple times on an unsaved new message
            bool alreadyExists = false;
            for(const auto& msg : m_messagesData) {
                if (msg.first == subjectToSend && msg.second == bodyToSend) {
                    alreadyExists = true;
                    break;
                }
            }
            if (!alreadyExists) {
                 m_messagesData.append({subjectToSend, bodyToSend});
                 updateMessageListModel();
            }
        }
        emit messageSent(true, "Message sent successfully!");
        onViewMessage_BackToList({}); // Go back to list after sending
    } else {
        emit messageSent(false, "Failed to send: Subject cannot be empty.");
        // Stay in EDIT_MESSAGE state for user to correct
    }
}

void MessageManager::onEditMessage_DiscardAndBackToList(const QVariantMap& data) {
    // No changes are saved
    if(m_selectedMessageIndex != -1) {
        m_selectedMessageIndex = -1; // Clear selection
        emit selectedMessageIndexChanged();
    }
    setCurrentMessageSubject(""); // Clear current message buffers
    setCurrentMessageBody("");
    setState(State::VIEW_LIST);
}