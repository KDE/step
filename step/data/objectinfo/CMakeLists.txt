FILE(GLOB INFO_HTML_FILES "*.html")

install(FILES
       ${INFO_HTML_FILES}
       DESTINATION  ${KDE_INSTALL_DATADIR}/step/objinfo)

FILE(GLOB po_files "${CMAKE_SOURCE_DIR}/po/*/step_objinfo_files.po")
foreach(po_file ${po_files})
    get_filename_component(po_dir ${po_file} DIRECTORY)
    get_filename_component(CURRENT_LANG ${po_dir} NAME)
    STEP_PROCESS_XML_TRANSLATION(${CURRENT_LANG} ${po_file}
        "--context='%(filename)s:%(tag)s' --tag-regex='^(?:title|body|p|h[1-6])$$' --recursive --strip"
        ALL INSTALL_DESTINATION ${KDE_INSTALL_DATADIR}/step/objinfo/l10n
        ${INFO_HTML_FILES})
endforeach()
