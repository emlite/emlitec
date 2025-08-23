cmake_minimum_required(VERSION 3.20)
project(dlmalloc VERSION 0.1.0 LANGUAGES C)

add_library(dlmalloc src/dlmalloc.c)
set_target_properties(dlmalloc PROPERTIES LINKER_LANGUAGE C)
target_compile_definitions(dlmalloc PUBLIC 
        LACKS_UNISTD_H
        LACKS_FCNTL_H
        LACKS_SYS_PARAM_H
        LACKS_SYS_MMAN_H
        LACKS_STRINGS_H
        LACKS_STRING_H
        LACKS_SYS_TYPES_H
        LACKS_ERRNO_H
        LACKS_STDLIB_H
        LACKS_SCHED_H
        LACKS_TIME_H
        NO_MALLOC_STATS=1
        HAVE_MMAP=0
        HAVE_MORECORE=1
        MORECORE=sbrk
        MORECORE_CANNOT_TRIM=1
        MORECORE_CONTIGUOUS=1
        NO_MALLINFO=1
    )
add_library(dlmalloc::dlmalloc ALIAS dlmalloc)

include(GNUInstallDirs)

set(dlmalloc_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/dlmalloc")

install(
  TARGETS dlmalloc
  EXPORT  dlmallocTargets
)

install(
  EXPORT      dlmallocTargets
  FILE        dlmallocTargets.cmake
  NAMESPACE   dlmalloc::
  DESTINATION "${dlmalloc_INSTALL_CMAKEDIR}"
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/dlmallocConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/dlmallocConfig.cmake"
  INSTALL_DESTINATION "${dlmalloc_INSTALL_CMAKEDIR}"
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/dlmallocConfigVersion.cmake"
  VERSION       ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/dlmallocConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/dlmallocConfigVersion.cmake"
  DESTINATION "${dlmalloc_INSTALL_CMAKEDIR}"
)
