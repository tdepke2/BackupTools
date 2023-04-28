# Create the executable from all source files.
# It's recommended to list source files explicitly instead of using a glob.
add_executable(backup_tools
    Application.cpp
    ArgumentParser.cpp
    FileHandler.cpp
    main.cpp
)

# Specify where to find the header files to include.
target_include_directories(backup_tools PRIVATE ../include)
