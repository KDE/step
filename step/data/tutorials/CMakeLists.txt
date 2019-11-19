FILE(GLOB STEP_TUTORIALS_FILES "*.step")

install(FILES
       ${STEP_TUTORIALS_FILES}
       DESTINATION  ${KDE_INSTALL_DATADIR}/step/tutorials)

FILE(GLOB po_files "${CMAKE_SOURCE_DIR}/po/*/step_example_files.po")
foreach(po_file ${po_files})
    get_filename_component(po_dir ${po_file} DIRECTORY)
    get_filename_component(CURRENT_LANG ${po_dir} NAME)
    STEP_PROCESS_XML_TRANSLATION(${CURRENT_LANG} ${po_file}
        "--context='%(tag)s' --tag=name --tag=text --unquote --parse-unquoted='--context=HTML:%(tag)s --tag-regex=^(?:title|body|p|h[1-6])$$ --recursive --strip'"
        ALL INSTALL_DESTINATION ${KDE_INSTALL_DATADIR}/step/tutorials
        ${STEP_TUTORIALS_FILES})
endforeach()
