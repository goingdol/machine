
import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts 6.2
// Enums are not directly used in this file, but CallLineDisplay might use them.

ScrollView {
    id: callViewRoot
    anchors.fill: parent
    clip: true
    ScrollBar.vertical.policy: ScrollBar.AsNeeded

    GridLayout {
        id: callLinesLayout
        columns: Screen.width > 700 ? 2 : 1 // 2 columns on wider screens, 1 on narrower
        width: parent.width
        // padding: 15
        columnSpacing: 15
        rowSpacing: 15

        // Repeater to create a display for each of the 4 call lines
        Repeater {
            id: callLinesRepeater
            model: 4 // Number of call lines

            // Delegate for each call line
            delegate: CallLineDisplay {
                lineNumber: index // Pass the line number to the delegate
                Layout.fillWidth: true // Ensure delegate takes available column width
                // Implicit height will be determined by CallLineDisplay's content
            }
        }
    }
}