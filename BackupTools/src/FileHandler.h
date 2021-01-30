#ifndef FILE_HANDLER_H_
#define FILE_HANDLER_H_

#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

struct WriteReadPath {
    fs::path writePath;
    fs::path readAbsolute;
    fs::path readLocal;
    
    bool isEmpty() const { return readAbsolute.empty(); }
};

class FileHandler {
    public:
    static char pathSeparator;
    
    static bool fnmatchPortable(char const* pattern, char const* str);    // Implementation of the unix fnmatch(3) function. More details in .cpp file.
    static bool containsWildcard(char const* pattern);    // Determines if a string contains glob wildcards.
    static std::vector<std::pair<fs::path, fs::path>> globPortable(fs::path pattern);    // Returns a list of absolute/local paths that match the pattern. The pattern must be in normal form.
    static bool checkFileEquivalence(const fs::path& source, const fs::path& dest);    // Returns true if files are identical, false otherwise.
    static void skipWhitespace(std::string::size_type& index, const std::string& str);    // Increment index while space character found in str.
    static std::string parseNextWord(std::string::size_type& index, const std::string& str);    // Return next string in str until space found.
    static fs::path parseNextPath(std::string::size_type& index, const std::string& str);    // Return next path (considers paths wrapped in double quotes) and normalize it.
    
    void loadConfigFile(const fs::path& filename);
    WriteReadPath getNextWriteReadPath();    // Get the next write/read combination from configFile_, or return empty paths if none left. Returned paths are stripped of regex and read path is unique and not contained in ignorePaths_.
    
    private:
    std::ifstream configFile_;
    fs::path configFilename_;
    unsigned int lineNumber_;
    std::map<fs::path, fs::path> rootPaths_;
    std::set<fs::path> ignorePaths_;
    std::set<fs::path> previousReadPaths_;
    std::vector<std::pair<fs::path, fs::path>> globPortableResults_;
    size_t globPortableResultsIndex_;
    fs::path writePath_, readPath_;
    bool writePathSet_, readPathSet_;
    
    fs::path substituteRootPath(const fs::path& path);    // Substitute the path root for a match in rootPaths_ if applicable.
    void parseNextLineInFile();    // Read next line in configFile_ (if no more lines, closes the file). Sets writePath_ and readPath_.
};

#endif
