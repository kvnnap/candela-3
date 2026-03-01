project(external)

find_package(cxxopts CONFIG REQUIRED)

add_library(external)

target_link_libraries(external PRIVATE cxxopts::cxxopts)

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
        cxxopts.cppm
)

set_target_properties(external PROPERTIES 
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_MODULE_STD ON
    # CXX_SCAN_FOR_MODULES ON
    COMPILE_WARNING_AS_ERROR ON
    DEBUG_POSTFIX "_d"
)
