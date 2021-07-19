cmake_minimum_required(VERSION 3.10)

# Set project name and version.
project(backup_tools)

# Specify C++17 standard (for std::filesystem library).
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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
