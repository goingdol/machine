
import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts 6.2
import com.company.enums 1.0 // For CallManagerEnums

Frame {
    id: lineDisplayFrame
    property int lineNumber // Passed from CallView repeater's index
    // Properties to hold the state and info for this specific line, updated by Connections
    property string currentContactInfo: ""
    property string currentStatusMessage: "Idle"
    property int currentLineState: CallManagerEnums.IDLE // Default state

    implicitWidth: 320 // Minimum width for the display
    // implicitHeight will be determined by content

    // Visual feedback based on line state
    background: Rectangle {
        color: {
            switch (lineDisplayFrame.currentLineState) {
                case CallManagerEnums.CONVERSATION: return "#C8E6C9"; // Light green
                case CallManagerEnums.RINGING_INCOMING: return "#FFF9C4"; // Light yellow
                case CallManagerEnums.DIALING: return "#BBDEFB"; // Light blue
                case CallManagerEnums.HANGING_UP: return "#FFCCBC"; // Light red/orange
                default: return "#FAFAFA"; // Default (Idle)
            }
        }
        border.color: {
            switch (lineDisplayFrame.currentLineState) {
                case CallManagerEnums.CONVERSATION: return "#4CAF50";
                case CallManagerEnums.RINGING_INCOMING: return "#FFEB3B";
                case CallManagerEnums.DIALING: return "#2196F3";
                case CallManagerEnums.HANGING_UP: return "#FF5722";
                default: return "#BDBDBD";
            }
        }
        border.width: 1
        radius: 6
    }

    // Connections to the C++ CallManager to receive state updates for this line
    Connections {
        target: callManager // Assumes callManager is globally accessible context property
        function onCallLineStateChanged(idx, newState, details) {
            if (idx === lineDisplayFrame.lineNumber) { // Check if update is for this line
                console.log("CallLineDisplay " + lineNumber + " received state: " + newState +
                            ", Status: " + details.statusMessage + ", Contact: " + details.contactInfo);
                lineDisplayFrame.currentLineState = newState;
                lineDisplayFrame.currentStatusMessage = details.statusMessage || "Status N/A";
                lineDisplayFrame.currentContactInfo = details.contactInfo || "";

                // Clear phone number input after initiating a dial or if call ends/fails
                if (newState === CallManagerEnums.DIALING ||
                    newState === CallManagerEnums.IDLE ||
                    newState === CallManagerEnums.HANGING_UP) {
                    phoneNumberInput.text = "";
                }
            }
        }
    }

    // Layout for the content of the call line display
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Label {
            text: "Line " + (lineDisplayFrame.lineNumber + 1)
            font.bold: true
            font.pixelSize: 16
            color: "#333"
            Layout.alignment: Qt.AlignHCenter
        }
        Label { // Displays current status (e.g., "Dialing...", "Connected")
            id: statusLabel
            text: lineDisplayFrame.currentStatusMessage
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 14
            color: "#555"
        }
        Label { // Displays contact info (caller ID, number dialed)
            id: contactLabel
            text: lineDisplayFrame.currentContactInfo
            visible: lineDisplayFrame.currentContactInfo !== ""
            font.italic: true
            font.pixelSize: 13
            color: "#777"
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }

        // Input field for dialing, visible only when line is IDLE
        TextField {
            id: phoneNumberInput
            placeholderText: "Enter number to dial"
            visible: lineDisplayFrame.currentLineState === CallManagerEnums.IDLE
            Layout.fillWidth: true
            font.pixelSize: 14
            onAccepted: dialButton.clicked() // Allow Enter key to trigger dialing
        }

        // Row for action buttons
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 10

            // Dial button: Visible in IDLE state
            Button {
                id: dialButton
                text: "Dial"
                visible: lineDisplayFrame.currentLineState === CallManagerEnums.IDLE
                enabled: phoneNumberInput.text.length > 0 // Enable only if number is entered
                icon.source: "qrc:/icons/call-outline.svg" // Example icon path (add SVGs to qrc)
                onClicked: {
                    callManager.processCallEvent(lineDisplayFrame.lineNumber,
                                                 CallManagerEnums.USER_INITIATE_DIAL,
                                                 {"phoneNumber": phoneNumberInput.text});
                }
            }
            // Answer button: Visible when RINGING_INCOMING
            Button {
                id: answerButton
                text: "Answer"
                visible: lineDisplayFrame.currentLineState === CallManagerEnums.RINGING_INCOMING
                icon.source: "qrc:/icons/call.svg"
                highlighted: true // Make it stand out
                onClicked: {
                    callManager.processCallEvent(lineDisplayFrame.lineNumber,
                                                 CallManagerEnums.USER_ANSWER_CALL, {});
                }
            }
            // Hang Up / Reject button: Visible in various active call states
            Button {
                id: hangupButton
                text: lineDisplayFrame.currentLineState === CallManagerEnums.RINGING_INCOMING ? "Reject" : "Hang Up"
                visible: lineDisplayFrame.currentLineState === CallManagerEnums.DIALING ||
                         lineDisplayFrame.currentLineState === CallManagerEnums.CONVERSATION ||
                         lineDisplayFrame.currentLineState === CallManagerEnums.RINGING_INCOMING ||
                         lineDisplayFrame.currentLineState === CallManagerEnums.HANGING_UP // Can be visible if hangup takes time
                icon.source: "qrc:/icons/call-end-outline.svg"
                enabled: lineDisplayFrame.currentLineState !== CallManagerEnums.HANGING_UP // Disable if already hanging up
                background: Rectangle { // Custom background to make it red
                    color: hangupButton.enabled ? "#F44336" : "#EF9A9A"
                    radius: 3
                    border.color: "#D32F2F"
                    border.width: 1
                }
                contentItem: Text {
                    text: hangupButton.text
                    font: hangupButton.font
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    callManager.processCallEvent(lineDisplayFrame.lineNumber,
                                                 CallManagerEnums.USER_HANGUP, {});
                }
            }
        }
    }
}