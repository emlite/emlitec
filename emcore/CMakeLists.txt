cmake_minimum_required(VERSION 3.25)
project(emcore VERSION 0.1.0 LANGUAGES C)

option(EMLITE_USE_DLMALLOC "Use dlmalloc for freestanding environments" OFF)
option(EMLITE_EMSCRIPTEN_STANDALONE_WASM "Use emscripten's default js output" OFF)

add_library(emcore)
set(EMCORE_HEADERS include/emcore/emcore.h)
target_sources(emcore 
    PRIVATE 
        src/lib.c
    PUBLIC 
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include
        FILES ${EMCORE_HEADERS}
)
set_target_properties(emcore PROPERTIES LINKER_LANGUAGE C)

set(EMLITE_WITH_EMSENV OFF)
if(EMSCRIPTEN AND NOT EMLITE_EMSCRIPTEN_STANDALONE_WASM)
    set(EMLITE_WITH_EMSENV ON)
    find_package(emsenv CONFIG QUIET)
    if (NOT emsenv_FOUND)
        add_subdirectory(emsenv)
    endif()
    target_link_libraries(emcore PRIVATE emsenv::emsenv)
endif()

set(EMLITE_WITH_DLMALLOC OFF)
if (EMLITE_USE_DLMALLOC)
    set(EMLITE_WITH_DLMALLOC ON)
    find_package(dlmalloc CONFIG QUIET)
    if(NOT dlmalloc_FOUND)
        add_subdirectory(dlmalloc)
    endif()
    target_compile_definitions(emcore PUBLIC HAVE_DLMALLOC)
    target_link_libraries(emcore PRIVATE dlmalloc)
endif()

add_library(emcore::emcore ALIAS emcore)

include(GNUInstallDirs)

set(emcore_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/emcore")

install(
  TARGETS emcore
  EXPORT  emcoreTargets
  FILE_SET  HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  EXPORT      emcoreTargets
  FILE        emcoreTargets.cmake
  NAMESPACE   emcore::
  DESTINATION "${emcore_INSTALL_CMAKEDIR}"
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/emcoreConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/emcoreConfig.cmake"
  INSTALL_DESTINATION "${emcore_INSTALL_CMAKEDIR}"
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/emcoreConfigVersion.cmake"
  VERSION       ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/emcoreConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/emcoreConfigVersion.cmake"
  DESTINATION "${emcore_INSTALL_CMAKEDIR}"
)
