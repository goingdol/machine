import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts 6.2
import QtMultimedia 6.2 // For VideoOutput and MediaPlayer QML types
import com.company.enums 1.0 // For VideoPlayerManagerEnums

Item {
    id: videoRoot
    anchors.fill: parent

    // Function to format duration from milliseconds to MM:SS or HH:MM:SS
    function formatMediaTime(ms) {
        if (isNaN(ms) || ms < 0) return "00:00";
        var totalSeconds = Math.floor(ms / 1000);
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

    // Connections to VideoPlayerManager for state and data updates
    Connections {
        target: videoPlayerManager
        function onStateChanged() {
            console.log("VideoPlayerView: C++ UI State changed to " + videoPlayerManager.currentState);
        }
        function onCurrentVideoChanged() {
            console.log("VideoPlayerView: Current video changed to: " + videoPlayerManager.currentVideoTitle +
                        " URL: " + videoPlayerManager.currentVideoUrl);
            // VideoOutput source is now bound directly to videoPlayerManager.mediaPlayerObject
        }
        function onPlaybackStateChanged() { // Update play/pause button text
            playPauseButton.text = videoPlayerManager.playbackState === MediaPlayer.PlayingState ? "Pause" : "Play";
            playPauseButton.icon.source = videoPlayerManager.playbackState === MediaPlayer.PlayingState ? "qrc:/icons/pause-circle-outline.svg" : "qrc:/icons/play-circle-outline.svg";
        }
        function onDurationChanged() {
            console.log("VideoPlayerView: Duration changed to " + videoPlayerManager.duration);
        }
        function onPositionChanged() {
            // Slider value is bound, no explicit action needed here unless for logging
            // console.log("VideoPlayerView: Position changed to " + videoPlayerManager.position);
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        Label {
            text: "Video Player"
            font.pixelSize: 22; font.bold: true
            Layout.alignment: Qt.AlignHCenter
            color: "#333"
        }

        // VideoOutput to display video from the C++ QMediaPlayer
        VideoOutput {
            id: videoOutput
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(videoRoot.height * 0.5, 400) // Responsive height
            // source: videoPlayerManager.mediaPlayerObject // CRITICAL: Binds to the C++ QMediaPlayer instance

            // Overlay to show title or status when no video is playing or on error
            Rectangle {
                anchors.fill: parent
                color: "black"
                // Visible if no valid source, or if player is stopped and no title, or error state
                visible: videoPlayerManager.playbackState === MediaPlayer.StoppedState && videoPlayerManager.currentVideoTitle === "" ||
                         videoPlayerManager.currentState === VideoPlayerManagerEnums.ERROR ||
                         !videoPlayerManager.mediaPlayerObject // Failsafe
                Label {
                    text: videoPlayerManager.currentState === VideoPlayerManagerEnums.ERROR ?
                          ("Error: " + videoPlayerManager.currentVideoTitle) : // Show "Error" or actual error message if available
                          (videoPlayerManager.currentVideoTitle || "No Video Selected")
                    color: "white"
                    font.pixelSize: 16
                    wrapMode: Text.WordWrap
                    anchors.centerIn: parent
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        Label { // Displays the title of the currently playing/selected video
            id: videoTitleLabel
            text: "Now Playing: " + (videoPlayerManager.currentVideoTitle || "None")
            Layout.alignment: Qt.AlignHCenter
            font.pixelSize: 14
            color: "#444"
            wrapMode: Text.WordWrap
            elide: Text.ElideRight
        }

        // Slider for seeking through the video
        Slider {
            id: positionSlider
            Layout.fillWidth: true
            from: 0
            to: videoPlayerManager.duration > 0 ? videoPlayerManager.duration : 1 // Avoid to=0 if duration is 0
            value: videoPlayerManager.position
            enabled: videoPlayerManager.duration > 0 && videoPlayerManager.playbackState !== MediaPlayer.StoppedState

            // When user finishes dragging the slider, seek in the video
            onMoved: videoPlayerManager.seek(value)
            // Live update while dragging (can be performance intensive for some videos/systems)
            // onValueChanged: if (pressed) videoPlayerManager.seek(value)
        }
        // Labels for current position and total duration
        RowLayout {
            Layout.fillWidth: true
            Label { text: formatMediaTime(videoPlayerManager.position); font.pixelSize: 12; color: "#666" }
            Item { Layout.fillWidth: true } // Spacer
            Label { text: formatMediaTime(videoPlayerManager.duration); font.pixelSize: 12; color: "#666" }
        }

        // Playback control buttons
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 12
            Button { // Play/Pause button
                id: playPauseButton
                text: videoPlayerManager.playbackState === MediaPlayer.PlayingState ? "Pause" : "Play"
                icon.source: videoPlayerManager.playbackState === MediaPlayer.PlayingState ? "qrc:/icons/pause-circle-outline.svg" : "qrc:/icons/play-circle-outline.svg"
                enabled: videoPlayerManager.currentVideoUrl !== "" || // Enable if a video is loaded
                           (videoPlayerManager.videoListModel.rowCount > 0 && videoPlayerManager.currentVideoIndex !== -1) // Or if an index is selected
                font.pixelSize: 14
                onClicked: {
                    if (videoPlayerManager.playbackState === MediaPlayer.PlayingState) {
                        videoPlayerManager.pause();
                    } else {
                        videoPlayerManager.play(); // Will play current or first if none selected yet
                    }
                }
            }
            Button { // Stop button
                text: "Stop"
                icon.source: "qrc:/icons/stop-circle-outline.svg"
                enabled: videoPlayerManager.playbackState !== MediaPlayer.StoppedState && videoPlayerManager.currentVideoUrl !== ""
                font.pixelSize: 14
                onClicked: videoPlayerManager.stop()
            }
        }

        // Label for the video list
        Label { text: "Available Videos:"; font.bold: true; font.pixelSize: 15; Layout.topMargin: 15; color: "#333" }
        // ListView to display available video files
        ListView {
            id: videoFileListView
            model: videoPlayerManager.videoListModel // C++ QStringListModel
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(100, videoRoot.height * 0.2) // Responsive height, min 100px
            clip: true
            // ScrollBar.vertical.policy: ScrollBar.AsNeeded
            ScrollIndicator.vertical: ScrollIndicator {}

            delegate: ItemDelegate {
                text: modelData // Subject string from the model
                width: parent.width
                font.pixelSize: 13
                highlighted: ListView.isCurrentItem // Visual feedback for selection
                background: Rectangle {
                    color: ItemDelegate.highlighted ? "#E3F2FD" : "transparent"
                }
                onClicked: {
                    videoFileListView.currentIndex = index; // Visually select in QML
                    videoPlayerManager.playFileByIndex(index); // Tell C++ to play this file
                }
            }
        }
    }
}