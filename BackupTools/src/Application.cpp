#include "Application.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>

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

void Application::loadFile(const fs::path& filename) {
    if (configFile_.is_open()) {
        configFile_.close();
    }
    configFile_.open(filename);
    if (!configFile_.is_open()) {
        throw std::runtime_error("Unable to open file.");
    }
}

void Application::printPaths() {
    /*while (hasNextPath()) {
        std::pair<fs::path, fs::path> nextPath = getNextPath();
        cout << "[" << nextPath.first << "] <- [" << nextPath.second << "]\n";
    }*/
    
    std::string line;
    unsigned int lineNumber = 0;
    fs::path writePath;
    bool writePathSet = false;
    while (getline(configFile_, line)) {
        ++lineNumber;
        /*std::string::size_type lastNonSpace = line.find_last_not_of(' ');    // Trim whitespace.
        if (lastNonSpace != std::string::npos && lastNonSpace != line.length() - 1) {
            line.erase(lastNonSpace + 1);
        }
        std::string::size_type firstNonSpace = line.find_first_not_of(' ');
        if (firstNonSpace != std::string::npos && firstNonSpace != 0) {
            line.erase(0, firstNonSpace);
        }*/
        std::string::size_type index = 0;
        skipWhitespace(index, line);
        std::cout << "Line " << lineNumber << ": [" << line << "]\n";
        if (index >= line.length() || line[index] == '#') {
            continue;
        }
        
        std::string command = parseNextCommand(index, line);
        if (command == "root") {    // Syntax: root <identifier> <replacement path>
            skipWhitespace(index, line);
            if (index >= line.length()) {
                std::cout << "Error: Missing first path.\n";
            }
            fs::path keyPath = parseNextPath(index, line);
            skipWhitespace(index, line);
            fs::path valuePath = parseNextPath(index, line);
            std::cout << "    Root: [" << keyPath << "] [" << valuePath << "]\n";
            
            rootPaths_.insert({keyPath, valuePath});
        } else if (command == "in") {    // Syntax: in <write path> [add <read path>]
            skipWhitespace(index, line);
            if (index >= line.length()) {
                std::cout << "Error: Missing first path.\n";
            }
            writePath = substituteRootPath(parseNextPath(index, line));
            writePathSet = true;
            skipWhitespace(index, line);
            if (index < line.length()) {
                command = parseNextCommand(index, line);
                if (command != "add") {
                    std::cout << "Error: Unexpected data.\n";
                }
                skipWhitespace(index, line);
                if (index >= line.length()) {
                    std::cout << "Error: Missing path.\n";
                }
                fs::path readPath = substituteRootPath(parseNextPath(index, line));
                
                std::cout << "    In: [" << writePath << "] [" << readPath << "]\n";
            }
        } else if (command == "add") {    // Syntax (write path must have previously been set): add <read path>
            skipWhitespace(index, line);
            if (!writePathSet) {
                std::cout << "Error: Missing write path.\n";
            }
            if (index >= line.length()) {
                std::cout << "Error: Missing path.\n";
            }
            fs::path readPath = substituteRootPath(parseNextPath(index, line));
            
            std::cout << "    Add: [" << writePath << "] [" << readPath << "]\n";
        } else if (command == "ignore") {    // Syntax: ignore <path>
            skipWhitespace(index, line);
            if (index >= line.length()) {
                std::cout << "Error: Missing path.\n";
            }
            fs::path ignorePath = substituteRootPath(parseNextPath(index, line));
            
            std::cout << "    Ignore: [" << ignorePath << "]\n";
        } else if (command == "include") {    // Syntax (path must match previously ignored path): include <path>
            skipWhitespace(index, line);
            if (index >= line.length()) {
                std::cout << "Error: Missing path.\n";
            }
            fs::path includePath = substituteRootPath(parseNextPath(index, line));
            
            std::cout << "    Include: [" << includePath << "]\n";
        } else {
            std::cout << "Error: Unknown command [" << command << "]\n";
        }
        skipWhitespace(index, line);
        if (index < line.length()) {
            std::cout << "Error: Unexpected data.\n";
        }
    }
    
    configFile_.close();
}

void Application::skipWhitespace(std::string::size_type& index, const std::string& str) {
    while (index < str.length() && str[index] == ' ') {
        ++index;
    }
}

std::string Application::parseNextCommand(std::string::size_type& index, const std::string& str) {
    std::string::size_type start = index;
    while (index < str.length()) {
        if (str[index] == ' ') {
            return str.substr(start, index - start);
        }
        ++index;
    }
    return str.substr(start);
}

fs::path Application::parseNextPath(std::string::size_type& index, const std::string& str) {
    std::string::size_type start = index;
    if (index < str.length() && str[index] == '\"') {
        ++index;
        while (index < str.length()) {
            if (str[index] == '\"') {
                ++index;
                return fs::path(str.substr(start + 1, index - start - 2)).lexically_normal();
            }
            ++index;
        }
    } else {
        while (index < str.length()) {
            if (str[index] == ' ') {
                return fs::path(str.substr(start, index - start)).lexically_normal();
            }
            ++index;
        }
    }
    return fs::path(str.substr(start)).lexically_normal();
}

fs::path Application::substituteRootPath(const fs::path& path) {
    auto pathIter = path.begin();
    if (pathIter != path.end()) {
        auto findResult = rootPaths_.find(*pathIter);
        if (findResult != rootPaths_.end()) {
            return findResult->second / fs::path(path.string().substr(pathIter->string().length() + 1));
        }
    }
    return path;
}

bool Application::hasNextPath() const {
    return false;
}

std::pair<fs::path, fs::path> Application::getNextPath() {
    return std::make_pair<fs::path, fs::path>({}, {});
}
