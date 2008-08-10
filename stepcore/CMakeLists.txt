
set(stepcore_SRCS
    object.cc
    constants.cc
    world.cc
    solver.cc
    collisionsolver.cc
    constraintsolver.cc
    factory.cc
    xmlfile.cc
    particle.cc
    rigidbody.cc
    gas.cc
    softbody.cc
    gravitation.cc
    coulombforce.cc
    spring.cc
    motor.cc
    joint.cc
    eulersolver.cc
    tool.cc
    group.cc
    air.cc
)

if(STEPCORE_WITH_GSL)
    add_definitions(-DSTEPCORE_WITH_GSL)
    include_directories(${GSL_INCLUDE_DIR})
    set(stepcore_use_LIBS ${stepcore_use_LIBS} ${GSL_LIBRARIES})
    set(stepcore_SRCS ${stepcore_SRCS} gslsolver.cc)
endif(STEPCORE_WITH_GSL)

if(STEPCORE_WITH_QT)
    add_definitions(-DSTEPCORE_WITH_QT)

    set(QT_DONT_USE_QTGUI ON)
    set(QT_USE_QTXML ON)
    find_package(Qt4 REQUIRED)

    include(${QT_USE_FILE})
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    set(stepcore_use_LIBS ${stepcore_use_LIBS} ${QT_LIBRARIES})
endif(STEPCORE_WITH_QT)

add_definitions(-Wall) # XXX ?

kde4_add_library(stepcore STATIC ${stepcore_SRCS})
target_link_libraries(stepcore ${stepcore_use_LIBS})

