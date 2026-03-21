project(candela-3)

find_package(lyra CONFIG REQUIRED)

add_custom_target(generate_version
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/version/generate_auto_version.$<IF:$<PLATFORM_ID:Windows>,bat,sh>
    BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/version/auto_version.h
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Generating version file"
)

add_executable(candela-3)
add_dependencies(candela-3 generate_version)
target_link_libraries(candela-3 PRIVATE external core bfg::lyra VulkanCppModule)

target_sources(candela-3 PRIVATE 
    main.cpp 
    version/version.cpp
    args/args.cpp
    renderer/vulkan_renderer.cpp
    renderer/vulkan_instance.cpp
    renderer/vulkan_device.cpp
    renderer/vulkan_swapchain.cpp
    renderer/vulkan_pipeline.cpp
    renderer/vulkan_utils.cpp
)

target_sources(candela-3 PRIVATE 
    FILE_SET cxx_modules TYPE CXX_MODULES
    FILES 
        version/version.cppm
        args/args.cppm
        renderer/renderer.cppm
)

set_target_properties(candela-3 PROPERTIES 
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_MODULE_STD ON
    # CXX_SCAN_FOR_MODULES ON
    COMPILE_WARNING_AS_ERROR OFF
    DEBUG_POSTFIX "_d"
)

target_compile_options(candela-3 PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4> # Should add /WX too
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

# Install Candela-3
install(TARGETS candela-3 
    RUNTIME_DEPENDENCY_SET candela_runtime
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# install(RUNTIME_DEPENDENCY_SET candela_runtime
#     DESTINATION ${CMAKE_INSTALL_BINDIR}
# )



find_program(SLANGC_EXECUTABLE NAMES slangc REQUIRED)

function (add_slang_shader_target TARGET)
    # Parse ARGN
    set(multiValueArgs SOURCES ENTRY_POINTS)
    cmake_parse_arguments ("SHADER" "" "OUTPUT" "${multiValueArgs}" ${ARGN})

    # Argument validation
    if(NOT SHADER_SOURCES)
        message(FATAL_ERROR "add_slang_shader_target: SOURCES is required")
    endif()

    if(NOT SHADER_ENTRY_POINTS)
        set(SHADER_ENTRY_POINTS vertMain fragMain)
    endif()

    if(NOT SHADER_OUTPUT)
        set(SHADER_OUTPUT "${TARGET}.spv")
    endif()

    set(SHADER_DIR_NAME "shaders")
    set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_DIR_NAME}")
    set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_DIR_NAME}")
    set(OUTPUT_FILE "${SHADER_OUTPUT_DIR}/${SHADER_OUTPUT}")
    set(DEP_FILE "${OUTPUT_FILE}.dep")

    foreach(src IN LISTS SHADER_SOURCES)
        list(APPEND SHADER_ABS_SOURCES "${SHADER_SOURCE_DIR}/${src}")
    endforeach()

    foreach(ep IN LISTS SHADER_ENTRY_POINTS)
        list(APPEND SHADER_EPS -entry "${ep}")
    endforeach()

    add_custom_command(
        OUTPUT  "${OUTPUT_FILE}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${SHADER_OUTPUT_DIR}"
        COMMAND "${SLANGC_EXECUTABLE}" ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${SHADER_EPS} -o "${OUTPUT_FILE}" -depfile "${DEP_FILE}"
        DEPENDS ${SHADER_ABS_SOURCES}
        DEPFILE "${DEP_FILE}"
        WORKING_DIRECTORY "${SHADER_SOURCE_DIR}"
        COMMENT "Compiling Slang shaders -> ${OUTPUT_FILE}"
        VERBATIM
    )
    add_custom_target (${TARGET} DEPENDS "${OUTPUT_FILE}")
    install(FILES "${OUTPUT_FILE}" DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DIR_NAME}")
endfunction()

add_slang_shader_target(shaders SOURCES shader.slang)
add_dependencies(candela-3 shaders)

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
