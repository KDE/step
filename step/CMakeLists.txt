set(step_SRCS
    mainwindow.cc
    worldmodel.cc
    worldscene.cc
    worldgraphics.cc
    worldbrowser.cc
    propertiesbrowser.cc
    infobrowser.cc
    itempalette.cc
    worldfactory.cc
    particlegraphics.cc
    polygongraphics.cc
    springgraphics.cc
    toolgraphics.cc
    simulationthread.cc
    main.cc
)

set(step_UIS
    configure_graph.ui
    configure_controller.ui)

if(STEPCORE_WITH_QT)
    add_definitions(-DSTEPCORE_WITH_QT)
endif(STEPCORE_WITH_QT)

if(STEPCORE_WITH_GSL)
    add_definitions(-DSTEPCORE_WITH_GSL)
endif(STEPCORE_WITH_GSL)

find_package(KDE4 REQUIRED)
include_directories( ${KDE4_INCLUDES} )
add_definitions( ${KDE4_DEFINITIONS} )

kde4_add_ui_files( step_SRCS ${step_UIS})
kde4_automoc(${step_SRCS})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

kde4_add_executable(step ${step_SRCS})
target_link_libraries(step stepcore ${KDE4_KIO_LIBS} ${KDE4_KHTML_LIBS})

install(TARGETS step DESTINATION ${BIN_INSTALL_DIR})
install(FILES stepui.rc DESTINATION  ${DATA_INSTALL_DIR}/step)

add_subdirectory(icons)
add_subdirectory(objinfo)


