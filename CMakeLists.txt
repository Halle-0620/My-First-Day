cmake_minimum_required(VERSION 3.21)

project(MyFirstDay VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets Multimedia)

qt_standard_project_setup()

qt_add_executable(MyFirstDay
    src/audio/audiomanager.cpp
    src/audio/audiomanager.h
    src/audio/audiotestdialog.cpp
    src/audio/audiotestdialog.h
    src/core/backgroundstyle.h
    src/core/interactiontypes.h
    src/main.cpp
    src/gamewindow.cpp
    src/gamewindow.h
    src/narrative/gamestate.cpp
    src/narrative/gamestate.h
    src/narrative/narrativeengine.cpp
    src/narrative/narrativeengine.h
    src/narrative/samplestory.cpp
    src/narrative/samplestory.h
    src/narrative/scene.cpp
    src/narrative/scene.h
    src/narrative/storyloader.cpp
    src/narrative/storyloader.h
    src/ui/backgroundwidget.cpp
    src/ui/backgroundwidget.h
    src/ui/dialoguepanel.cpp
    src/ui/dialoguepanel.h
    src/ui/floatingchoicelayer.cpp
    src/ui/floatingchoicelayer.h
    src/ui/interactiontypes.h
    src/ui/shadertoywidget.cpp
    src/ui/shadertoywidget.h
    src/core/shadereffect.h
)

target_link_libraries(MyFirstDay PRIVATE Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Multimedia)

if(WIN32)
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    set(QT_WINDEPLOYQT_EXECUTABLE "${QT_BIN_DIR}/windeployqt.exe")

    add_custom_command(TARGET MyFirstDay POST_BUILD
        COMMAND "${QT_WINDEPLOYQT_EXECUTABLE}"
            --no-compiler-runtime
            --no-translations
            "$<TARGET_FILE:MyFirstDay>"
    )
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/data")
configure_file("${CMAKE_SOURCE_DIR}/data/story_main.json" "${CMAKE_BINARY_DIR}/data/story_main.json" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/data/audio_manifest.json" "${CMAKE_BINARY_DIR}/data/audio_manifest.json" COPYONLY)
# configure_file runs only at CMake configure time. This stamp rule re-copies JSON when the
# sources change, even if `cmake --build` would otherwise do no C++ work (ninja: no work to do).
set(_DATA_JSON_STAMP "${CMAKE_BINARY_DIR}/data/.data_json_sync_stamp")
add_custom_command(
    OUTPUT "${_DATA_JSON_STAMP}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_SOURCE_DIR}/data/story_main.json"
        "${CMAKE_BINARY_DIR}/data/story_main.json"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_SOURCE_DIR}/data/audio_manifest.json"
        "${CMAKE_BINARY_DIR}/data/audio_manifest.json"
    COMMAND "${CMAKE_COMMAND}" -E touch "${_DATA_JSON_STAMP}"
    DEPENDS
        "${CMAKE_SOURCE_DIR}/data/story_main.json"
        "${CMAKE_SOURCE_DIR}/data/audio_manifest.json"
    COMMENT "Syncing data/*.json into build directory"
    VERBATIM
)
add_custom_target(data_json_sync DEPENDS "${_DATA_JSON_STAMP}")
add_dependencies(MyFirstDay data_json_sync)
file(COPY "${CMAKE_SOURCE_DIR}/assets/audio" DESTINATION "${CMAKE_BINARY_DIR}/assets")
file(COPY "${CMAKE_SOURCE_DIR}/assets/shaders" DESTINATION "${CMAKE_BINARY_DIR}/assets")
file(COPY "${CMAKE_SOURCE_DIR}/图" DESTINATION "${CMAKE_BINARY_DIR}")
