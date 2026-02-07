cmake_minimum_required(VERSION 4.2.3)

# Set experimental flag to enable `import std` support from CMake.
# This must be enabled before C++ language support.
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  # This specific value changes as experimental support evolves. See
  # `Help/dev/experimental.rst` in the CMake source corresponding to
  # your CMake build for the exact value to use.
  "d0edc3af-4c50-42ea-a356-e2862fe7a444")

set(CMAKE_CXX_EXTENSIONS OFF)

project(candela-3)

add_executable(candela-3 main.cpp)
# target_sources(candela-3 PRIVATE FILE_SET CXX_MODULES FILES test_module.cppm another_module.cppm shape_module.cppm separated_module.cppm split_main_module.cppm)
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
