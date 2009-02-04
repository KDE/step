FILE(GLOB STEP_EXAMPLE_FILES "*.step")

if(STEP_BUILD_TRANSLATIONS)

    set(po_file ${CMAKE_SOURCE_DIR}/messages/kdeedu/step_example_files.po)
    STEP_PROCESS_XML_TRANSLATION(${CURRENT_LANG} ${po_file} "--tag=name;--tag=text;--unquote"
            ALL INSTALL_DESTINATION ${DATA_INSTALL_DIR}/step/examples
            ${STEP_EXAMPLE_FILES})

else(STEP_BUILD_TRANSLATIONS)

    install(FILES 
       ${STEP_EXAMPLE_FILES}
       DESTINATION  ${DATA_INSTALL_DIR}/step/examples)

endif(STEP_BUILD_TRANSLATIONS)

