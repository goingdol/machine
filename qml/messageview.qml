
import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts 6.2
import com.company.enums 1.0 // For MessageManagerEnums

Item {
    id: messageRoot
    anchors.fill: parent

    // Connections to MessageManager for state and data updates
    Connections {
        target: messageManager
        function onStateChanged() {
            console.log("MessageView: C++ State changed to " + messageManager.currentState);
            // StackLayout currentIndex is now bound directly to manager's state
        }
        function onCurrentMessageChanged() {
            // Update text fields only if in edit mode and text differs, to avoid cursor jumps
            if (messageManager.currentState === MessageManagerEnums.EDIT_MESSAGE) {
                if (subjectField.text !== messageManager.currentMessageSubject) {
                    subjectField.text = messageManager.currentMessageSubject;
                }
                if (bodyArea.text !== messageManager.currentMessageBody) {
                    bodyArea.text = messageManager.currentMessageBody;
                }
            }
        }
        function onMessageSent(success, info) { // Display feedback on message send attempt
            sentStatusLabel.text = info;
            sentStatusLabel.color = success ? "green" : "red";
            sentStatusPopup.open();
        }
    }

    // Popup for displaying message send status
    Popup {
        id: sentStatusPopup
        x: (messageRoot.width - width) / 2
        y: (messageRoot.height - height) / 2
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside | Popup.CloseOnTimeout
       // timeout: 3000 // Auto-close after 3 seconds
       //  padding: 20
        background: Frame {} // Use Frame for standard popup appearance
        Label { id: sentStatusLabel; font.pixelSize: 14 }
    }

    // StackLayout to switch between message list, view message, and edit message screens
    StackLayout {
        id: messageStack
        anchors.fill: parent
        // Bind currentIndex to a transformation of MessageManager's state
        currentIndex: {
            switch (messageManager.currentState) {
                case MessageManagerEnums.VIEW_LIST: return 0;
                case MessageManagerEnums.VIEW_MESSAGE: return 1;
                case MessageManagerEnums.EDIT_MESSAGE: return 2;
                default: return 0; // Fallback to list view
            }
        }

        // -------- View List State (Index 0) --------
        Item {
            id: viewListPage
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 15

                Label {
                    text: "Inbox"
                    font.pixelSize: 20; font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                    color: "#333"
                }
                ListView {
                    id: messageListView
                    model: messageManager.messageListModel // C++ QStringListModel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    // ScrollBar.vertical.policy: ScrollBar.AsNeeded
                    delegate: ItemDelegate {
                        text: modelData // modelData is the subject string from QStringListModel
                        width: parent.width
                        font.pixelSize: 14
                        highlighted: ListView.isCurrentItem // Visual feedback for selection
                        background: Rectangle {
                            color: ItemDelegate.highlighted ? "#E3F2FD" : "transparent"
                        }
                        onClicked: {
                            messageListView.currentIndex = index; // Visually select in QML
                            messageManager.processEvent(MessageManagerEnums.SELECT_MESSAGE_FROM_LIST, {"index": index});
                        }
                    }
                    ScrollIndicator.vertical: ScrollIndicator {} // For touch scrolling
                }
                Button {
                    text: "Compose New Message"
                    Layout.alignment: Qt.AlignHCenter
                    icon.source: "qrc:/icons/create-outline.svg" // Example icon
                    onClicked: messageManager.processEvent(MessageManagerEnums.CREATE_NEW_MESSAGE, {})
                }
            }
        }

        // -------- View Message State (Index 1) --------
        Item {
            id: viewMessagePage
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 10

                Label { text: "View Message"; font.pixelSize: 20; font.bold: true; Layout.alignment: Qt.AlignHCenter; color: "#333" }
                Label {
                    text: "Subject: " + messageManager.currentMessageSubject
                    wrapMode: Text.WordWrap
                    font.pixelSize: 15; font.bold: true
                    color: "#444"
                }
                TextArea { // Read-only display of message body
                    id: viewBodyArea
                    readOnly: true
                    text: messageManager.currentMessageBody
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: Text.WordWrap
                    font.pixelSize: 14
                    background: Rectangle { color: "#F5F5F5"; radius: 4; border.color: "#DDD" }
                }
                 RowLayout { // Action buttons for viewing message
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 10
                    Button {
                        text: "Edit"
                        icon.source: "qrc:/icons/pencil-outline.svg"
                        onClicked: messageManager.processEvent(MessageManagerEnums.EDIT_SELECTED_MESSAGE, {})
                    }
                    Button {
                        text: "Back to List"
                        icon.source: "qrc:/icons/arrow-back-outline.svg"
                        onClicked: messageManager.processEvent(MessageManagerEnums.BACK_TO_LIST_FROM_VIEW, {})
                    }
                }
            }
        }

        // -------- Edit Message State (Index 2) --------
        Item {
            id: editMessagePage
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 10

                Label { text: "Compose / Edit Message"; font.pixelSize: 20; font.bold: true; Layout.alignment: Qt.AlignHCenter; color: "#333" }
                TextField { // For message subject
                    id: subjectField
                    placeholderText: "Subject"
                    text: messageManager.currentMessageSubject // Two-way binding (implicit via onTextChanged)
                    Layout.fillWidth: true
                    font.pixelSize: 14
                    // Update C++ model property when text changes
                    onTextChanged: messageManager.currentMessageSubject = text
                }
                TextArea { // For message body
                    id: bodyArea
                    placeholderText: "Enter message body here..."
                    text: messageManager.currentMessageBody // Two-way binding
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: Text.WordWrap
                    font.pixelSize: 14
                    // Update C++ model property when text changes
                    onTextChanged: messageManager.currentMessageBody = text
                }
                RowLayout { // Action buttons for editing message
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 10
                    Button {
                        text: "Save Draft"
                        icon.source: "qrc:/icons/save-outline.svg"
                        onClicked: messageManager.processEvent(MessageManagerEnums.SAVE_DRAFT, {})
                    }
                    Button {
                        text: "Send"
                        icon.source: "qrc:/icons/send-outline.svg"
                        highlighted: true // Make send button prominent
                        onClicked: messageManager.processEvent(MessageManagerEnums.SEND_MESSAGE, {})
                    }
                    Button {
                        text: "Discard"
                        icon.source: "qrc:/icons/trash-outline.svg"
                        onClicked: messageManager.processEvent(MessageManagerEnums.DISCARD_AND_BACK_TO_LIST, {})
                    }
                }
            }
        }
    }
}