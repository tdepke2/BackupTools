#include "Application.h"
#include <algorithm>
#include <fstream>
#include <iostream>

bool Application::checkFileEquivalence(const fs::path& source, const fs::path& dest) const {
    /*std::uintmax_t sourceSize, destSize;
    try {
        sourceSize = fs::file_size(source);
        destSize = fs::file_size(dest);
    } catch(...) {
        return false;
    }
    if (sourceSize != destSize) {
        return false;
    }
    
    std::cout << "Sizes match, checking equivalence...\n";
    return ;*/
    
    std::ifstream sourceFile(source, std::ios::ate | std::ios::binary);    // Open files in binary mode and seek to end.
    std::ifstream destFile(dest, std::ios::ate | std::ios::binary);
    if (!sourceFile.is_open() || !destFile.is_open()) {
        return false;
    }
    const std::ios::pos_type sourceSize = sourceFile.tellg();    // Find file sizes.
    const std::ios::pos_type destSize = destFile.tellg();
    std::cout << "Size 1 = " << sourceSize << ", Size 2 = " << destSize << "\n";
    if (sourceSize != destSize) {
        return false;
    }
    
    sourceFile.seekg(0);
    destFile.seekg(0);
    
    std::istreambuf_iterator<char> sourceIter(sourceFile);
    std::istreambuf_iterator<char> destIter(destFile);
    
    return std::equal(sourceIter, std::istreambuf_iterator<char>(), destIter);    // Compare streams to check for equality (both streams are same length so this is safe).
}
