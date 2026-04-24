cmake_minimum_required(VERSION 3.21)

project(MyFirstDay VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(MyFirstDay
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
    src/ui/interactiontypes.h
)

target_link_libraries(MyFirstDay PRIVATE Qt6::Widgets)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/data")
configure_file("${CMAKE_SOURCE_DIR}/data/story_main.json" "${CMAKE_BINARY_DIR}/data/story_main.json" COPYONLY)
