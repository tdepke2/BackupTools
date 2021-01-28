#include "Application.h"
#include <iostream>

void Application::printPaths(const fs::path& filename) {
    try {
        fileHandler_.loadConfigFile(filename);
        for (WriteReadPath nextPath = fileHandler_.getNextWriteReadPath(); !nextPath.isEmpty(); nextPath = fileHandler_.getNextWriteReadPath()) {
            std::cout << "        printPaths() [" << nextPath.writePath << "] ->\n";
            std::cout << "                     [" << nextPath.readAbsolute << "]   [" << nextPath.readLocal << "]\n";
        }
    } catch (std::exception& ex) {
        std::cout << "Error: " << ex.what() << "\n";
    }
}
