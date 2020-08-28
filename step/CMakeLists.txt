ecm_setup_version(${RELEASE_SERVICE_VERSION} VARIABLE_PREFIX STEP VERSION_HEADER step_version.h)

set(step_SRCS
    arrow.cc
    clipboard.cc
    mainwindow.cc
    worldmodel.cc
    worldscene.cc
    worldgraphics.cc
    worldbrowser.cc
    propertiesbrowser.cc
    infobrowser.cc
    undobrowser.cc
    itempalette.cc
    worldfactory.cc
    stepgraphicsitem.cc
    particlegraphics.cc
    polygongraphics.cc
    gasgraphics.cc
    gascreationdialog.cc
    softbodygraphics.cc
    springgraphics.cc
    motorgraphics.cc
    jointgraphics.cc
    toolgraphics.cc
    simulationthread.cc
    unitscalc.cc
    latexformula.cc
    messageframe.cc
    main.cc
    choicesmodel.cc
)

set(step_UIS
    create_gas_particles.ui
    create_softbody_items.ui
    configure_graph.ui
    configure_meter.ui
    configure_controller.ui
    configure_step_general.ui)

set(step_KCFGS
    settings.kcfgc)

if(STEPCORE_WITH_QT)
    add_definitions(-DSTEPCORE_WITH_QT)
endif(STEPCORE_WITH_QT)

if(STEPCORE_WITH_GSL)
    add_definitions(-DSTEPCORE_WITH_GSL)
endif(STEPCORE_WITH_GSL)

if(STEP_WITH_QALCULATE)
    add_definitions(-DSTEP_WITH_QALCULATE)
    add_definitions(-DSTEP_WITH_UNITSCALC)
    add_definitions(${QALCULATE_CFLAGS})
    set(qalculate_LIBS ${QALCULATE_LIBRARIES} ${CLN_LIBRARIES})
    # To compile with libcln
    string (REPLACE "-fno-exceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    add_definitions (-fexceptions)
endif(STEP_WITH_QALCULATE)

ki18n_wrap_ui(step_SRCS ${step_UIS})
kconfig_add_kcfg_files(step_SRCS ${step_KCFGS})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

ecm_create_qm_loader(step_SRCS step_qt)

add_executable(step ${step_SRCS})

target_link_libraries(step
    stepcore
    KF5::Crash
    KF5::I18n
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::TextWidgets
    KF5::Plotting
    KF5::NewStuff
    Qt5::OpenGL
    Qt5::Widgets
    ${qalculate_LIBS})

install(TARGETS step ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES step.kcfg DESTINATION  ${KDE_INSTALL_KCFGDIR})
install(FILES stepui.rc DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/step)
install(FILES step.knsrc DESTINATION  ${KDE_INSTALL_KNSRCDIR})
install( PROGRAMS org.kde.step.desktop DESTINATION ${KDE_INSTALL_APPDIR} )

add_subdirectory(icons)
add_subdirectory(data)

