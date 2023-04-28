cmake_minimum_required(VERSION 3.14...3.26)


# Set project name and version.
project(backup_tools
    VERSION 1.0
    DESCRIPTION "Simple automatic and manual backups for local storage"
    LANGUAGES CXX
)

# Specify C++17 standard (for std::filesystem library).
set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Create the executable from all source files.
# It's recommended to list source files explicitly instead of using a glob.
add_executable(backup_tools
    BackupTools/src/Application.cpp
    BackupTools/src/ArgumentParser.cpp
    BackupTools/src/FileHandler.cpp
    BackupTools/src/main.cpp
)

# Specify where to find the header files to include.
target_include_directories(backup_tools
PRIVATE
    ${PROJECT_SOURCE_DIR}/BackupTools/include
)



include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  hello_test
  BackupTools/src/ArgumentParser.cpp
  BackupTools/src/FileHandler.cpp
  Tests/test.cpp
)
target_include_directories(hello_test
PRIVATE
    ${PROJECT_SOURCE_DIR}/BackupTools/include
)
target_link_libraries(
  hello_test PUBLIC gtest_main
)

include(GoogleTest)
gtest_discover_tests(hello_test)
