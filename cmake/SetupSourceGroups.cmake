function(setup_source_groups curdir)
    file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${curdir} ${curdir}/*)
    foreach (child ${children})
        if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${curdir}/${child})
            setup_source_groups(${curdir}/${child})
        else ()
            string(REPLACE "/" "\\" group_name ${curdir})
            source_group(${group_name} FILES ${curdir}/${child})
        endif ()
    endforeach ()
endfunction()
