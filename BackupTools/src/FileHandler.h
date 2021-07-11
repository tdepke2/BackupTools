#ifndef FILE_HANDLER_H_
#define FILE_HANDLER_H_

#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

/**
 * Control Sequence Introducer used to set colors and formatting in terminal.
 * 
 * See: https://en.wikipedia.org/wiki/ANSI_escape_code#CSI_sequences
 */
enum CSI : int {
    Reset = 0,
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

/**
 * Stores info about a group of tracked files with the same write and read
 * prefix paths. The relativePaths should be laid out like a file tree (each
 * filename includes it's parents in the set) but this is not enforced by the
 * struct.
 */
struct WriteReadPathTree {
    fs::path writePrefix;
    fs::path readPrefix;
    std::set<fs::path> relativePaths;
    
    bool isEmpty() const { return relativePaths.empty(); }
};

/*
// Tree method for storing paths (instead of the sorted-set idea).
// May not want to use this in the end because of efficiency concerns, will leave it here if need to reconsider.

class PathTreeNode {
public:
    PathTreeNode* getParent();
    PathTreeNode* addChild(const fs::path& subpath);
    
private:
    PathTreeNode* parent_;
    std::vector<PathTreeNode*> children_;
    fs::path subpath_;
    
    PathTree();
};

class PathTree {
public:
    PathTree();
    PathTreeNode* getRoot();
    
private:
    PathTreeNode* root_;
};
*/

/**
 * Comparator to sort filenames (case is ignored).
 */
bool compareFilename(const fs::path& lhs, const fs::path& rhs);

/**
 * Utility class to deal with config file format, file globs (pattern matching),
 * and other file operations.
 */
class FileHandler {
public:
    static char pathSeparator;
    static bool globMatching;
    static bool globMatchesHiddenFiles;
    
    /**
     * Implementation of the unix fnmatch(3) function. More details in .cpp
     * file.
     */
    static bool fnmatchPortable(char const* pattern, char const* str);
    
    /**
     * Determines if a string contains glob wildcards.
     */
    static bool containsWildcard(char const* pattern);
    
    /**
     * Increment index while space character found in str. Automatically called
     * at end of other parsing functions (unless an exception occurs).
     */
    static void skipWhitespace(std::string::size_type& index, const std::string& str);
    
    /**
     * Return next string in str until space found.
     */
    static std::string parseNextWord(std::string::size_type& index, const std::string& str);
    
    /**
     * Return next path (considers paths wrapped in double quotes) and normalize
     * it.
     */
    static fs::path parseNextPath(std::string::size_type& index, const std::string& str);
    
    /**
     * Return next integer in str.
     */
    static int parseNextInt(std::string::size_type& index, const std::string& str);
    
    /**
     * Return next bool in str.
     */
    static bool parseNextBool(std::string::size_type& index, const std::string& str);
    
    /**
     * Returns true if files are identical, false otherwise. Works with
     * directories as well.
     * 
     * The skipCache parameter avoids reading/writing the cachedWriteTimes_ data
     * that gets initialized from loadCacheFile().
     * The fastCompare parameter skips the binary scan of each file and only
     * returns true if the file modification timestamps match (if the times are
     * within 2 seconds of each other to be exact, due to slightly different
     * time representations across digital storage mediums).
     */
    bool checkFileEquivalence(const fs::path& source, const fs::path& dest, bool skipCache = false, bool fastCompare = false);
    
    /**
     * Opens the file (closes the previous one if still open) and resets all
     * internal state.
     */
    void loadConfigFile(const fs::path& filename);
    
    /**
     * Parses a cache file and stores it in cachedWriteTimes_. The cache file
     * keeps track of each file that gets scanned by checkFileEquivalence() and
     * stores the last known modification timestamp of the source and
     * destination files, and whether the files were equivalent or not at that
     * time. The file format is filename (of source), null character, the byte
     * data of the corresponding CachedWriteTime, and a newline character.
     */
    void loadCacheFile(const fs::path& filename);
    
    /**
     * Creates a cache file using the format mentioned in loadCacheFile().
     */
    void saveCacheFile(const fs::path& filename);
    
    /**
     * Get the next set of write/read paths from configFile_, or return empty
     * result if none left. Returned paths are stripped of regex and read paths
     * (the absolute paths, not just relative ones that are returned) are unique
     * and not contained in ignorePaths_.
     * 
     * Unlike with globPortable(), the returned paths are guaranteed to include
     * all of their parents.
     */
    WriteReadPathTree nextWriteReadPathTree();
    
    /**
     * Returns a common path prefix (an absolute path), and list of relative
     * paths that match the pattern. The pattern must be in normal form.
     * 
     * Note that only the exact files/directories that match the pattern end up
     * in the returned list. Their parent paths are not guaranteed to exist in
     * the list.
     */
    std::pair<fs::path, std::vector<fs::path>> globPortable(fs::path pattern);
    
    /**
     * Iterates through all of ignorePaths_ and determines if there is a match.
     * The complexity is O(N*M) so use carefully.
     */
    bool checkPathIgnored(const fs::path& p) const;
    
private:
    /**
     * Stores last known modification timestamps and equivalence of a source and
     * destination file. Used for reducing the number of file scans required in
     * checkFileEquivalence().
     */
    struct CachedWriteTime {
        fs::file_time_type sourceTime;
        fs::file_time_type destTime;
        bool fileEquivalence;
    };
    
    std::ifstream configFile_;
    fs::path configFilename_;
    unsigned int lineNumber_;
    std::map<fs::path, fs::path> rootPaths_;
    std::set<fs::path> ignorePaths_;
    std::set<fs::path> previousReadPaths_;
    fs::path writePath_, readPath_;
    bool writePathSet_, readPathSet_;
    std::map<fs::path, CachedWriteTime> cachedWriteTimes_;    // May be a good idea to use a radix tree instead of std::map, but also good to stick with simplicity. ###############################################
    
    /**
     * Determines if the current sub-path is ignored given the current position
     * (ignoreIter) in ignorePath.
     */
    static bool checkSubPathIgnored(const fs::path& ignorePath, fs::path::iterator& ignoreIter, const fs::path& currentSubPath);
    
    /**
     * Substitute the path root for a match in rootPaths_ if applicable.
     */
    fs::path substituteRootPath(const fs::path& path);
    
    /**
     * Read next line in configFile_ (if no more lines, closes the file). Sets
     * writePath_ and readPath_.
     */
    void parseNextLineInFile();
};

#endif
