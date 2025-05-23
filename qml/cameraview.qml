
import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts 6.2
import com.company.enums 1.0 // For CameraManagerEnums
// import QtMultimedia 6.2 // Needed if using QML Camera type for actual preview

Item {
    id: cameraRoot
    anchors.fill: parent

    // Function to format recording time from seconds to HH:MM:SS or MM:SS
    function formatTime(totalSeconds) {
        if (isNaN(totalSeconds) || totalSeconds < 0) return "00:00";
        var hours = Math.floor(totalSeconds / 3600);
        var minutes = Math.floor((totalSeconds % 3600) / 60);
        var seconds = totalSeconds % 60;

        var timeString = "";
        if (hours > 0) {
            timeString += String(hours).padStart(2, '0') + ":";
        }
        timeString += String(minutes).padStart(2, '0') + ":" + String(seconds).padStart(2, '0');
        return timeString;
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        Label {
            text: "Camera Control"
            font.pixelSize: 22; font.bold: true
            Layout.alignment: Qt.AlignHCenter
            color: "#333"
        }

        // Placeholder for camera preview area
        Frame {
            id: cameraPreviewFrame
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(cameraRoot.height * 0.6, 480) // Responsive height
            background: Rectangle { color: "#212121" } // Dark background for preview area
            clip: true // Clip content like labels if they overflow

            // --- Actual Camera Preview (requires more setup) ---
            // If you have a C++ QAbstractVideoSurface or QVideoSink:
            // VideoOutput {
            //     anchors.fill: parent
            //     source: cameraManager.videoSurface // Assuming cameraManager exposes this
            // }
            // Or if using QML Camera type:
            // Camera { id: qmlCameraRef; /* ... configuration ... */ }
            // VideoOutput { anchors.fill: parent; source: qmlCameraRef; }
            // For now, a label indicates the state.

            Label { // Status label inside preview area
                text: cameraManager.currentState === CameraManagerEnums.IDLE ? "Preview Off" :
                      cameraManager.currentState === CameraManagerEnums.PREVIEW ? "Live Preview Active" :
                      "RECORDING"
                anchors.centerIn: parent
                color: "white"
                font.pixelSize: 18
                font.bold: cameraManager.isRecording
            }
            Label { // Recording time display
                id: recordingTimeLabel
                text: cameraManager.isRecording ? formatTime(cameraManager.recordingTime) : ""
                visible: cameraManager.isRecording
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.margins: 10
                color: "#F44336" // Red color for recording time
                font.pixelSize: 16
                font.bold: true
                background: Rectangle { // Semi-transparent background for readability
                    // color: "rgba(0, 0, 0, 0.5)"
                    color: "red"
                    radius: 3
                    // padding: 2
                    visible: recordingTimeLabel.text !== ""
                }
            }
        }

        // Row for control buttons
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 15

            Button { // Start/Stop Preview button
                id: previewButton
                text: (cameraManager.currentState === CameraManagerEnums.PREVIEW ||
                       cameraManager.currentState === CameraManagerEnums.RECORDING) ? "Stop Preview" : "Start Preview"
                // Cannot stop preview if currently recording; must stop recording first.
                enabled: cameraManager.currentState !== CameraManagerEnums.RECORDING ||
                         // Allow "Stop Preview" only if it's just previewing
                         (cameraManager.currentState === CameraManagerEnums.PREVIEW && !cameraManager.isRecording)
                icon.source: (cameraManager.currentState === CameraManagerEnums.PREVIEW ||
                              cameraManager.currentState === CameraManagerEnums.RECORDING) ? "qrc:/icons/videocam-off-outline.svg" : "qrc:/icons/videocam-outline.svg"
                onClicked: {
                    if (cameraManager.currentState === CameraManagerEnums.PREVIEW) {
                        cameraManager.processEvent(CameraManagerEnums.STOP_PREVIEW);
                    } else if (cameraManager.currentState === CameraManagerEnums.IDLE) {
                        cameraManager.processEvent(CameraManagerEnums.START_PREVIEW);
                    }
                }
            }

            Button { // Start/Stop Recording button
                id: recordButton
                text: cameraManager.isRecording ? "Stop Recording" : "Start Recording"
                // Enable recording only if in PREVIEW state and not already recording,
                // or if already recording (to allow stopping).
                enabled: (cameraManager.currentState === CameraManagerEnums.PREVIEW && !cameraManager.isRecording) ||
                         cameraManager.isRecording
                icon.source: cameraManager.isRecording ? "qrc:/icons/stop-circle-outline.svg" : "qrc:/icons/radio-button-on-outline.svg"
                highlighted: cameraManager.isRecording // Visually indicate recording
                // Custom styling for recording button
                background: Rectangle {
                    color: recordButton.highlighted ? "#E53935" : (recordButton.enabled ? "#4CAF50" : "#BDBDBD")
                    radius: 4
                    border.color: recordButton.highlighted ? "#C62828" : (recordButton.enabled ? "#388E3C" : "#757575")
                    border.width: 1
                }
                contentItem: Text {
                    text: recordButton.text
                    font: recordButton.font
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    if (cameraManager.isRecording) {
                        cameraManager.processEvent(CameraManagerEnums.STOP_RECORDING);
                    } else {
                        cameraManager.processEvent(CameraManagerEnums.START_RECORDING);
                    }
                }
            }
        }
    }
}