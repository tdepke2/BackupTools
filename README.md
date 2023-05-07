Work in progress tools for local file backups. Planned features are manual/automatic backups specified from a file, file diff checking, and support for remote drives if possible.

Implementation details:

    * Wildcards cannot be used in the path specified with "in" command (the write location must be singular).
    * A globstar in a path uses the "\*\*" representation and must be separated from other fields with directory separators. For example use "files/\*\*" instead of "files\*\*".
    * If the last entry in a path contains wildcards, it will not match any children paths. Append a globstar if this is desired instead.

Development environment setup:

The following is for Windows development using Sublime Text, but most of it still applies to a Linux/Mac setup.

    1. Prerequisites: [Git](https://git-scm.com/downloads), [CMake](https://cmake.org/download/), [LLVM](https://releases.llvm.org/), and a compiler to use with clang, like MSVC provided with VisualStudio (or use MinGW and build with clang++). For Sublime, install [LSP](https://lsp.sublimetext.io/) and [LSP-clangd](https://github.com/sublimelsp/LSP-clangd) from Package Control.
    2. Clone this repo `git clone https://github.com/tdepke2/BackupTools.git && cd BackupTools`.
    3. Generate "compile_commands.json" for clangd with `CC=clang CXX=clang++ cmake -S . -B build_clang -G "Unix Makefiles" && cp build_clang/compile_commands.json .`. Open up the project folder with Sublime and LSP-clangd should be able to find all of the sources and compile flags.
    4. To start a build with VisualStudio, run `cmake -S . -B build` and `cmake --build build` (assuming VisualStudio is the default generator in CMake). The resulting binaries can be found in the build/ directory.
