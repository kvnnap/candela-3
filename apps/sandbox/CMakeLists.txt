
project(candela-sandbox)

find_package(Vulkan REQUIRED)

add_executable(candela-sandbox)
# target_link_libraries(candela-sandbox PRIVATE Vulkan::Vulkan)
target_link_libraries(candela-sandbox PRIVATE VulkanCppModule)

target_sources(candela-sandbox PRIVATE 
    main.cpp 
)

target_sources(candela-sandbox PRIVATE 
    FILE_SET cxx_modules TYPE CXX_MODULES
    FILES 
        test_std.cppm
)

set_target_properties(candela-sandbox PROPERTIES 
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_MODULE_STD ON
    # CXX_SCAN_FOR_MODULES ON
    COMPILE_WARNING_AS_ERROR OFF
    DEBUG_POSTFIX "_d"
)

target_compile_options(candela-sandbox PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4> # Should add /WX too
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

install(TARGETS candela-sandbox 
    RUNTIME_DEPENDENCY_SET candela_runtime
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)