
import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts 6.2

ScrollView {
    id: scrollView
    width: parent.width
    // height: parent.height // Let GridView determine its height within ScrollView
    clip: true // Important for ScrollView
    ScrollBar.vertical.policy: ScrollBar.AsNeeded
    ScrollBar.horizontal.policy: ScrollBar.AsNeeded

    GridView {
        id: channelGrid
        width: scrollView.availableWidth // Use availableWidth for ScrollView content
        // height: contentHeight // GridView calculates its own content height

        // Adjust cellWidth for responsiveness, aiming for roughly 4 columns
        // but ensuring a minimum sensible width.
        property int idealColumns: Math.max(1, Math.floor(width / 200)) // Min width 200 per cell
        cellWidth: width / idealColumns - (idealColumns > 1 ? 5 : 0) // Subtract spacing if multiple columns
        cellHeight: 75 // Fixed height for each channel item

        model: channelModel // C++ model instance

        // Delegate for each channel item
        delegate: Frame {
            width: channelGrid.cellWidth - 10 // Leave some margin
            height: channelGrid.cellHeight - 10
            padding: 8
            background: Rectangle {
                color: model.isOn ? "#E8F5E9" : "#FFEBEE" // Greenish if on, Reddish if off
                radius: 5
                border.color: model.isOn ? "#4CAF50" : "#F44336"
                border.width: 1
            }

            RowLayout {
                anchors.fill: parent
                spacing: 10

                Label {
                    text: model.channelName // From ChannelModel's roleNames
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 14
                    color: "#212121"
                }
                Switch {
                    id: channelSwitch
                    checked: model.isOn // Bind to model's 'isOn' role
                    // When clicked, call the model's method to toggle the state.
                    // The model will then handle communication with hardware and update 'isOn'.
                    onClicked: {
                        console.log("ChannelSwitch for '" + model.channelName + "' (ID: " + model.channelId + ") clicked. Current state: " + checked);
                        channelModel.toggleChannelViaModel(model.channelId);
                    }
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                    // Visual feedback for the switch itself
                    background: Rectangle {
                        x: channelSwitch.leftPadding
                        y: channelSwitch.topPadding + (channelSwitch.availableHeight - height) / 2
                        implicitWidth: 50
                        implicitHeight: 26
                        radius: 13
                        color: channelSwitch.checked ? "#4CAF50" : "#BDBDBD" // Green when on, Gray when off
                        border.color: channelSwitch.checked ? "#388E3C" : "#757575"
                    }
                    indicator: Rectangle {
                        x: channelSwitch.checked ? parent.width - width - channelSwitch.rightPadding - 3 : channelSwitch.leftPadding + 3
                        y: channelSwitch.topPadding + (channelSwitch.availableHeight - height) / 2 + 3
                        width: 20
                        height: 20
                        radius: 10
                        color: "white"
                        Behavior on x { SpringAnimation { spring: 2; damping: 0.2 } }
                    }
                }
            }
        }
    }
}