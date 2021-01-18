#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <filesystem>
#include <fstream>
#include <map>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

class Application {
    public:
    bool checkFileEquivalence(const fs::path& source, const fs::path& dest) const;
    void loadFile(const fs::path& filename);
    void printPaths();
    
    private:
    std::ifstream configFile_;
    std::map<fs::path, fs::path> rootPaths_;
    std::vector<fs::path> ignorePaths_;
    
    static void skipWhitespace(std::string::size_type& index, const std::string& str);    // Increment index while space character found in str.
    static std::string parseNextCommand(std::string::size_type& index, const std::string& str);    // Return next string in str until space found.
    static fs::path parseNextPath(std::string::size_type& index, const std::string& str);    // Return next path (considers paths wrapped in double quotes) and normalize it.
    fs::path substituteRootPath(const fs::path& path);    // Substitute the path root for a match in rootPaths_ if applicable.
    bool hasNextPath() const;    // FIXME: refactor these two ####################################################################################
    std::pair<fs::path, fs::path> getNextPath();
};

#endif
