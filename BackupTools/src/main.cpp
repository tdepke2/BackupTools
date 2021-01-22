#include "Application.h"
#include <iostream>
#include <string>

// Quick desc:
// CLI backup tool ideal for backing up files to a flash drive. Target filename to include/exclude are stored in a config file.
// Supports diff checking, manual backup, and automatic backups. Ideally should work on linux too, and use CMake and TDD.

bool fnmatchPortable(const std::string& pattern, const std::string& str, std::string::size_type pIndex = 0, std::string::size_type sIndex = 0);

int main(int argc, char** argv) {
    Application app;
    bool result = app.checkFileEquivalence("sandbox/tmp.txt", "sandbox/a.txt");
    std::cout << "app.checkFileEquivalence = " << result << "\n";
    
    app.loadFile("sample.txt");
    app.printPaths();
    
    return 0;
}
