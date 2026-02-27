project(core)

add_library(configuration)

target_sources(configuration PRIVATE
    configuration/configuration_node.cpp
    configuration/list_node.cpp
    configuration/literal_node.cpp
    configuration/object_node.cpp
    configuration/object_node_iterator.cpp
)

# target_sources(configuration PUBLIC 
#     FILE_SET public_modules TYPE CXX_MODULES 
#     FILES 
#         configuration/configuration.cppm
# )

target_sources(configuration PUBLIC 
    FILE_SET cxx_modules TYPE CXX_MODULES 
    FILES
        configuration/configuration.cppm
        configuration/configuration_node.cppm
        configuration/literal_node.cppm
        configuration/list_node.cppm
        configuration/object_node.cppm
        configuration/object_node_types.cppm
        configuration/object_node_iterator.cppm
)

set_target_properties(configuration PROPERTIES 
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_MODULE_STD ON
    # CXX_SCAN_FOR_MODULES ON
    COMPILE_WARNING_AS_ERROR ON
    DEBUG_POSTFIX "_d"
)
