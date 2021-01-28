#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "FileHandler.h"
#include <filesystem>

namespace fs = std::filesystem;

class Application {
    public:
    void printPaths(const fs::path& configFilename);
    void checkBackup(const fs::path& configFilename);
    void startBackup(const fs::path& configFilename);
    void restoreFromBackup(const fs::path& configFilename);
    
    private:
    FileHandler fileHandler_;
};

#endif
