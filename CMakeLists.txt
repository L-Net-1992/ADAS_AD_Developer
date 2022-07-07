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
#find_package(ADAS_AD_Backend REQUIRED CONFIG)

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

    ADAS_AD_Backend/inspector.hpp
    ADAS_AD_Backend/inspector.cpp
    ADAS_AD_Backend/inspector_thread.cpp
    ADAS_AD_Backend/inspector_thread.hpp
    ADAS_AD_Backend/invocable.hpp
    ADAS_AD_Backend/invocable_parser.hpp
    ADAS_AD_Backend/models.hpp
    ADAS_AD_Backend/models.cpp
    ADAS_AD_Backend/module_library.hpp
    ADAS_AD_Backend/module_library.cpp
    ADAS_AD_Backend/source_generator.hpp
    ADAS_AD_Backend/package_library.h ADAS_AD_Backend/package_library.cpp
    ADAS_AD_Backend/subsystem_library.h ADAS_AD_Backend/subsystem_library.cpp
    ADAS_AD_Backend/subsystem_window.ui ADAS_AD_Backend/subsystem_window.h ADAS_AD_Backend/subsystem_window.cpp
    ADAS_AD_Backend/new_subsystem_dialog.ui ADAS_AD_Backend/new_subsystem_dialog.h ADAS_AD_Backend/new_subsystem_dialog.cpp
    ADAS_AD_Backend/var_name_dialog.ui ADAS_AD_Backend/var_name_dialog.h ADAS_AD_Backend/var_name_dialog.cpp


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
#find_library(ADAS_AD_Backend libADAS_AD_Backend.so REQUIRED)

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
#    ${ADAS_AD_Backend}
    )


add_subdirectory(nodeeditor)



































