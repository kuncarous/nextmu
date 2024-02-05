import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts

MenuBar {
    id: menuBar

    Menu {
        title: qsTr("&Server")
        Action { text: qsTr("&5 Minutes Close") }
        Action { text: qsTr("&3 Minutes Close") }
        Action { text: qsTr("&1 Minutes Close") }
        Action { text: qsTr("&Force Close") }
    }
    Menu {
        title: qsTr("&Load")
        Action { text: qsTr("&Monsters Reload") }
        Action { text: qsTr("&Shops Reload") }
    }
    Menu {
        title: qsTr("&Help")
        Action { text: qsTr("&About") }
    }
}
