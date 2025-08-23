set(DEFAULT_SUFFIX .wasm)
set(DEFAULT_LINK_FLAGS "-Wl,--no-entry,--allow-undefined,--export-dynamic,--export-if-defined=main,--export-table,--import-memory,--export-memory,--strip-all")

if (EMSCRIPTEN)
    if (EMSCRIPTEN_STANDALONE_WASM)
        set(DEFAULT_LINK_FLAGS "${DEFAULT_LINK_FLAGS},--export-if-defined=_start -sERROR_ON_UNDEFINED_SYMBOLS=0 -sALLOW_MEMORY_GROWTH=1")
    else()
        set(DEFAULT_LINK_FLAGS "-sERROR_ON_UNDEFINED_SYMBOLS=0 -sALLOW_MEMORY_GROWTH=1 -sEXPORTED_FUNCTIONS=_main -Wl,--strip-all,--export-dynamic")
        set(DEFAULT_SUFFIX .js)
    endif()
endif()

add_executable(console console.c)
target_link_libraries(console PRIVATE emlitec::emlitec)
set_target_properties(console PROPERTIES LINKER_LANGUAGE C SUFFIX ${DEFAULT_SUFFIX} LINK_FLAGS ${DEFAULT_LINK_FLAGS})

add_executable(cbind cbind.c)
target_link_libraries(cbind PRIVATE emlitec::emlitec)
set_target_properties(cbind PROPERTIES LINKER_LANGUAGE C SUFFIX ${DEFAULT_SUFFIX} LINK_FLAGS ${DEFAULT_LINK_FLAGS})

if (NOT USING_FREESTANDING)
    
endif()


if (NOT USING_FREESTANDING)
    if (NOT USING_WASI_LIBC)
        
    endif()
    add_executable(dom_test2 dom_test2.c)
    target_link_libraries(dom_test2 PRIVATE emlitec::emlitec)
    set_target_properties(dom_test2 PROPERTIES LINKER_LANGUAGE C SUFFIX ${DEFAULT_SUFFIX} LINK_FLAGS ${DEFAULT_LINK_FLAGS})
endif()
add_executable(dom_test2_nostdlib dom_test2_nostdlib.c)
target_link_libraries(dom_test2_nostdlib PRIVATE emlitec::emlitec)
set_target_properties(dom_test2_nostdlib PROPERTIES LINKER_LANGUAGE C SUFFIX ${DEFAULT_SUFFIX} LINK_FLAGS ${DEFAULT_LINK_FLAGS})