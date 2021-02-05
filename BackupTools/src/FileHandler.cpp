#include "FileHandler.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>
#include <numeric>
#include <stack>
#include <stdexcept>

char FileHandler::pathSeparator = std::filesystem::path::preferred_separator;

bool compareFilename(const fs::path& lhs, const fs::path& rhs) {
    // Alternative method for cases like "lowercase must be sorted before uppercase" is to use collation table. https://stackoverflow.com/questions/19509110/sorting-a-string-with-stdsort-so-that-capital-letters-come-after-lower-case
    const std::string lhsString = lhs.string(), rhsString = rhs.string();
    size_t i = 0, minSize = std::min(lhsString.size(), rhsString.size());
    while (std::tolower(static_cast<unsigned char>(lhsString[i])) == std::tolower(static_cast<unsigned char>(rhsString[i])) && i < minSize) {
        ++i;
    }
    return std::tolower(static_cast<unsigned char>(lhsString[i])) < std::tolower(static_cast<unsigned char>(rhsString[i]));
}

// Helper function for fnmatchSimple().
bool fnmatchSimple_(char const* pattern, char const* str) {
    while (*str != '\0' && *str != FileHandler::pathSeparator) {
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
            } while (*str != '\0' && *str != FileHandler::pathSeparator);
            
            return false;
        } else if (*pattern == '?') {    // Question mark matches any one character. Does not match a leading dot in a name.
            ++pattern;
            ++str;
        } else if (*pattern == '[') {    // Brackets match any characters contained within (including other brackets, a ] must come first) except when brackets are empty. Can match leading dot unlike on UNIX fnmatch.
            ++pattern;
            bool invertSearch = false;
            if (*pattern == '\0' || *pattern == FileHandler::pathSeparator) {    // Case where [ is remaining pattern.
                return *str == '[';
            } else if (*pattern == '!' || *pattern == '^') {    // Inverted search.
                invertSearch = true;
                ++pattern;
                if (*pattern == '\0' || *pattern == FileHandler::pathSeparator) {    // Case where [! or [^ is remaining pattern.
                    return *str == '[' && *(str + 1) == *(pattern - 1);
                }
            }
            char const* endingBracket = pattern;
            while (true) {
                ++endingBracket;
                if (*endingBracket == '\0' || *endingBracket == FileHandler::pathSeparator) {
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
                        if (*(pattern + 1) == '-' && pattern + 2 != endingBracket) {    // Check for characters between range (left character must not be greater than right).
                            char c = *pattern;
                            pattern += 2;
                            while (c <= *pattern) {
                                if (c == *str) {
                                    matchedChar = true;
                                }
                                ++c;
                            }
                        } else if (*pattern == *str) {
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
    return *pattern == '\0' || *pattern == FileHandler::pathSeparator;
}

// Alternative fnmatch version for cases with either no path separators, or path separators only at the end of pattern and str. Called by fnmatchPortable().
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
        Can match leading . in a filename, unlike how this behaves on unix.
        Use a ! or ^ as first character to invert and only match if the character is not contained in the list.
        Use a - to specify a range, the range only matches if left character is less than/equal to right. Range can include non-alphanumeric characters. Put - as first (with exception of ! or ^) or last character to match the - instead.
    ** is not currently supported (globstar). It is supported in globPortable() though.
*/
bool FileHandler::fnmatchPortable(char const* pattern, char const* str) {
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

bool FileHandler::containsWildcard(char const* pattern) {
    while (*pattern != '\0') {
        if (*pattern == '*' || *pattern == '?') {
            return true;
        } else if (*pattern == '[') {
            ++pattern;
            if (*pattern == '\0') {
                return false;
            }
            char const* endingBracket = pattern;
            do {
                ++endingBracket;
                if (*endingBracket == ']') {
                    return true;
                }
            } while (*endingBracket != '\0');
        } else {
            ++pattern;
        }
    }
    return false;
}

/** Globbing details:
    If last directory name does not contain wildcards, the directory is matched recursively (and all files stem from the directory name).
    * will skip directories (even empty ones) when used in last directory name.
    ** will include everything (matches zero to n directories and files). Similar to just putting the directory name, but files do not stem from the name.
        For simplicity, ** only works when by itself in a sub-path.
    For file/directory specific matching, the rules in fnmatchPortable() apply.
*/
std::vector<std::pair<fs::path, fs::path>> FileHandler::globPortable(fs::path pattern) {
    std::vector<std::pair<fs::path, fs::path>> result;
    
    if (pattern.root_directory().empty()) {    // If pattern is relative, make it absolute.
        pattern = (fs::current_path() / pattern).lexically_normal();
    }
    bool addedTrailingGlobstar = false;
    if (!containsWildcard(pattern.filename().string().c_str())) {    // If last sub-path is not a glob, assume it is a directory and match contents recursively.
        pattern /= "**";
        addedTrailingGlobstar = true;
    }
    
    auto patternIter = pattern.begin();
    if (pattern.has_root_name()) {    // Skip root name.
        ++patternIter;
        if (patternIter == pattern.end()) {
            return result;
        }
    }
    ++patternIter;    // Skip root directory.
    if (patternIter == pattern.end()) {
        return result;
    }
    
    fs::path directoryPrefix = pattern.root_path();
    auto patternIter2 = patternIter;
    ++patternIter2;
    
    while (patternIter2 != pattern.end() && !containsWildcard(patternIter2->string().c_str())) {    // Determine the directoryPrefix (the longest path in the pattern without wildcards).
        directoryPrefix /= *patternIter;
        patternIter = patternIter2;
        ++patternIter2;
    }
    if (!addedTrailingGlobstar) {
        directoryPrefix /= *patternIter;
        ++patternIter;
    }
    if (!fs::exists(directoryPrefix)) {
        return result;
    }
    std::string::size_type dirPrefixOffset = directoryPrefix.string().length() + 1;
    
    std::stack<fs::path> pathStack;
    pathStack.push(directoryPrefix);
    std::stack<fs::path::iterator> iterStack;
    iterStack.push(patternIter);
    
    while (!pathStack.empty()) {    // Recursive operation to iterate through matching directories.
        fs::path pathTraversal = pathStack.top();    // The matched path thus far.
        pathStack.pop();
        fs::path::iterator currentPatternIter = iterStack.top();    // Iterator to the current sub-path.
        iterStack.pop();
        
        if (currentPatternIter == pattern.end()) {
            continue;
        }
        
        //std::cout << "Current pathTraversal is " << pathTraversal << "\n";
        //std::cout << "Current sub-pattern is " << *currentPatternIter << "\n";
        fs::path::iterator nextPatternIter = std::next(currentPatternIter);
        
        bool matchAllPaths = false;
        bool addToResult = true;
        if (*currentPatternIter == fs::path("**")) {    // If this sub-pattern is a globstar, match current path with the next sub-pattern and all contained directories with the current sub-pattern.
            matchAllPaths = true;
            if (nextPatternIter != pattern.end()) {
                addToResult = false;
            }
            
            pathStack.push(pathTraversal);
            iterStack.push(nextPatternIter);
            nextPatternIter = currentPatternIter;
        }
        
        for (const auto& entry : fs::directory_iterator(pathTraversal)) {
            if (matchAllPaths || fnmatchSimple(currentPatternIter->string().c_str(), entry.path().filename().string().c_str())) {
                //std::cout << "Matched " << entry.path() << "\n";
                if (addToResult) {
                    fs::path nextPathTraversal = pathTraversal / entry.path().filename();
                    result.emplace_back(nextPathTraversal, nextPathTraversal.string().substr(dirPrefixOffset));
                }
                if (entry.is_directory()) {
                    pathStack.push(pathTraversal / entry.path().filename());
                    iterStack.push(nextPatternIter);
                    //std::cout << "    " << pathStack.top() << "\n";
                }
            }
        }
        
        //std::cout << "========================================\n";
    }
    
    return result;
}

bool FileHandler::checkFileEquivalence(const fs::path& source, const fs::path& dest) {    // Based on https://stackoverflow.com/questions/15118661/in-c-whats-the-fastest-way-to-tell-whether-two-string-or-binary-files-are-di
    std::ifstream sourceFile(source, std::ios::ate | std::ios::binary);    // Open files in binary mode and seek to end.
    std::ifstream destFile(dest, std::ios::ate | std::ios::binary);
    if (!sourceFile.is_open() || !destFile.is_open()) {
        if (fs::is_directory(source) && fs::is_directory(dest) && source.filename() == dest.filename()) {
            return true;
        } else {
            return false;
        }
    }
    const std::ios::pos_type sourceSize = sourceFile.tellg();    // Find file sizes.
    const std::ios::pos_type destSize = destFile.tellg();
    if (sourceSize != destSize) {
        return false;
    }
    
    sourceFile.seekg(0);
    destFile.seekg(0);
    
    std::istreambuf_iterator<char> sourceIter(sourceFile);
    std::istreambuf_iterator<char> destIter(destFile);
    
    return std::equal(sourceIter, std::istreambuf_iterator<char>(), destIter);    // Compare streams to check for equality (both streams are same length so this is safe).
}

void FileHandler::skipWhitespace(std::string::size_type& index, const std::string& str) {
    while (index < str.length() && str[index] == ' ') {
        ++index;
    }
}

std::string FileHandler::parseNextWord(std::string::size_type& index, const std::string& str) {
    std::string::size_type start = index;
    while (index < str.length()) {
        if (str[index] == ' ') {
            return str.substr(start, index - start);
        }
        ++index;
    }
    return str.substr(start);
}

fs::path FileHandler::parseNextPath(std::string::size_type& index, const std::string& str) {
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

void FileHandler::loadConfigFile(const fs::path& filename) {
    if (configFile_.is_open()) {
        configFile_.close();
    }
    configFile_.open(filename);
    if (!configFile_.is_open()) {
        throw std::runtime_error("\"" + filename.string() + "\": Unable to open file for reading.");
    }
    
    configFilename_ = filename;
    lineNumber_ = 0;
    rootPaths_.clear();
    ignorePaths_.clear();
    previousReadPaths_.clear();
    writePath_.clear();
    readPath_.clear();
    writePathSet_ = false;
    readPathSet_ = false;
}

WriteReadPath FileHandler::getNextWriteReadPath() {
    WriteReadPath result;
    while (!readPathSet_) {
        if (!configFile_.is_open()) {
            return result;
        }
        parseNextLineInFile();
    }
    
    if (globPortableResults_.empty()) {
        globPortableResults_ = globPortable(readPath_);
        globPortableResultsIndex_ = 0;
    }
    
    if (globPortableResults_.empty()) {
        return result;
    }
    
    result.writePath = writePath_;
    result.readAbsolute = globPortableResults_[globPortableResultsIndex_].first;
    result.readLocal = globPortableResults_[globPortableResultsIndex_].second;
    ++globPortableResultsIndex_;
    if (globPortableResultsIndex_ >= globPortableResults_.size()) {
        globPortableResults_.clear();
        readPathSet_ = false;
    }
    
    return result;
}

fs::path FileHandler::substituteRootPath(const fs::path& path) {
    auto pathIter = path.begin();
    if (pathIter != path.end()) {
        auto findResult = rootPaths_.find(*pathIter);
        if (findResult != rootPaths_.end()) {
            if (std::next(pathIter) == path.end()) {    // If this is last sub-path, return mapped root path as it is.
                return findResult->second;
            } else {
                return findResult->second / fs::path(path.string().substr(pathIter->string().length() + 1));
            }
        }
    }
    return path;
}

void FileHandler::parseNextLineInFile() {
    std::string line;
    if (!getline(configFile_, line)) {
        configFile_.close();
        return;
    }
    
    ++lineNumber_;
    try {
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
        //std::cout << "Line " << lineNumber_ << ": [" << line << "]\n";
        if (index >= line.length() || line[index] == '#') {
            return;
        }
        
        std::string command = parseNextWord(index, line);
        skipWhitespace(index, line);
        if (command == "root") {    // Syntax: root <identifier> <replacement path>
            if (index >= line.length()) {
                throw std::runtime_error("Missing identifier path parameter.");
            }
            fs::path keyPath = parseNextPath(index, line);
            skipWhitespace(index, line);
            if (index >= line.length()) {
                throw std::runtime_error("Missing replacement path parameter.");
            }
            fs::path valuePath = parseNextPath(index, line);
            //std::cout << "    Root: [" << keyPath << "] [" << valuePath << "]\n";
            
            rootPaths_.emplace(keyPath, valuePath);
        } else if (command == "in") {    // Syntax: in <write path> [add <read path>]
            if (index >= line.length()) {
                throw std::runtime_error("Missing write path parameter.");
            }
            writePath_ = substituteRootPath(parseNextPath(index, line));
            writePathSet_ = true;
            skipWhitespace(index, line);
            if (index < line.length()) {
                command = parseNextWord(index, line);
                if (command != "add") {
                    throw std::runtime_error("Unexpected command \"" + command + "\" after \"in <write path>\".");
                }
                skipWhitespace(index, line);
                if (index >= line.length()) {
                    throw std::runtime_error("Missing read path parameter.");
                }
                readPath_ = substituteRootPath(parseNextPath(index, line));
                readPathSet_ = true;
                
                //std::cout << "    In: [" << writePath_ << "] [" << readPath_ << "]\n";
            }
        } else if (command == "add") {    // Syntax (write path must have previously been set): add <read path>
            if (!writePathSet_) {
                throw std::runtime_error("Missing previous call to \"in <write path>\".");
            }
            if (index >= line.length()) {
                throw std::runtime_error("Missing read path parameter.");
            }
            readPath_ = substituteRootPath(parseNextPath(index, line));
            readPathSet_ = true;
            
            //std::cout << "    Add: [" << writePath_ << "] [" << readPath_ << "]\n";
        } else if (command == "ignore") {    // Syntax: ignore <path>
            if (index >= line.length()) {
                throw std::runtime_error("Missing ignore path parameter.");
            }
            fs::path ignorePath = substituteRootPath(parseNextPath(index, line));
            
            //std::cout << "    Ignore: [" << ignorePath << "]\n";
            
            ignorePaths_.emplace(ignorePath);
        } else if (command == "include") {    // Syntax: include <path>
            if (index >= line.length()) {
                throw std::runtime_error("Missing include path parameter.");
            }
            fs::path includePath = substituteRootPath(parseNextPath(index, line));
            
            //std::cout << "    Include: [" << includePath << "]\n";
            
            auto findResult = ignorePaths_.find(includePath);
            if (findResult != ignorePaths_.end()) {
                ignorePaths_.erase(findResult);
            }
        } else {
            throw std::runtime_error("Unknown command \"" + command + "\".");
        }
        skipWhitespace(index, line);
        if (index < line.length()) {
            throw std::runtime_error("Unexpected data after command.");
        }
    } catch (std::exception& ex) {
        throw std::runtime_error("\"" + configFilename_.string() + "\" at line " + std::to_string(lineNumber_) + ": " + ex.what());
    }
}
