#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <comdef.h>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
  #ifdef CPPUTILS_EXPORTS
    #define CPPUTILS_API __declspec(dllexport)
  #else
    #define CPPUTILS_API __declspec(dllimport)
  #endif
#else
  #define CPPUTILS_API
#endif

// File operations
CPPUTILS_API void textWriter(const std::string &filePath, const std::string &text);
CPPUTILS_API void textReader(const std::string &filePath, std::string &result);

// String and BSTR utilities
CPPUTILS_API BSTR stringToBstr(const std::string &str);
CPPUTILS_API std::string tcharToString(const TCHAR *tcharString);

// File system utilities
CPPUTILS_API void unzipFile(const std::string &source, const std::string &destination, int &result);
CPPUTILS_API void removeFilesAndFolders(const std::filesystem::path &directory);
CPPUTILS_API int copyFolder(const std::filesystem::path &source, const std::filesystem::path &destination);
CPPUTILS_API std::vector<std::filesystem::path> findAllExtensionFiles(const std::filesystem::path &directory, const std::string extension);
CPPUTILS_API std::optional<std::filesystem::path> findFile(const std::filesystem::path &directory, const std::string &filename);

// Hash and Date utilities
CPPUTILS_API std::string getChecksum(std::string filePath);
CPPUTILS_API std::string getDateFromDateTimeStr(const std::string &dateTimeString);
CPPUTILS_API std::string getDateFromISODate(const std::string &isoDate);
