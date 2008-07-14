set(stepgui_SRCS
    svgtheme.cpp
    svgthememanager.cpp
)

find_package(KDE4 REQUIRED)

include_directories( ${KDE4_INCLUDES} )
add_definitions( ${KDE4_DEFINITIONS} )

SET( QT_USE_QTXML TRUE )
SET( QT_USE_QTSVG TRUE )

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

kde4_add_library(stepgui STATIC ${stepgui_SRCS} )
target_link_libraries( stepgui ${QT_LIBRARIES} )


