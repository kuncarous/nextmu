import QtQuick 6.2
import GameServer

Window {
    width: 1024
    height: 768

    visible: true
    minimumHeight: 600
    minimumWidth: 400
    title: "NextMU GameServer"

    MainScreen {
        id: gsMainScreen
        anchors.fill: parent
    }

}

