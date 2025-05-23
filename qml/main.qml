
import QtQuick 6.2 // Use 2.15 for Qt 5.15
import QtQuick.Controls 6.2 // Use 2.15 for Qt 5.15
import QtQuick.Layouts 6.2 // Use 1.15 for Qt 5.15
import com.company.enums 1.0 // Custom namespace for C++ enums

ApplicationWindow {
    id: rootWindow
    visible: true
    width: 1024
    height: 768
    title: "Home Automation System"
    color: "#F0F0F0" // Light background for the window

    // Font for the application (optional, can be set on specific controls)
    font.family: "Arial" // Choose a common font

    // Header showing the current mode
    header: ToolBar {
        background: Rectangle {
            color: "#333" // Darker toolbar
        }
        Label {
            id: titleLabel
            text: {
                switch (mainController.currentMode) {
                    case MainControllerEnums.Home: return "Home Dashboard";
                    case MainControllerEnums.Call: return "Call Interface";
                    case MainControllerEnums.Message: return "Messaging";
                    case MainControllerEnums.Camera: return "Camera Control";
                    case MainControllerEnums.VideoPlayer: return "Video Player";
                    default: return "Home Automation";
                }
            }
            font.bold: true
            font.pixelSize: 18
            color: "white"
            anchors.centerIn: parent
        }
    }

    // Footer with TabBar for navigation
    footer: TabBar {
        id: mainTabBar
        currentIndex: mainStackLayout.currentIndex // Sync with StackLayout
        background: Rectangle { color: "#E0E0E0" } // Lighter tab bar background

        Repeater {
            model: ["Home", "Call", "Message", "Camera", "Video"]
            delegate: TabButton {
                text: modelData
                font.pixelSize: 14
                background: Rectangle {
                    color: TabButton.checked ? "#BBDEFB" : "transparent" // Highlight checked tab
                    border.color: TabButton.checked ? "#2196F3" : "transparent"
                    border.width: 2
                }
                onClicked: {
                    switch (index) {
                        case 0: mainController.selectMode(MainControllerEnums.Home); break;
                        case 1: mainController.selectMode(MainControllerEnums.Call); break;
                        case 2: mainController.selectMode(MainControllerEnums.Message); break;
                        case 3: mainController.selectMode(MainControllerEnums.Camera); break;
                        case 4: mainController.selectMode(MainControllerEnums.VideoPlayer); break;
                    }
                }
            }
        }
    }

    // StackLayout to switch between different views
    StackLayout {
        id: mainStackLayout
        anchors.fill: parent
        currentIndex: 0 // Default to Home view

        // Connection to update StackLayout when C++ changes the mode
        Connections {
            target: mainController
            function onCurrentModeChanged(mode) {
                var newIndex = 0;
                switch (mode) {
                    case MainControllerEnums.Home: newIndex = 0; break;
                    case MainControllerEnums.Call: newIndex = 1; break;
                    case MainControllerEnums.Message: newIndex = 2; break;
                    case MainControllerEnums.Camera: newIndex = 3; break;
                    case MainControllerEnums.VideoPlayer: newIndex = 4; break;
                }
                if (mainStackLayout.currentIndex !== newIndex) {
                    mainStackLayout.currentIndex = newIndex;
                }
                // Ensure TabBar is also synced if mode changes programmatically
                if (mainTabBar.currentIndex !== newIndex) {
                     mainTabBar.currentIndex = newIndex;
                }
            }
        }

        // Views for each mode
        HomeView { id: homeView }
        CallView { id: callView }
        MessageView { id: messageView }
        CameraView { id: cameraView }
        VideoPlayerView { id: videoPlayerView }
    }

    // Example: Simulate an incoming call periodically for testing
    Timer {
        interval: 45000 // Every 45 seconds
        running: true   // Set to false to disable
        repeat: true
        onTriggered: {
            // Only trigger if not already in call view and a line is idle (conceptual)
            // This is a very basic simulation trigger.
            if (callManager && mainController.currentMode !== MainControllerEnums.Call) {
                // In a real app, HardwareManager would emit a signal for incoming calls.
                // This simulates such a signal.
                console.log("QML Timer: Simulating incoming call on line 0 via CallManager.");
                // Find an idle line to simulate on, e.g., line 0 for simplicity
                // A more robust check would involve querying line states from callManager if possible
                callManager.processCallEvent(0, CallManagerEnums.SYSTEM_INCOMING_CALL,
                                             {"callerId": "Simulated Caller " + Math.floor(Math.random()*1000)});
            }
        }
    }
}