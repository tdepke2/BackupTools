#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <filesystem>

namespace fs = std::filesystem;

class Application {
    public:
    bool checkFileEquivalence(const fs::path& source, const fs::path& dest) const;
};

#endif
