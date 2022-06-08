cmake_minimum_required(VERSION 3.12)

project(ADAS_AD_Developer VERSION 0.1 LANGUAGES CXX)

if(UNIX)
    set(CMAKE_BUILD_RPATH "$ORIGIN")
endif(UNIX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/node.db3 DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/sqlite)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/conf/config.json DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/conf)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/conf/model_menu.json DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/conf)
file(COPY ${allCopyFiles} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/nodeconfig)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generate)

find_package(QT NAMES QT6 Qt5 COMPONENTS Widgets Xml Sql Charts Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Xml Sql Charts Concurrent REQUIRED)
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
find_package(Boost 1.76.0 REQUIRED COMPONENTS system graph json)
find_package(HDF5 REQUIRED )

set ( PROJECT_SOURCES
    main.cpp
    dialogs/mainwindow.h dialogs/mainwindow.cpp dialogs/mainwindow.ui
    dialogs/projectdialog.h dialogs/projectdialog.cpp dialogs/projectdialog.ui
#    dialogs/editordialog.h dialogs/editordialog.cpp dialogs/editordialog.ui
    dialogs/editorwindow.h dialogs/editorwindow.cpp dialogs/editorwindow.ui
    dialogs/nodeparametersdialog.h dialogs/nodeparametersdialog.cpp dialogs/nodeparametersdialog.ui
    dialogs/nodetreedialog.h dialogs/nodetreedialog.cpp dialogs/nodetreedialog.ui
    dialogs/importscriptdialog.h dialogs/importscriptdialog.cpp dialogs/importscriptdialog.ui
    dialogs/testdialog.h dialogs/testdialog.cpp dialogs/testdialog.ui
    dialogs/datainspectordialog.h dialogs/datainspectordialog.cpp dialogs/datainspectordialog.ui
    dialogs/calibrationdialog.h dialogs/calibrationdialog.cpp dialogs/calibrationdialog.ui
    dialogs/monitordialog.h dialogs/monitordialog.cpp dialogs/monitordialog.ui
    dialogs/nodeparametersmildialog.h dialogs/nodeparametersmildialog.cpp dialogs/nodeparametersmildialog.ui

    dialogs/recentprojectdialog.h dialogs/recentprojectdialog.cpp dialogs/recentprojectdialog.ui
    dialogs/exportmoduledialog.h dialogs/exportmoduledialog.cpp dialogs/exportmoduledialog.ui
    hdf5/hdf5files_handle.h hdf5/hdf5files_handle.cpp

    monitor_files/monitor_chartview.h monitor_files/monitor_chartview.cpp
    monitor_files/monitor_signal.h  monitor_files/monitor_signal.cpp

    splashscreen/splashscreen.h splashscreen/splashscreen.cpp

    image.qrc
    itemreadonlydelegate.h
    utils.h utils.cpp

    model/modelsproject.hpp
    model/modelsrecentproject.hpp
    model/modelcategory.hpp

    controllers/aicctoolbutton.hpp
    controllers/aiccbreadcrumbnavigation.hpp
    controllers/aiccflowview.hpp
    controllers/aiccflowscene.hpp
    controllers/aiccstackedwidget.hpp
    controllers/aicctreewidget.hpp
    controllers/aiccstructtreewidget.hpp
    controllers/aicctitlebar.hpp

    nodeparser/inspector.hpp
    nodeparser/inspector.cpp
    nodeparser/inspector_thread.cpp
    nodeparser/inspector_thread.hpp
    nodeparser/invocable.hpp
    nodeparser/invocable_parser.hpp
    nodeparser/models.hpp
    nodeparser/models.cpp
    nodeparser/module_library.hpp
    nodeparser/module_library.cpp
    nodeparser/source_generator.hpp
    nodeparser/package_library.h nodeparser/package_library.cpp
    nodeparser/subsystem_library.h nodeparser/subsystem_library.cpp
    nodeparser/subsystem_window.ui nodeparser/subsystem_window.h nodeparser/subsystem_window.cpp
    nodeparser/new_subsystem_dialog.ui nodeparser/new_subsystem_dialog.h nodeparser/new_subsystem_dialog.cpp
    nodeparser/var_name_dialog.ui nodeparser/var_name_dialog.h nodeparser/var_name_dialog.cpp
    nodeparser/inspector.hpp nodeparser/inspector.cpp nodeparser/inspector_thread.hpp nodeparser/inspector_thread.cpp


    sqlite/aiccsqlite.hpp

    process/aiccprocess.hpp
#    nlohmann/json.hpp
    conf/config.json
    conf/model_menu.json
    conf/Config.hpp
    )

add_executable(ADAS_AD_Developer
    ${PROJECT_SOURCES}
    )

target_compile_options(ADAS_AD_Developer PRIVATE -Wall -fvisibility=hidden -fno-rtti )
target_include_directories(ADAS_AD_Developer
    PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers
    nodes
    )
find_library(QSCINTILLA libqscintilla2_qt5.so.15 REQUIRED)

target_link_options(ADAS_AD_Developer PRIVATE -no-pie)
target_link_libraries(ADAS_AD_Developer
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::Concurrent
    nodes
    ${LLVM_AVAILABLE_LIBS}
    clangAST
    clangBasic
    clangFrontend
    clangSerialization
    clangTooling
    Boost::json Boost::headers Boost::system Boost::graph
    ${HDF5_LIBRARIES}
    ${QSCINTILLA}
    )


add_subdirectory(nodeeditor)



































