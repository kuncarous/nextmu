

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts
import GameServer
import GameServerBackend

Rectangle {
    property string performanceStatistics: "Test Label"
    property list<NConsoleMessage> messages: [
        NConsoleMessage {
            message: "testing"
            backgroundColor: "red"
        }
    ]

    id: gsBackgroundRect
    width: Constants.width
    height: Constants.height
    color: Constants.backgroundColor

    ColumnLayout {
        id: column
        anchors.fill: parent
        spacing: 0

        MainBar {
            Layout.fillWidth: true
        }

        Rectangle {
            color: "#ffffff"
            Layout.fillHeight: true
            Layout.fillWidth: true

            ListView {
                anchors.fill: parent
                id: gsConsole
                clip: true
                interactive: false
                verticalLayoutDirection: ListView.BottomToTop

                model: messages
                delegate: RowLayout {
                    id: gsLayout
                    clip: true
                    width: gsConsole.width
                    Layout.leftMargin: 5
                    Layout.minimumHeight: 40

                    Rectangle {
                        clip: true
                        Layout.fillWidth: true
                        Layout.minimumHeight: childrenRect.height
                        color: backgroundColor.length > 0 ? backgroundColor : "transparent"

                        TextEdit {
                            width: gsLayout.width

                            text: message
                            color: fontColor.length > 0 ? fontColor : {}
                            selectedTextColor: selectedColor.length > 0 ? selectedColor : {}
                            selectionColor: highlightColor.length > 0 ? highlightColor : {}
                            readOnly: true
                            selectByMouse: false

                            font.pixelSize: 18
                            font.bold: true
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }
        }

        RowLayout {
            id: rowLayout
            width: 100
            height: 20
            Layout.rightMargin: 8
            Layout.leftMargin: 8
            Layout.maximumHeight: 20
            Layout.fillWidth: true

            Label {
                id: fpsLabel
                text: performanceStatistics
                Layout.fillHeight: true
            }
        }
    }
    states: [
        State {
            name: "clicked"
        }
    ]
}
