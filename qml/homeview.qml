
import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts 6.2

Item {
    id: homeRoot
    anchors.fill: parent

    Rectangle { // Optional background for the view
        anchors.fill: parent
        color: "#FFFFFF" // White background for content area
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20 // Add some padding around the content
        spacing: 15         // Space between elements

        Label {
            text: "Device Channels Control"
            font.pixelSize: 22
            font.bold: true
            color: "#333"
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 10
        }

        // Delegate to ChannelSwitchesView for displaying the switches
        ChannelSwitchesView {
            Layout.fillWidth: true
            Layout.fillHeight: true // Allow it to take available space
        }
    }
}