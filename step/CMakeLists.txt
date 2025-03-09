add_executable(step)

ecm_setup_version(${RELEASE_SERVICE_VERSION} VARIABLE_PREFIX STEP VERSION_HEADER step_version.h)

target_sources(step PRIVATE
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

    step.qrc
)

if(STEP_WITH_QALCULATE)
    target_compile_definitions(step PRIVATE
        -DSTEP_WITH_QALCULATE
        -DSTEP_WITH_UNITSCALC
    )
    target_compile_options(step PRIVATE ${QALCULATE_CFLAGS})
    target_link_libraries(step ${QALCULATE_LIBRARIES} ${CLN_LIBRARIES})
    # To compile with libcln
    string (REPLACE "-fno-exceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    add_definitions (-fexceptions)
endif(STEP_WITH_QALCULATE)

ki18n_wrap_ui(step
    create_gas_particles.ui
    create_softbody_items.ui
    configure_graph.ui
    configure_meter.ui
    configure_controller.ui
    configure_step_general.ui
)

kconfig_add_kcfg_files(step settings.kcfgc)

ecm_create_qm_loader(step step_qt)

target_include_directories(step PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(step
    stepcore
    KF6::Crash
    KF6::I18n
    KF6::IconThemes
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::TextWidgets
    KF6::Plotting
    KF6::NewStuffWidgets
    KF6::XmlGui
    Qt::Widgets
    Qt::OpenGLWidgets
)

install(TARGETS step ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES step.kcfg DESTINATION  ${KDE_INSTALL_KCFGDIR})
install(FILES step.knsrc DESTINATION  ${KDE_INSTALL_KNSRCDIR})
install( PROGRAMS org.kde.step.desktop DESTINATION ${KDE_INSTALL_APPDIR} )

add_subdirectory(icons)
add_subdirectory(data)
