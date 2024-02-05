

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

Rectangle {
    id: gsBackgroundRect
    width: Constants.width
    height: Constants.height
    color: Constants.backgroundColor

    ColumnLayout {
        id: column
        anchors.fill: parent

        MainBar {
            Layout.fillWidth: true
        }

        ListView {
            id: gsConsole
            clip: true
            Layout.fillHeight: true
            Layout.fillWidth: true
            ScrollBar.vertical: ScrollBar {}

            model: rootCxt.messages
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
                    color: backgroundColor.length > 0 ? backgroundColor : Constants.backgroundColor

                    TextEdit {
                        width: gsLayout.width

                        text: message
                        color: fontColor.length > 0 ? fontColor : {}
                        selectedTextColor: selectedColor.length > 0 ? selectedColor : {}
                        selectionColor: highlightColor.length > 0 ? highlightColor : {}
                        readOnly: true
                        selectByMouse: true
                        font.bold: true
                        wrapMode: Text.Wrap
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
                text: rootCxt.performanceStatistics
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
