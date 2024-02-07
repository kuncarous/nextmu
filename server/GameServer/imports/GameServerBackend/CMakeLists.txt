# Root Context Group
set(
    N_ROOT_CONTEXT_INCLUDE
	n_root_context.qml.h
)

set(
    N_ROOT_CONTEXT_SOURCE
	n_root_context.qml.cpp
)
 
source_group("Root\\Context" FILES ${N_ROOT_CONTEXT_SOURCE} ${N_ROOT_CONTEXT_INCLUDE})
list(APPEND N_QML_SOURCE ${N_ROOT_CONTEXT_INCLUDE})
list(APPEND N_QML_INCLUDE ${N_ROOT_CONTEXT_SOURCE})

qt_add_library(GameServerBackend STATIC)
qt6_add_qml_module(GameServerBackend
    URI "GameServerBackend"
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    SOURCES
        ${N_QML_SOURCE}
        ${N_QML_INCLUDE}
)