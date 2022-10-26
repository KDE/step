add_library(stepcore STATIC)

target_sources(stepcore PRIVATE
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

target_link_libraries(stepcore
    Eigen3::Eigen
)

if(STEPCORE_WITH_GSL)
    target_compile_definitions(stepcore PUBLIC -DSTEPCORE_WITH_GSL)
    target_include_directories(stepcore PUBLIC ${GSL_INCLUDE_DIR})
    target_link_libraries(stepcore ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
    target_sources(stepcore PRIVATE gslsolver.cc)
endif(STEPCORE_WITH_GSL)

if(STEPCORE_WITH_QT)
    target_compile_definitions(stepcore PUBLIC -DSTEPCORE_WITH_QT)

    find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Xml)

    target_link_libraries(stepcore Qt::Core Qt::Xml)
endif(STEPCORE_WITH_QT)
