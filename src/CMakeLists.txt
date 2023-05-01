# Create a library for the sources (makes it easy to link the tests to these files).
set(HEADER_LIST 
    "${backup_tools_SOURCE_DIR}/include/BackupTools/Application.h"
    "${backup_tools_SOURCE_DIR}/include/BackupTools/ArgumentParser.h"
    "${backup_tools_SOURCE_DIR}/include/BackupTools/FileHandler.h"
)

# It's recommended to list source files explicitly instead of using a glob.
# Headers are included so that they appear when using an IDE.
add_library(backup_tools_lib
    Application.cpp
    ArgumentParser.cpp
    FileHandler.cpp
    ${HEADER_LIST}
)

# An alternative method to using source_group() is to list headers as PUBLIC,
# but that will allow them to show up for every dependent target.
#
#add_library(backup_tools_lib)
#target_sources(backup_tools_lib
#PUBLIC
#    "${backup_tools_SOURCE_DIR}/include/BackupTools/Application.h"
#    "${backup_tools_SOURCE_DIR}/include/BackupTools/ArgumentParser.h"
#    "${backup_tools_SOURCE_DIR}/include/BackupTools/FileHandler.h"
#PRIVATE
#    Application.cpp
#    ArgumentParser.cpp
#    FileHandler.cpp
#)

# Specify where to find the header files to include.
target_include_directories(backup_tools_lib PUBLIC ../include)

# Organize headers in IDE.
source_group(
    TREE "${PROJECT_SOURCE_DIR}/include"
    PREFIX "Header Files"
    FILES ${HEADER_LIST}
)

# Create the main executable using the library.
add_executable(backup_tools main.cpp)

target_link_libraries(backup_tools PRIVATE backup_tools_lib)
