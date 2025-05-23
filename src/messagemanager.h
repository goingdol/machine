
#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include <QObject>
#include <QStringListModel> // For displaying list of message subjects
#include <QVariantMap>    // For passing data with events
#include <QList>          // For storing messages
#include <QPair>          // For storing (Subject, Body) pairs
#include <map>            // For state machine
#include <functional>     // For std::function
#include <QQmlEngine>     // For QML_ELEMENT

// Manages message viewing, editing, and sending logic
class MessageManager : public QObject {
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_ELEMENT
#endif

    // Properties exposed to QML for data binding
    Q_PROPERTY(QStringListModel* messageListModel READ messageListModel CONSTANT)
    Q_PROPERTY(QString currentMessageSubject READ currentMessageSubject WRITE setCurrentMessageSubject NOTIFY currentMessageChanged)
    Q_PROPERTY(QString currentMessageBody READ currentMessageBody WRITE setCurrentMessageBody NOTIFY currentMessageChanged)
    Q_PROPERTY(State currentState READ currentState NOTIFY stateChanged)
    Q_PROPERTY(int selectedMessageIndex READ selectedMessageIndex NOTIFY selectedMessageIndexChanged)


public:
    // UI states for the messaging feature
    enum class State { VIEW_LIST, VIEW_MESSAGE, EDIT_MESSAGE };
    Q_ENUM(State)

    // Events that can occur in the messaging feature
    enum class Event {
        SELECT_MESSAGE_FROM_LIST, // User selects a message from the list
        CREATE_NEW_MESSAGE,       // User wants to compose a new message
        EDIT_SELECTED_MESSAGE,    // User wants to edit the currently viewed message
        SAVE_DRAFT,               // User saves the current message draft
        SEND_MESSAGE,             // User sends the current message
        DISCARD_AND_BACK_TO_LIST, // User discards changes and returns to list
        BACK_TO_LIST_FROM_VIEW    // User goes back to list from viewing a message
    };
    Q_ENUM(Event)

    explicit MessageManager(QObject *parent = nullptr);

    // Accessors for Q_PROPERTY
    QStringListModel* messageListModel() const { return m_messageListModel; }
    QString currentMessageSubject() const { return m_currentMessageSubject; }
    QString currentMessageBody() const { return m_currentMessageBody; }
    State currentState() const { return m_currentState; }
    int selectedMessageIndex() const { return m_selectedMessageIndex; }


    void setCurrentMessageSubject(const QString& subject);
    void setCurrentMessageBody(const QString& body);

    // Q_INVOKABLE method for QML to trigger message events
    Q_INVOKABLE void processEvent(Event event, const QVariantMap& data = {});

signals:
    void currentMessageChanged(); // Emitted when subject or body of current message changes
    void stateChanged();          // Emitted when UI state changes
    void messageSent(bool success, const QString& info); // Emitted after attempting to send a message
    void selectedMessageIndexChanged();


private:
    // Type alias for state event handler function pointer
    using EventHandler = void (MessageManager::*)(const QVariantMap& data);
    // State machine: maps (CurrentState, Event) to a handler function
    std::map<State, std::map<Event, EventHandler>> m_stateMachine;

    void initializeStateMachine(); // Sets up the state machine
    void setState(State newState); // Helper to change state and emit signal
    void updateMessageListModel(); // Refreshes the QStringListModel for QML

    // --- State Handler Function Declarations ---
    void onViewList_SelectMessage(const QVariantMap& data);
    void onViewList_CreateNew(const QVariantMap& data);

    void onViewMessage_EditSelected(const QVariantMap& data);
    void onViewMessage_BackToList(const QVariantMap& data);

    void onEditMessage_SaveDraft(const QVariantMap& data);
    void onEditMessage_Send(const QVariantMap& data);
    void onEditMessage_DiscardAndBackToList(const QVariantMap& data);

    State m_currentState;                       // Current UI state
    QStringListModel* m_messageListModel;       // Model for QML ListView
    QList<QPair<QString, QString>> m_messagesData; // Internal storage for messages (Subject, Body)
    int m_selectedMessageIndex;                 // Index of the currently selected/viewed message (-1 if none)

    // Buffers for the message being viewed or edited
    QString m_currentMessageSubject;
    QString m_currentMessageBody;
};

#endif // MESSAGEMANAGER_H