#include "Application.h"
#include <iostream>
#include <string>


#include <filesystem>
#include <cctype>

// Quick desc:
// CLI backup tool ideal for backing up files to a flash drive. Target filename to include/exclude are stored in a config file.
// Supports diff checking, manual backup, and automatic backups. Ideally should work on linux too, and use CMake and TDD.

bool fnmatchPortable(const std::string& pattern, const std::string& str, std::string::size_type pIndex = 0, std::string::size_type sIndex = 0);
bool fnmatchPortable2(char const* pattern, char const* str);

int main(int argc, char** argv) {
    Application app;
    bool result = app.checkFileEquivalence("sandbox/tmp.txt", "sandbox/a.txt");
    std::cout << "app.checkFileEquivalence = " << result << "\n";
    
    app.loadFile("sample.txt");
    app.printPaths();
    
    std::string strFile = "Hello";
    std::string strSpec;
    while (true) {
        getline(std::cin, strSpec);
        std::cout << "Match with [" << strFile << "]: " << fnmatchPortable2(strSpec.c_str(), strFile.c_str()) << "\n";
    }
    
    return 0;
}

char pathSeparator = std::filesystem::path::preferred_separator;

bool fnmatchPortable(const std::string& pattern, const std::string& str, std::string::size_type pIndex, std::string::size_type sIndex) {
    while (pIndex < pattern.length() && sIndex < str.length()) {
        if (isalnum(pattern[pIndex])) {
            if (pattern[pIndex] == str[sIndex]) {
                ++pIndex;
                ++sIndex;
            } else {
                return false;
            }
        } else if (pattern[pIndex] == '*') {
            ++pIndex;
            for (std::string::size_type sOffset = 0; sOffset <= str.length() - sIndex; ++sOffset) {
                if (fnmatchPortable(pattern, str, pIndex, sIndex + sOffset)) {
                    return true;
                }
            }
            
            return false;
        } else if (pattern[pIndex] == '?') {
            if (str[sIndex] == '.' || str[sIndex] == pathSeparator) {
                return false;
            }
            ++pIndex;
            ++sIndex;
        } else if (pattern[pIndex] == '[') {
            
        } else {
            if (pattern[pIndex] == str[sIndex]) {
                ++pIndex;
                ++sIndex;
            } else {
                return false;
            }
        }
    }
    
    return pIndex == pattern.length() && sIndex == str.length();
}

bool fnmatchPortable2(char const* pattern, char const* str) {
    while (*str != '\0') {
        if (*pattern == '*') {
            do {
                ++pattern;
            } while (*pattern == '*');
            if (*pattern == '\0') {    // Not gonna work, need to exclude path sep #######################
                return true;
            }
            
            while (*str != '\0') {
                if (fnmatchPortable2(pattern, str)) {
                    return true;
                }
                ++str;
            }
            
            return false;
        } else if (*pattern == '?') {
            if (*str == '.' || *str == pathSeparator) {
                return false;
            }
            ++pattern;
            ++str;
        } else if (*pattern == '[') {
            
        } else if (*pattern == *str) {
            ++pattern;
            ++str;
        } else {
            return false;
        }
    }
    
    while (*pattern == '*') {
        ++pattern;
    }
    return *pattern == '\0';
}

/*
Alternative just in case https://stackoverflow.com/questions/35877738/windows-fnmatch-substitute
Similar impl https://stackoverflow.com/questions/3300419/file-name-matching-with-wildcard

Glob mechanics: https://www.man7.org/linux/man-pages/man7/glob.7.html
Wildcards: * ? [abc] [a-z] [!abc] [!a-z]
? does not match .
* matches all except for .files, but these can be matched with .*
Path separator / or \ never matched

*e*lo
H*
*
??*?
Hello


*/
