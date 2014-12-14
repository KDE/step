include(ECMInstallIcons)

add_subdirectory(actions)

ecm_install_icons(
    ICONS
        hi128-apps-step.png
        hi16-apps-step.png
        hi22-apps-step.png
        hi32-apps-step.png
        hi48-apps-step.png
        hi64-apps-step.png

    DESTINATION ${ICON_INSTALL_DIR}
    THEME hicolor
)
