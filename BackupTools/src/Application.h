#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "FileHandler.h"
#include <filesystem>
#include <string>
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

bool compareFileChange(const std::pair<fs::path, fs::path>& lhs, const std::pair<fs::path, fs::path>& rhs);

class Application {
    public:
    struct FileChanges {
        std::set<fs::path, decltype(&compareFilename)> deletions;
        std::set<std::pair<fs::path, fs::path>, decltype(&compareFileChange)> additions;
        std::set<std::pair<fs::path, fs::path>, decltype(&compareFileChange)> modifications;
        
        FileChanges() : deletions(&compareFilename), additions(&compareFileChange), modifications(&compareFileChange) {}
        bool isEmpty() const { return deletions.empty() && additions.empty() && modifications.empty(); }
    };
    
    static bool checkUserConfirmation();    // Gets user input and returns true only if any variant of "yes" was entered.
    void printPaths(const fs::path& configFilename);
    FileChanges checkBackup(const fs::path& configFilename, bool displayConfirmation = false);
    void startBackup(const fs::path& configFilename, bool forceBackup);
    void restoreFromBackup(const fs::path& configFilename);
    
    private:
    FileHandler fileHandler_;
    
    void printTree(const fs::path& searchPath, const std::map<fs::path, fs::path>& readPathsMapping);
    void printTree2(const fs::path& searchPath, const std::map<fs::path, fs::path>& readPathsMapping, const std::string& prefix, unsigned int* numDirectories, unsigned int* numFiles);
};

#endif
