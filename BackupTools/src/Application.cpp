#include "Application.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>

char Application::pathSeparator = std::filesystem::path::preferred_separator;

// Helper function for fnmatchSimple().
bool fnmatchSimple_(char const* pattern, char const* str) {
    while (*str != '\0' && *str != Application::pathSeparator) {
        if (*pattern == '*') {    // Star matches zero to n characters. Does not match a leading dot in a name.
            do {    // Skip consecutive stars (globstar not supported).
                ++pattern;
            } while (*pattern == '*');
            
            if (fnmatchSimple_(pattern, str)) {    // Attempt to match zero characters.
                return true;
            }
            do {
                ++str;
                if (fnmatchSimple_(pattern, str)) {    // Skip character and attempt sub-match again (includes matching against null character).
                    return true;
                }
            } while (*str != '\0' && *str != Application::pathSeparator);
            
            return false;
        } else if (*pattern == '?') {    // Question mark matches any one character. Does not match a leading dot in a name.
            ++pattern;
            ++str;
        } else if (*pattern == '[') {    // Brackets match any characters contained within (including other brackets, a ] must come first) except when brackets are empty. Can match leading dot unlike on UNIX fnmatch.
            ++pattern;
            bool invertSearch = false;
            if (*pattern == '\0' || *pattern == Application::pathSeparator) {    // Case where [ is remaining pattern.
                return *str == '[';
            } else if (*pattern == '!' || *pattern == '^') {    // Inverted search.
                invertSearch = true;
                ++pattern;
                if (*pattern == '\0' || *pattern == Application::pathSeparator) {    // Case where [! or [^ is remaining pattern.
                    return *str == '[' && *(str + 1) == *(pattern - 1);
                }
            }
            char const* endingBracket = pattern;
            while (true) {
                ++endingBracket;
                if (*endingBracket == '\0' || *endingBracket == Application::pathSeparator) {
                    if (*pattern == ']') {
                        if (invertSearch) {    // Case where [!]* or [^]* is remaining pattern (and no more ] left).
                            if (*str != *(pattern - 1)) {
                                return false;
                            }
                        } else {    // Case where []* is remaining pattern (and no more ] left).
                            if (*str != '[' && *(str + 1) != ']') {
                                return false;
                            }
                            ++str;
                        }
                        ++pattern;
                    } else {    // Else, there is no end bracket.
                        if (*str != '[') {
                            return false;
                        }
                        pattern -= (invertSearch ? 1 : 0);
                    }
                    ++str;    // Remaining string must lexically match.
                    break;
                } else if (*endingBracket == ']') {
                    bool matchedChar = false;
                    while (pattern != endingBracket) {    // Traverse to bracket again and check for a match.
                        if (*pattern == *str) {
                            matchedChar = true;
                        }
                        ++pattern;
                    }
                    if (matchedChar != invertSearch) {    // Check if a character in the brackets was matched in str.
                        ++pattern;
                        ++str;
                    } else {
                        return false;
                    }
                    break;
                }
            }
        } else if (*pattern == *str) {    // Else, characters should match exactly.
            ++pattern;
            ++str;
        } else {
            return false;
        }
    }
    
    while (*pattern == '*') {    // Skip trailing stars.
        ++pattern;
    }
    return *pattern == '\0' || *pattern == Application::pathSeparator;
}

// Alternative fnmatch version for cases with either no path separators, or path separators only at the end of pattern and str.
bool fnmatchSimple(char const* pattern, char const* str) {
    // Star does not match a leading dot in a name (because it's not supposed to match hidden files or the . and .. directories). Question mark does not match a leading dot in a name.
    if ((*pattern == '*' && *str == '.') || (*pattern == '?' && *str == '.')) {
        return false;
    }
    
    return fnmatchSimple_(pattern, str);
}

/** Implementation of the unix fnmatch(3) function. Has a bit fewer options but still matches most patterns decently well.
    Alternative for windows (not very good though): https://stackoverflow.com/questions/35877738/windows-fnmatch-substitute
    Simpler but less powerful version: https://stackoverflow.com/questions/3300419/file-name-matching-with-wildcard
    Glob mechanics: https://www.man7.org/linux/man-pages/man7/glob.7.html    https://www.gnu.org/software/bash/manual/html_node/Pattern-Matching.html
    
    Wildcards: ? * [abc] [a-z] [!abc] [!a-z]
    ? matches any single character (except leading . in filename).
    * matches any number of characters (except leading . in filename, which can be matched with .* pattern).
    [] matches any single character in the brackets (to match a ] it must be first in the list, besides the optional ! or ^). An empty list is matched as it is (it matches to [] string).
        Use a ! or ^ as first character to invert and only match if the character is not contained in the list.
        Use a - to specify a range, the range only matches if left character is less than/equal to right. Range can include non-alphanumeric characters. Put - as first (with exception of ! or ^) or last character to match the - instead.
    ** is not currently supported (globstar).
*/
bool Application::fnmatchPortable(char const* pattern, char const* str) {
    while (true) {
        if (!fnmatchSimple(pattern, str)) {
            return false;
        }
        while (*pattern != pathSeparator && *pattern != '\0') {
            ++pattern;
        }
        while (*str != pathSeparator && *str != '\0') {
            ++str;
        }
        if (*pattern != *str) {
            return false;
        } else if (*pattern == '\0') {
            return true;
        }
        ++pattern;
        ++str;
    }
    return false;
}

std::vector<fs::path> Application::globPortable(const fs::path& pattern) {
    //const fs::path current = fs::current_path();
    
    std::vector<fs::path> result;
    
    auto patternIter = pattern.begin();
    //auto currentIter = current.begin();
    if (pattern.root_directory().empty()) {    // Check if pattern is relative.
        result.push_back("relative");
    } else {    // Pattern is absolute.
        
        // Note: Since fs::directory_iterator passes over the . and .. hardlinks, may be necessary to only use absolute paths (with the make_absolute thingy) and normalize paths first?
        // ######################################################################################################
        
        if (pattern.has_root_name()) {    // Skip root name.
            ++patternIter;
        }
        ++patternIter;    // Skip root directory.
        
        
        
        fs::path patternTraversal = pattern.root_path();
        while (patternIter != pattern.end()) {
            for (const auto& entry : fs::directory_iterator(patternTraversal)) {
                if (entry.path().filename() == *patternIter) {
                    result.push_back(patternTraversal / *patternIter);
                }
                //result.push_back(entry.path().filename());
            }
            
            patternTraversal /= *patternIter;
            result.push_back("============================");
            ++patternIter;
        }
        
    }
    
    return result;
}

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
    
    lineNumber_ = 0;
    rootPaths_.clear();
    ignorePaths_.clear();
    previousReadPaths_.clear();
    writePath_.clear();
    readPath_.clear();
    writePathSet_ = false;
    readPathSet_ = false;
}

void Application::printPaths() {
    for (std::pair<fs::path, fs::path> nextPath = getNextWriteReadPath(); !nextPath.first.empty() || !nextPath.second.empty(); nextPath = getNextWriteReadPath()) {
        
        std::cout << "        printPaths() [" << nextPath.first << "] <- [" << nextPath.second << "]\n";
    }
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

void Application::parseNextLineInFile() {
    std::string line;
    if (!getline(configFile_, line)) {
        configFile_.close();
        return;
    }
    
    ++lineNumber_;
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
    std::cout << "Line " << lineNumber_ << ": [" << line << "]\n";
    if (index >= line.length() || line[index] == '#') {
        return;
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
        writePath_ = substituteRootPath(parseNextPath(index, line));
        writePathSet_ = true;
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
            readPath_ = substituteRootPath(parseNextPath(index, line));
            readPathSet_ = true;
            
            std::cout << "    In: [" << writePath_ << "] [" << readPath_ << "]\n";
        }
    } else if (command == "add") {    // Syntax (write path must have previously been set): add <read path>
        skipWhitespace(index, line);
        if (!writePathSet_) {
            std::cout << "Error: Missing write path.\n";
        }
        if (index >= line.length()) {
            std::cout << "Error: Missing path.\n";
        }
        readPath_ = substituteRootPath(parseNextPath(index, line));
        readPathSet_ = true;
        
        std::cout << "    Add: [" << writePath_ << "] [" << readPath_ << "]\n";
    } else if (command == "ignore") {    // Syntax: ignore <path>
        skipWhitespace(index, line);
        if (index >= line.length()) {
            std::cout << "Error: Missing path.\n";
        }
        fs::path ignorePath = substituteRootPath(parseNextPath(index, line));
        
        std::cout << "    Ignore: [" << ignorePath << "]\n";
        
        ignorePaths_.insert(ignorePath);
    } else if (command == "include") {    // Syntax: include <path>
        skipWhitespace(index, line);
        if (index >= line.length()) {
            std::cout << "Error: Missing path.\n";
        }
        fs::path includePath = substituteRootPath(parseNextPath(index, line));
        
        std::cout << "    Include: [" << includePath << "]\n";
        
        auto findResult = ignorePaths_.find(includePath);
        if (findResult != ignorePaths_.end()) {
            ignorePaths_.erase(findResult);
        }
    } else {
        std::cout << "Error: Unknown command [" << command << "]\n";
    }
    skipWhitespace(index, line);
    if (index < line.length()) {
        std::cout << "Error: Unexpected data.\n";
    }
}

std::pair<fs::path, fs::path> Application::getNextWriteReadPath() {
    while (!readPathSet_) {
        if (!configFile_.is_open()) {
            return {fs::path(), fs::path()};
        }
        parseNextLineInFile();
    }
    
    //for () {
        
    //}
    
    std::cout << ".==============.\n";
    auto vec = globPortable(readPath_);
    for (auto& x : vec) {
        std::cout << x << "\n";
    }
    std::cout << "\'==============\'\n";
    
    readPathSet_ = false;
    
    return {writePath_, readPath_};
}
