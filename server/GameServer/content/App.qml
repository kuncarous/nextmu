import QtQuick 6.2
import GameServer
import GameServerBackend

Window {
    width: 1024
    height: 768

    visible: true
    minimumHeight: 600
    minimumWidth: 400
    title: "NextMU GameServer"

    NRootContext {
        objectName: "rootCxt"
        id: rootCxt
        performanceStatistics: ""
        messages: []
    }

    MainScreen {
        id: gsMainScreen
        anchors.fill: parent
        performanceStatistics: rootCxt.performanceStatistics
        messages: rootCxt.messages
    }

}

