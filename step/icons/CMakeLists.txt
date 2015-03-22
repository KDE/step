include(ECMInstallIcons)

add_subdirectory(actions)

ecm_install_icons(
    ICONS
        128-apps-step.png
        16-apps-step.png
        22-apps-step.png
        32-apps-step.png
        48-apps-step.png
        64-apps-step.png

    DESTINATION ${KDE_INSTALL_ICONDIR} THEME hicolor
)
