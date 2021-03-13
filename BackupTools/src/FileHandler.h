#ifndef FILE_HANDLER_H_
#define FILE_HANDLER_H_

#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

enum CSI : int {    // Control Sequence Introducer used to set colors and formatting in terminal.
    Reset = 0,      // https://en.wikipedia.org/wiki/ANSI_escape_code#CSI_sequences
    Bold = 1,
    Underline = 4,
    Inverse = 7,
    Black = 30,
    Red = 31,
    Green = 32,
    Yellow = 33,
    Blue = 34,
    Magenta = 35,
    Cyan = 36,
    White = 37,
    BlackBG = 40,
    RedBG = 41,
    GreenBG = 42,
    YellowBG = 43,
    BlueBG = 44,
    MagentaBG = 45,
    CyanBG = 46,
    WhiteBG = 47
};
std::ostream& operator<<(std::ostream& out, CSI csiCode);

struct WriteReadPath {
    fs::path writePath;
    fs::path readAbsolute;
    fs::path readLocal;
    
    bool isEmpty() const { return readAbsolute.empty(); }
};

bool compareFilename(const fs::path& lhs, const fs::path& rhs);    // Comparator to sort filenames (case is ignored).

class FileHandler {
    public:
    static char pathSeparator;
    static bool globMatchesHiddenFiles;
    
    static bool fnmatchPortable(char const* pattern, char const* str);    // Implementation of the unix fnmatch(3) function. More details in .cpp file.
    static bool containsWildcard(char const* pattern);    // Determines if a string contains glob wildcards.
    static bool checkFileEquivalence(const fs::path& source, const fs::path& dest);    // Returns true if files are identical, false otherwise. Works with directories as well.
    static void skipWhitespace(std::string::size_type& index, const std::string& str);    // Increment index while space character found in str.
    static std::string parseNextWord(std::string::size_type& index, const std::string& str);    // Return next string in str until space found.
    static fs::path parseNextPath(std::string::size_type& index, const std::string& str);    // Return next path (considers paths wrapped in double quotes) and normalize it.
    static int parseNextInt(std::string::size_type& index, const std::string& str);    // Return next integer in str.
    static bool parseNextBool(std::string::size_type& index, const std::string& str);    // Return next bool in str.
    
    void loadConfigFile(const fs::path& filename);
    WriteReadPath getNextWriteReadPath();    // Get the next write/read combination from configFile_, or return empty paths if none left. Returned paths are stripped of regex and read path is unique and not contained in ignorePaths_.
    std::vector<std::pair<fs::path, fs::path>> globPortable(fs::path pattern) const;    // Returns a list of absolute/local paths that match the pattern. The pattern must be in normal form.
    bool checkPathIgnored(const fs::path& p) const;    // Iterates through all of ignorePaths_ and determines if there is a match. The complexity is O(N*M) so use carefully.
    
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
    
    static bool checkSubPathIgnored(const fs::path& ignorePath, fs::path::iterator& ignoreIter, const fs::path& currentSubPath);    // Determines if the current sub-path is ignored given the current position (ignoreIter) in ignorePath.
    fs::path substituteRootPath(const fs::path& path);    // Substitute the path root for a match in rootPaths_ if applicable.
    void parseNextLineInFile();    // Read next line in configFile_ (if no more lines, closes the file). Sets writePath_ and readPath_.
};

#endif
