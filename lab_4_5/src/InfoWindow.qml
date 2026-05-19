import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

Rectangle {
    id: root

    width: compactMode ? 420 : 540
    height: compactMode ? 260 : 360
    color: "#f8fafc"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 22
        spacing: 14

        Rectangle {
            Layout.fillWidth: true
            height: 4
            radius: 2
            color: accentColor
        }

        Label {
            text: windowTitle
            font.pixelSize: 22
            font.bold: true
            color: "#0f172a"
            Layout.fillWidth: true
        }

        Label {
            text: message
            color: "#0f172a"
            font.pixelSize: 15
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            color: "#ffffff"
            border.color: "#e2e8f0"

            Label {
                anchors.fill: parent
                anchors.margins: 14
                text: details
                color: "#334155"
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
            }
        }

        Button {
            Layout.alignment: Qt.AlignRight
            text: "Close"
            onClicked: root.Window.window.close()
        }
    }
}
