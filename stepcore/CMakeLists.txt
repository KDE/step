
set(stepcore_SRCS
    constants.cc

    material.cc

    # Base objects
    object.cc
    item.cc
    body.cc
    objecterrors.cc
    force.cc
    joint.cc
    itemgroup.cc

    # Storage objects
    world.cc
    factory.cc
    particle.cc
    rigidbody.cc
    gas.cc
    softbody.cc
    gravitation.cc
    coulombforce.cc
    spring.cc
    motor.cc
    tool.cc
    joints.cc
    solver.cc
    eulersolver.cc
    collisionsolver.cc
    constraintsolver.cc
    xmlfile.cc
)

if(STEPCORE_WITH_GSL)
    add_definitions(-DSTEPCORE_WITH_GSL)
    include_directories(${GSL_INCLUDE_DIR})
    set(stepcore_use_LIBS ${stepcore_use_LIBS} ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
    set(stepcore_SRCS ${stepcore_SRCS} gslsolver.cc)
endif(STEPCORE_WITH_GSL)

if(STEPCORE_WITH_QT)
    add_definitions(-DSTEPCORE_WITH_QT)

    set(QT_DONT_USE_QTGUI ON)
    set(QT_USE_QTXML ON)
    find_package(Qt5 5.2 REQUIRED NO_MODULE COMPONENTS Xml)

    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    set(stepcore_use_LIBS ${stepcore_use_LIBS} Qt5::Core Qt5::Xml)
endif(STEPCORE_WITH_QT)

add_definitions(-Wall) # XXX ?

add_library(stepcore STATIC ${stepcore_SRCS})

target_link_libraries(stepcore ${stepcore_use_LIBS})

