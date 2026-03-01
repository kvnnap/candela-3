project(core)

find_package(RapidJSON CONFIG REQUIRED)

add_library(core)

target_link_libraries(core PRIVATE RapidJSON rapidjson)

target_sources(core PUBLIC
    FILE_SET public_headers TYPE HEADERS
    BASE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
        system/shared_library.h
)

target_sources(core PRIVATE
    configuration/configuration_node.cpp
    configuration/list_node.cpp
    configuration/literal_node.cpp
    configuration/object_node.cpp
    configuration/object_node_iterator.cpp

    configuration/parser/json_configuration_parser.cpp

    system/shared_library.cpp
)

# target_sources(configuration PUBLIC 
#     FILE_SET public_modules TYPE CXX_MODULES 
#     FILES 
#         configuration/configuration.cppm
# )

target_sources(core PUBLIC 
    FILE_SET cxx_modules TYPE CXX_MODULES 
    FILES
        configuration/configuration.cppm
        configuration/configuration_node.cppm
        configuration/literal_node.cppm
        configuration/list_node.cppm
        configuration/object_node.cppm
        configuration/object_node_types.cppm
        configuration/object_node_iterator.cppm
        configuration/parser/parser.cppm

        factory/factory.cppm

        environment/manager.cppm

        system/shared_library.cppm

        util/util.cppm

        plugin/plugin.cppm
)

set_target_properties(core PROPERTIES 
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_MODULE_STD ON
    # CXX_SCAN_FOR_MODULES ON
    COMPILE_WARNING_AS_ERROR ON
    DEBUG_POSTFIX "_d"
)
