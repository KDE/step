FILE(GLOB INFO_HTML_FILES "*.html")

if(STEP_BUILD_TRANSLATIONS)

    set(po_file ${CMAKE_SOURCE_DIR}/messages/kdeedu/step_objinfo_files.po)
    STEP_PROCESS_XML_TRANSLATION(${CURRENT_LANG} ${po_file} "--tag-regex=.;--recursive;--strip"
            ALL INSTALL_DESTINATION ${DATA_INSTALL_DIR}/step/objinfo
            ${INFO_HTML_FILES})

else(STEP_BUILD_TRANSLATIONS)

    install(FILES 
       ${INFO_HTML_FILES}
       DESTINATION  ${DATA_INSTALL_DIR}/step/objinfo)

endif(STEP_BUILD_TRANSLATIONS)
