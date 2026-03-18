cmake_minimum_required(VERSION 4.3.0)

include(GNUInstallDirs)

# Set experimental flag to enable `import std` support from CMake.
# This must be enabled before C++ language support.
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  # This specific value changes as experimental support evolves. See
  # `Help/dev/experimental.rst` in the CMake source corresponding to
  # your CMake build for the exact value to use.
  "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")

set(CMAKE_CXX_EXTENSIONS OFF)

project(candela-solution)

# The dependency set can be used by all relatable 
install(RUNTIME_DEPENDENCY_SET candela_runtime
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  PRE_EXCLUDE_REGEXES
    [[api-ms-]]
    [[ext-ms-]]
    [[kernel32\.dll]]
    [[vulkan-1\.dll]]
    [[libvulkan\.so(\..*)?]]
    [[libc\.so(\..*)?]] [[libgcc_s\.so(\..*)?]] [[libm\.so(\..*)?]] [[libstdc\+\+\.so(\..*)?]]
  POST_EXCLUDE_REGEXES
    [[.*system32\/.*\.dll]]
    [[^\/(lib|usr\/lib|usr\/local\/lib)]]
  # DIRECTORIES ${CONAN_RUNTIME_LIB_DIRS} $<TARGET_FILE_DIR:betaimpl>
  DIRECTORIES C:\\toolchains\\llvm-mingw-20251216-ucrt-x86_64\\bin
)

add_subdirectory(libs/external)
add_subdirectory(libs/core)
add_subdirectory(apps/candela-3)
add_subdirectory(apps/sandbox)
