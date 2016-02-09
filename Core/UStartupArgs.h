#ifndef UStartupArgs_H
#define UStartupArgs_H

#include <iostream>
#include <string>
#include <vector>
#include <map>


class UStartupArgs
{
    bool m_isValid;
public:
    UStartupArgs(int argc, const char* argv[]);
    UStartupArgs(const UStartupArgs& other) = default;
    ~UStartupArgs() = default;

    inline bool IsValid() { return m_isValid; } //< Checks if the inputs are all valid

    std::string InputFile;
    std::string OutputPath;
    std::vector<std::string> IncludePaths;
    std::string LibraryName;
    std::string ResourcePath;
    bool VB6_IgnoreUnsigned;
    bool VB6_PtrToLong;
    std::string Language;
};

#endif


