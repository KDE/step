FILE(GLOB STEP_EXAMPLE_FILES "*.step")

if(STEP_BUILD_TRANSLATIONS)

    set(po_file ${CMAKE_SOURCE_DIR}/messages/kdeedu/step_example_files.po)
    STEP_PROCESS_XML_TRANSLATION(${CURRENT_LANG} ${po_file}
            "--context='%(tag)s' --tag=name --tag=text --unquote --parse-unquoted='--context=HTML:%(tag)s --tag-regex=^(?:title|body|p|h[1-6])$$ --recursive --strip'"
            ALL INSTALL_DESTINATION ${KDE_INSTALL_DATADIR}/step/examples
            ${STEP_EXAMPLE_FILES})

else(STEP_BUILD_TRANSLATIONS)

    install(FILES 
       ${STEP_EXAMPLE_FILES}
       DESTINATION  ${KDE_INSTALL_DATADIR}/step/examples)

endif(STEP_BUILD_TRANSLATIONS)

