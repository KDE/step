enable_testing()

set(stepcore_tests_SRCS
    maintest.cc
    metaobject.cc
    worldcopy.cc
)

if(STEPCORE_WITH_QT)
    add_definitions(-DSTEPCORE_WITH_QT)
endif(STEPCORE_WITH_QT)

if(STEPCORE_WITH_GSL)
    add_definitions(-DSTEPCORE_WITH_GSL)
endif(STEPCORE_WITH_GSL)

set(QT_DONT_USE_QTGUI ON)
set(QT_USE_QTTEST ON)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
qt4_automoc(${stepcore_tests_SRCS})

add_executable(tests ${stepcore_tests_SRCS})
target_link_libraries(tests stepcore ${QT_LIBRARIES})

add_test(StepCore tests)

