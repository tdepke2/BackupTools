#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "FileHandler.h"
#include <filesystem>

namespace fs = std::filesystem;

class Application {
    public:
    void printPaths(const fs::path& filename);
    
    private:
    FileHandler fileHandler_;
};

#endif
