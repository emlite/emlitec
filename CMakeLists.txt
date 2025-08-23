cmake_minimum_required(VERSION 3.28)
project(emlitec VERSION 0.1.2 LANGUAGES C)

option(EMLITEC_BUILD_EXAMPLES "Build examples" OFF)

add_library(emlitec)

set(EMLITEC_HEADERS)
set(EMLITEC_SOURCES)

set(EMLITEC_HEADERS
    include/emlite/emlite.h
)
set(EMLITEC_SOURCES
    src/emlite.c
)
target_compile_features(emlitec PUBLIC c_std_11)
set_target_properties(emlitec PROPERTIES LINKER_LANGUAGE C)

target_sources(emlitec 
    PRIVATE 
        ${EMLITEC_SOURCES} 
    PUBLIC 
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include
        FILES ${EMLITEC_HEADERS}
)

find_package(emcore CONFIG QUIET)
if(NOT emcore_FOUND)
    add_subdirectory(emcore)
endif()
target_link_libraries(emlitec PUBLIC emcore::emcore)

add_library(emlitec::emlitec ALIAS emlitec)

if (EMLITEC_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

include(GNUInstallDirs)

set(emlitec_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/emlitec")

install(TARGETS emlitec EXPORT emlitecTargets FILE_SET  HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT emlitecTargets
    FILE emlitecTargets.cmake
    NAMESPACE emlitec::
    DESTINATION ${emlitec_INSTALL_CMAKEDIR}
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/emlitecConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/emlitecConfig.cmake"
  INSTALL_DESTINATION "${emlitec_INSTALL_CMAKEDIR}"
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/emlitecConfigVersion.cmake"
  VERSION       ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/emlitecConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/emlitecConfigVersion.cmake"
  DESTINATION "${emlitec_INSTALL_CMAKEDIR}"
)


