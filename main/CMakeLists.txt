set(SOURCES
        Main.cpp
        Scales/ScalesEventLoop.cpp
        Scales/HX711/HX711.cpp
        Lib/Timer.cpp
        Lib/EventLoop.cpp
        Lib/Wifi.cpp
        RESTServer/Server.cpp
        UI/Ui.cpp
        UI/LoadingScreen.cpp
        UI/ScalesScreen.cpp
        Updater/Updater.cpp
        Updater/UpdaterEventLoop.cpp
)

set(INCLUDES
        .
        ../Secrets
        Scales
        Scales/HX711
        Lib
        UI
        Updater
)

idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS ${INCLUDES})

target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLV_LVGL_H_INCLUDE_SIMPLE")
