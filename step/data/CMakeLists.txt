find_package(Python3 COMPONENTS Interpreter)

set(EXTRACTXML_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/extractxml)

if (TARGET Python3::Interpreter)


    macro(STEP_PROCESS_XML_TRANSLATION _lang _po_file _extractxml_args)
    set(_xmlfiles_tr)
    set(_args ${ARGN})
    set(_addToAll)
    set(_installDest)

    list(GET _args 0 _tmp)
    if("${_tmp}" STREQUAL "ALL")
        set(_addToAll ALL)
        list(REMOVE_AT _args 0)
    endif()

    list(GET _args 0 _tmp)
    if("${_tmp}" STREQUAL "INSTALL_DESTINATION")
        list(GET _args 1 _installDest )
        list(REMOVE_AT _args 0 1)
    endif()

    if(EXISTS ${_po_file})
        get_filename_component(_po_file_we ${_po_file} NAME_WE)
        set(_mo_file ${CMAKE_CURRENT_BINARY_DIR}/${_lang}/${_po_file_we}.mo)
        add_custom_command(OUTPUT ${_mo_file}
        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_mo_file} ${_po_file}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS ${_po_file}
        )

        foreach(_xmlfile ${_args})
        get_filename_component(_xmlfile_basename ${_xmlfile} NAME)
        set(_xmlfile_tr ${CMAKE_CURRENT_BINARY_DIR}/${_lang}/${_xmlfile_basename})
        set(_extractxml_args_copy ${_extractxml_args})
        separate_arguments(_extractxml_args_copy)
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_lang})
        add_custom_command(OUTPUT ${_xmlfile_tr}
            COMMAND ${Python3_EXECUTABLE} ${EXTRACTXML_EXECUTABLE} --translate --mo-file=${_mo_file} --output-dir=${CMAKE_CURRENT_BINARY_DIR}/${_lang} ${_extractxml_args_copy} ${_xmlfile}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            DEPENDS ${_xmlfile} ${_mo_file}
        )
        if(_installDest)
            install(FILES ${_xmlfile_tr} DESTINATION ${_installDest}/${_lang})
        endif()
        list(APPEND _xmlfiles_tr ${_xmlfile_tr})
        endforeach()

        get_filename_component(DirId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
        string(REPLACE "@" "_" stripped_lang ${_lang})
        add_custom_target(${DirId}_${_po_file_we}_${stripped_lang}_xmlfiles_tr ${_addToAll} DEPENDS ${_xmlfiles_tr})
    endif()
    endmacro()

else()
    message("Python executable NOT found - Step data translations will not be build")
    macro(STEP_PROCESS_XML_TRANSLATION)
    endmacro()
endif()

add_subdirectory(objectinfo)
add_subdirectory(examples)
add_subdirectory(tutorials)

