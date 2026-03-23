project(external)

# find_package(cxxopts CONFIG REQUIRED)
# find_package(CLI11 CONFIG REQUIRED)
# find_package(lyra CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_library(external)

# install(TARGETS external 
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
# )

# install(IMPORTED_RUNTIME_ARTIFACTS glfw
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
# )

# target_link_libraries(external PRIVATE cxxopts::cxxopts)
# target_link_libraries(external PRIVATE CLI11::CLI11 bfg::lyra)
target_link_libraries(external PRIVATE glfw)


# target_sources(external PUBLIC
#     FILE_SET public_headers TYPE HEADERS
#     BASE_DIRS
#         ${CMAKE_CURRENT_SOURCE_DIR}
#     FILES
#         system/shared_library.h
# )

# target_sources(external PRIVATE
#     configuration/configuration_node.cpp
# )

target_sources(external PUBLIC 
    FILE_SET cxx_modules TYPE CXX_MODULES 
    FILES
        glfw.cppm
)

set_target_properties(external PROPERTIES 
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_MODULE_STD ON
    # CXX_SCAN_FOR_MODULES ON
    COMPILE_WARNING_AS_ERROR ON
    DEBUG_POSTFIX "_d"
)


find_package(VulkanHeaders CONFIG)

# set up Vulkan C++ module (enabled when ENABLE_CPP20_MODULE=ON)
add_library(VulkanCppModule)
add_library(Vulkan::cppm ALIAS VulkanCppModule)

target_compile_definitions(VulkanCppModule PRIVATE
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
    VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS=1
    VULKAN_HPP_CXX_MODULE_EXPERIMENTAL_WARNING=1
)

target_link_libraries(VulkanCppModule PUBLIC Vulkan::Headers)
# set_target_properties(VulkanCppModule PROPERTIES CXX_STANDARD 23 CXX_MODULE_STD ON)
set_target_properties(VulkanCppModule PROPERTIES 
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_MODULE_STD ON
    # CXX_SCAN_FOR_MODULES ON
    COMPILE_WARNING_AS_ERROR ON
    DEBUG_POSTFIX "_d"
)

get_target_property(VKH_INCLUDE_DIR Vulkan::Headers INTERFACE_INCLUDE_DIRECTORIES)
target_sources(VulkanCppModule PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        BASE_DIRS 
            "${VKH_INCLUDE_DIR}"
        FILES 
            "${VKH_INCLUDE_DIR}/vulkan/vulkan.cppm"
            "${VKH_INCLUDE_DIR}/vulkan/vulkan_video.cppm"
)
