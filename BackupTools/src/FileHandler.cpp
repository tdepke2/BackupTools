#include "FileHandler.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>
#include <numeric>
#include <stack>
#include <stdexcept>

char FileHandler::pathSeparator = fs::path::preferred_separator;
bool FileHandler::globMatchesHiddenFiles = true;

std::ostream& operator<<(std::ostream& out, CSI csiCode) {
    return out << '\033' << '[' << static_cast<int>(csiCode) << 'm';
}

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
bool fnmatchSimple(char const* pattern, char const* str, bool matchAllPaths = false) {
    if (matchAllPaths) {
        return (FileHandler::globMatchesHiddenFiles || *str != '.');
    }
    
    // Star does not match a leading dot in a name (because it's not supposed to match hidden files or the . and .. directories). Question mark does not match a leading dot in a name.
    if (!FileHandler::globMatchesHiddenFiles && ((*pattern == '*' && *str == '.') || (*pattern == '?' && *str == '.'))) {
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
    
    sourceFile.seekg(0);    // Return to beginning of files.
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

std::string parseNextWord_(std::string::size_type& index, const std::string& str) {
    std::string::size_type start = index;
    while (index < str.length()) {
        if (str[index] == ' ') {
            return str.substr(start, index - start);
        }
        ++index;
    }
    return str.substr(start);
}

std::string FileHandler::parseNextWord(std::string::size_type& index, const std::string& str) {
    std::string s = parseNextWord_(index, str);
    skipWhitespace(index, str);
    return s;
}

/**
 * Note: Double quotes can be used to encase a path and include whitespace. The
 * quotes must appear as the first and last characters in the path, the path
 * must also not contain any double quotes. It is theoretically possible for a
 * path name to contain double quotes, so a ? character could be used to match
 * it for implementations that support globbing.
 * 
 * This behavior is different than how splitArguments() in main.cpp handles
 * quotes. One of the reasons for this is that a path ending with a backslash
 * (on a Windows implementation) should recognize an ending double quote and
 * not escape it.
 */
fs::path parseNextPath_(std::string::size_type& index, const std::string& str) {
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
        return fs::path(str.substr(start + 1)).lexically_normal();
    }
    while (index < str.length()) {
        if (str[index] == ' ') {
            return fs::path(str.substr(start, index - start)).lexically_normal();
        }
        ++index;
    }
    return fs::path(str.substr(start)).lexically_normal();
}

fs::path FileHandler::parseNextPath(std::string::size_type& index, const std::string& str) {
    fs::path p = parseNextPath_(index, str);
    skipWhitespace(index, str);
    return p;
}

int parseNextInt_(std::string::size_type& index, const std::string& str) {
    std::string::size_type start = index;
    while (index < str.length()) {
        if (str[index] == ' ') {
            return stoi(str.substr(start, index - start));
        }
        ++index;
    }
    return stoi(str.substr(start));
}

int FileHandler::parseNextInt(std::string::size_type& index, const std::string& str) {
    int i = parseNextInt_(index, str);
    skipWhitespace(index, str);
    return i;
}

bool parseNextBool_(std::string::size_type& index, const std::string& str) {
    std::string nextWord = "";
    std::string::size_type start = index;
    while (index < str.length() && str[index] != ' ') {
        nextWord.push_back(std::tolower(static_cast<unsigned char>(str[index])));
        ++index;
    }
    if (nextWord == "false" || nextWord == "f" || nextWord == "0" || nextWord == "no" || nextWord == "n") {
        return false;
    } else if (nextWord == "true" || nextWord == "t" || nextWord == "1" || nextWord == "yes" || nextWord == "y") {
        return true;
    } else {
        throw std::runtime_error("Cannot convert argument to bool.");
    }
}

bool FileHandler::parseNextBool(std::string::size_type& index, const std::string& str) {
    bool b = parseNextBool_(index, str);
    skipWhitespace(index, str);
    return b;
}

void FileHandler::loadConfigFile(const fs::path& filename) {
    if (configFile_.is_open()) {
        configFile_.close();
    }
    configFile_.open(filename);
    if (!configFile_.is_open()) {
        throw std::runtime_error("\"" + filename.string() + "\": Unable to open file for reading.");
    }
    
    globMatchesHiddenFiles = true;
    
    configFilename_ = filename;
    lineNumber_ = 0;
    rootPaths_.clear();
    ignorePaths_.clear();
    previousReadPaths_.clear();
    globPortableResults_.clear();
    globPortableResultsIndex_ = 0;
    writePath_.clear();
    readPath_.clear();
    writePathSet_ = false;
    readPathSet_ = false;
}

WriteReadPathTree FileHandler::nextWriteReadPathTree() {
    WriteReadPathTree result;
    while (configFile_.is_open()) {
        parseNextLineInFile();
        
        if (readPathSet_) {    // If read path encountered, grab more results from globPortable().
            std::pair<fs::path, std::vector<fs::path>> globPortableResults = globPortable(readPath_);
            result.writePrefix = writePath_;
            result.readPrefix = globPortableResults.first;
            
            for (const auto& p : globPortableResults.second) {    // The results from globPortable() are just the matching items, loop through and ensure each item includes its parent paths.
                result.relativePaths.insert(p);
                
                std::string::size_type lastSeparator = p.string().rfind(pathSeparator);
                fs::path parentPath(p.string().substr(0, lastSeparator));
                if (result.relativePaths.insert(parentPath).second) {    // If adding parent path succeeded, step through each sub-path and make sure they are added.
                    while (true) {
                        lastSeparator = p.string().rfind(pathSeparator, lastSeparator - 1);
                        if (lastSeparator == std::string::npos || lastSeparator == 0) {
                            break;
                        }
                        
                        fs::path parentPath2(p.string().substr(0, lastSeparator));
                        if (!result.relativePaths.insert(parentPath2).second) {    // If insertion fails, all of the parents have been accounted for.
                            break;
                        }
                    }
                }
            }
            
            readPathSet_ = false;
            
            return result;
        }
    }
    
    return result;    // End of file reached, return empty result.
}

/** Globbing details:
    If last directory name does not contain wildcards, the directory is matched recursively (and all files stem from the directory name).
    * will match any one file or directory name.
    ** will match everything (matches zero to n directories and files). Similar to just putting the directory name, but files do not stem from the name.
        For simplicity, ** only works when by itself in a sub-path.
    For file/directory specific matching, the rules in fnmatchPortable() apply.
*/
std::pair<fs::path, std::vector<fs::path>> FileHandler::globPortable(fs::path pattern) {
    std::pair<fs::path, std::vector<fs::path>> result;
    
    if (!pattern.empty() && pattern.filename().empty() && pattern != pattern.root_path()) {    // If pattern includes a trailing separator, remove it (except if it is a root path).
        pattern = pattern.string().substr(0, pattern.string().length() - 1);
    }
    if (pattern.is_relative()) {    // If pattern is relative, make it absolute.
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
    if (pattern.has_root_directory()) {    // Skip root directory.
        ++patternIter;
        if (patternIter == pattern.end()) {
            return result;
        }
    }
    
    fs::path directoryPrefix = pattern.root_path();
    auto patternIterAhead = patternIter;
    ++patternIterAhead;
    
    while (patternIterAhead != pattern.end() && !containsWildcard(patternIter->string().c_str())) {    // Determine the directoryPrefix (the longest path in the pattern without wildcards).
        fs::file_status s = fs::status(directoryPrefix / *patternIter);
        if (!fs::exists(s)) {    // Confirm the path does indeed exist and is not a file (otherwise attempt to iterate the file would fail).
            return result;
        } else if (fs::is_regular_file(s) || (addedTrailingGlobstar && containsWildcard(patternIterAhead->string().c_str()))) {    // If a globstar was appended, stop before the last directory so that it will be included in write paths.
            break;
        }
        directoryPrefix /= *patternIter;    // The directoryPrefix stops before the last path, ensuring that patternIter points to a non-empty path.
        patternIter = patternIterAhead;
        ++patternIterAhead;
    }
    std::string::size_type dirPrefixOffset = directoryPrefix.string().length();    // dirPrefixOffset is the index to trim off the directoryPrefix. If directoryPrefix does not end with a slash, the slash is skipped when taking the substring.
    if (directoryPrefix.has_filename()) {
        ++dirPrefixOffset;
    }
    result.first = directoryPrefix;
    
    std::stack<fs::path> pathStack;    // Stacks for recursive directory matching process.
    pathStack.push(directoryPrefix);
    std::stack<fs::path::iterator> iterStack;
    iterStack.push(patternIter);
    std::stack<std::vector<fs::path::iterator>> ignoreIterStack;
    ignoreIterStack.push({});
    
    std::vector<fs::path> ignorePathsCopy;    // Make a copy of ignorePaths_ but add a globstar to the front of local paths.
    ignorePathsCopy.reserve(ignorePaths_.size());
    for (const auto& p : ignorePaths_) {
        if (p.is_relative()) {
            ignorePathsCopy.push_back("**" / p);
        } else {
            ignorePathsCopy.push_back(p);
        }
        ignoreIterStack.top().push_back(ignorePathsCopy.back().begin());    // Add iterator to ignoreIterStack.
    }
    
    for (const auto& p : directoryPrefix) {    // Step through directoryPrefix to determine if an ignore matches it.
        for (size_t i = 0; i < ignoreIterStack.top().size(); ++i) {
            if (checkSubPathIgnored(ignorePathsCopy[i], ignoreIterStack.top()[i], p)) {
                return result;
            }
        }
    }
    
    while (!pathStack.empty()) {    // Recursive operation to iterate through matching directories.
        fs::path pathTraversal = pathStack.top();    // The matched path thus far.
        pathStack.pop();
        fs::path::iterator currentPatternIter = iterStack.top();    // Iterator to the current sub-path.
        iterStack.pop();
        std::vector<fs::path::iterator> ignoreIters = ignoreIterStack.top();    // Iterators to the current positions in ignorePathsCopy.
        ignoreIterStack.pop();
        
        if (currentPatternIter == pattern.end()) {
            continue;
        }
        
        //std::cout << "Current pathTraversal is " << pathTraversal << "\n";
        //std::cout << "Current sub-pattern is " << *currentPatternIter << "\n";
        fs::path::iterator nextPatternIter = std::next(currentPatternIter);
        bool matchAllPaths = false;
        bool addToResult = (nextPatternIter == pattern.end());    // Only add to result if at the end, otherwise the path may not match the full pattern and we don't want it.
        if (addedTrailingGlobstar && *nextPatternIter == fs::path("**") && std::next(nextPatternIter) == pattern.end()) {    // Special case if globstar appended and pattern points to a file.
            addToResult = true;
        }
        
        if (*currentPatternIter == fs::path("**")) {    // If this sub-pattern is a globstar, match current path with the next sub-pattern and all contained directories with the current sub-pattern.
            pathStack.push(pathTraversal);
            iterStack.push(nextPatternIter);
            ignoreIterStack.push(ignoreIters);
            
            matchAllPaths = true;
            nextPatternIter = currentPatternIter;
        }
        
        try {
            for (const auto& entry : fs::directory_iterator(pathTraversal)) {
                if (fnmatchSimple(currentPatternIter->string().c_str(), entry.path().filename().string().c_str(), matchAllPaths)) {
                    bool includeThisPath = true;    // Check if path (and derived ones) can be ignored.
                    std::vector<fs::path::iterator> ignoreItersNext = ignoreIters;
                    for (size_t i = 0; i < ignoreItersNext.size(); ++i) {
                        if (checkSubPathIgnored(ignorePathsCopy[i], ignoreItersNext[i], entry.path().filename())) {
                            includeThisPath = false;
                            break;
                        }
                    }
                    
                    if (includeThisPath) {
                        //std::cout << "Matched " << entry.path() << "\n";
                        if (addToResult) {
                            fs::path nextPathTraversal = pathTraversal / entry.path().filename();
                            if (previousReadPaths_.insert(nextPathTraversal).second) {    // Add to result if this read path is unique.
                                result.second.emplace_back(nextPathTraversal.string().substr(dirPrefixOffset));
                            }
                        }
                        if (entry.is_directory()) {
                            pathStack.push(pathTraversal / entry.path().filename());
                            iterStack.push(nextPatternIter);
                            ignoreIterStack.push(std::move(ignoreItersNext));
                            //std::cout << "    " << pathStack.top() << "\n";
                        }
                    }
                }
            }
        } catch (fs::filesystem_error& ex) {    // Exception accessing path can be ignored (treat it like an empty directory).
            std::cout << CSI::Red << "Error: " << ex.code().message() << ": \"" << ex.path1().string() << "\"";
            if (!ex.path2().empty()) {
                std::cout << ", \"" << ex.path2().string() << "\"";
            }
            std::cout << CSI::Reset << "\n";
        } catch (std::exception& ex) {
            std::cout << CSI::Red << "Error: " << ex.what() << CSI::Reset << "\n";
        }
        
        //std::cout << "========================================\n";
    }
    
    return result;
}

bool FileHandler::checkPathIgnored(const fs::path& p) const {
    for (fs::path ignorePath : ignorePaths_) {    // Step through ignores and path p to determine if there is a match.
        if (ignorePath.is_relative()) {    // Append a globstar to local paths.
            ignorePath = "**" / ignorePath;
        }
        fs::path::iterator ignorePathIter = ignorePath.begin();
        for (const auto& subPath : p) {
            if (checkSubPathIgnored(ignorePath, ignorePathIter, subPath)) {
                return true;
            }
        }
    }
    return false;
}

bool FileHandler::checkSubPathIgnored(const fs::path& ignorePath, fs::path::iterator& ignoreIter, const fs::path& currentSubPath) {
    if (ignoreIter == ignorePath.end()) {    // End iterator means match failed previously.
        return false;
    }
    while (ignoreIter != ignorePath.end() && *ignoreIter == fs::path("**")) {    // Skip any globstars.
        ++ignoreIter;
    }
    if (ignoreIter == ignorePath.end() || ignoreIter->empty()) {    // Globstar matched the path. Note that an ignore path that ends with a directory separator also ends with an empty path.
        return true;
    } else if (fnmatchSimple(ignoreIter->string().c_str(), currentSubPath.string().c_str())) {    // Else if sub-path matched, ignore succeeds if it's at the end.
        ++ignoreIter;
        return ignoreIter == ignorePath.end() || ignoreIter->empty();    // Note: If a globstar still remains, then we don't ignore the current sub-path. This matches the behavior of path matching in globPortable().
    }
    
    while (ignoreIter != ignorePath.begin() && *ignoreIter != fs::path("**")) {    // Sub-path not matched, rewind to find globstar.
        --ignoreIter;
    }
    if (*ignoreIter != fs::path("**")) {    // No globstar, the match failed indefinitely.
        ignoreIter = ignorePath.end();
    }
    return false;
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
        if (command == "set") {    // Syntax: set <option> <value>
            if (index >= line.length()) {
                throw std::runtime_error("Missing option parameter.");
            }
            std::string option = parseNextWord(index, line);
            if (option == "match-hidden") {    // Controls matching of hidden files when using wildcard matching.
                if (index >= line.length()) {
                    throw std::runtime_error("Missing value for \"" + option + "\".");
                }
                globMatchesHiddenFiles = parseNextBool(index, line);
            } else {
                throw std::runtime_error("Invalid option \"" + option + "\".");
            }
        } else if (command == "root") {    // Syntax: root <identifier> <replacement path>
            if (index >= line.length()) {
                throw std::runtime_error("Missing identifier path parameter.");
            }
            fs::path keyPath = parseNextPath(index, line);
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
            if (index < line.length()) {
                command = parseNextWord(index, line);
                if (command != "add") {
                    throw std::runtime_error("Unexpected command \"" + command + "\" after \"in <write path>\".");
                }
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
            } else {
                throw std::runtime_error("No matching ignore path found for \"" + includePath.string() + "\".");
            }
        } else {
            throw std::runtime_error("Unknown command \"" + command + "\".");
        }
        if (index < line.length()) {
            throw std::runtime_error("Unexpected data after command: \"" + line.substr(index) + "\".");
        }
    } catch (std::exception& ex) {
        throw std::runtime_error("\"" + configFilename_.string() + "\" at line " + std::to_string(lineNumber_) + ": " + ex.what());
    }
}
