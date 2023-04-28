cmake_minimum_required(VERSION 3.14...3.26)

# Set project name and version.
project(backup_tools
    VERSION 1.0
    DESCRIPTION "Simple automatic and manual backups for local storage"
    LANGUAGES CXX
)

# Check if this is the main project (not included with add_subdirectory).
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Specify C++17 standard (for std::filesystem library).
    set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard to use")
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Enable support for folders in IDEs.
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Calls enable_testing and must be in main CMakeLists.
    include(CTest)
endif()

# Add the executable code.
add_subdirectory(src)

#option(BUILD_TESTS "Build the tests" ON)
#if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTS)

# Add tests if this is the main project and testing is enabled.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
endif()
