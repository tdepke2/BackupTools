#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "FileHandler.h"
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class Application {
    public:
    static bool checkUserConfirmation();    // Gets user input and returns true only if any variant of "yes" was entered.
    void printPaths(const fs::path& configFilename);
    std::vector<fs::path> checkBackup(const fs::path& configFilename, bool displayConfirmation = false);
    void startBackup(const fs::path& configFilename, bool forceBackup);
    void restoreFromBackup(const fs::path& configFilename);
    
    private:
    FileHandler fileHandler_;
};

#endif
