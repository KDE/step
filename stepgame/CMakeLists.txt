set(stepgame_SRCS
    mainwindow.cc
    main.cc
)

set(stepgame_KCFGS
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
    add_definitions(${KDE4_ENABLE_EXCEPTIONS})
    set(qalculate_LIBS ${QALCULATE_LIBRARIES})
endif(STEP_WITH_QALCULATE)

find_package(KDE4 REQUIRED)
include_directories( ${KDE4_INCLUDES} )
add_definitions( ${KDE4_DEFINITIONS} )

kde4_add_ui_files( stepgame_SRCS ${stepgame_UIS} )
kde4_add_kcfg_files( stepgame_SRCS ${stepgame_KCFGS} )

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../step)

add_subdirectory(levels)

kde4_add_executable(stepgame ${stepgame_SRCS})
target_link_libraries(stepgame
    libstepgui
    stepcore
    ${KDE4_KHTML_LIBS}
    ${KDE4_KNEWSTUFF2_LIBS}
    ${QT_QTOPENGL_LIBRARY}
    ${qalculate_LIBS})

install(TARGETS stepgame ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES stepgame.kcfg DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES stepgameui.rc DESTINATION  ${DATA_INSTALL_DIR}/stepgame)
install( FILES stepgame.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


