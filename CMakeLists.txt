cmake_minimum_required(VERSION 4.0.3...4.2.3)

# Set experimental flag to enable `import std` support from CMake.
# This must be enabled before C++ language support.
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  # This specific value changes as experimental support evolves. See
  # `Help/dev/experimental.rst` in the CMake source corresponding to
  # your CMake build for the exact value to use.
  "d0edc3af-4c50-42ea-a356-e2862fe7a444")

set(CMAKE_CXX_EXTENSIONS OFF)

project(candela-3)

add_custom_target(generate_version
    COMMAND ${CMAKE_SOURCE_DIR}/version/generate_auto_version.$<IF:$<PLATFORM_ID:Windows>,bat,sh>
    BYPRODUCTS ${CMAKE_SOURCE_DIR}/version/auto_version.h
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Generating version file"
)

add_executable(candela-3 main.cpp version/version.cpp version/auto_version.h)
add_dependencies(candela-3 generate_version)

target_sources(candela-3 PRIVATE FILE_SET cxx_modules TYPE CXX_MODULES FILES 
    version/version.cppm
)
set_target_properties(candela-3 PROPERTIES 
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_MODULE_STD ON
    # CXX_SCAN_FOR_MODULES ON
    COMPILE_WARNING_AS_ERROR ON
    DEBUG_POSTFIX "_d"
)

target_compile_options(candela-3 PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4> # Should add /WX too
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

# Install Candela-3
install(TARGETS candela-3 
    # EXPORT candela-3-export
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install any pdb files (Windows)
# if(MSVC)
# 	install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL)
# endif()

# # Install the exports in the appropriate cmake folder
# install(EXPORT candela-3-export
# 	FILE Candela3Targets.cmake
# 	NAMESPACE Feanor::
# 	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Candela3
# )

# # Install the supplied cmake.in file (this should replace
# # any placeholders in it)
# include(CMakePackageConfigHelpers)
# configure_package_config_file(
# 	${CMAKE_CURRENT_LIST_DIR}/cmake/Candela3Config.cmake.in
# 	${CMAKE_CURRENT_BINARY_DIR}/Candela3Config.cmake
# 	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Candela3
# )

# # Install the generated config file (not sure if this is needed)
# install(FILES
# 	${CMAKE_CURRENT_BINARY_DIR}/Candela3Config.cmake
# 	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Candela3
# )
