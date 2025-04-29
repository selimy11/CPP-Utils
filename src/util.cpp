#include "util.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <openssl/sha.h>

void textWriter(const std::string &filePath, const std::string &text) {
    std::ofstream MyFile(filePath);
    if (MyFile.is_open()) {
        MyFile << text << "\n";
        MyFile.close();
    } else {
        std::cerr << "Failed to open file for writing: " << filePath << std::endl;
    }
}

void textReader(const std::string &filePath, std::string &result) {
    std::ifstream ifs(filePath);
    if (ifs.good()) {
        result.assign((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));
        ifs.close();
    } else {
        std::cerr << "There is no file to read: " << filePath << std::endl;
        result = "";
        ifs.close();
    }
}

BSTR stringToBstr(const std::string &str) {
    int length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    BSTR bstr = SysAllocStringLen(nullptr, length - 1);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, bstr, length);
    return bstr;
}

void unzipFile(const std::string &source, const std::string &destination, int &result) {
    BSTR lpZipFile = stringToBstr(source);
    BSTR lpFolder = stringToBstr(destination);
    IShellDispatch *pISD;
    Folder *pZippedFile = nullptr;
    Folder *pDestination = nullptr;
    FolderItems *pFilesInside = nullptr;
    IDispatch *pItem = nullptr;

    VARIANT Options, OutFolder, InZipFile, Item;
    CoInitialize(NULL);
    __try {
        if (CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void**)&pISD) != S_OK) {
            result = 1;
            return;
        }

        InZipFile.vt = VT_BSTR;
        InZipFile.bstrVal = lpZipFile;
        pISD->NameSpace(InZipFile, &pZippedFile);
        if (!pZippedFile) {
            pISD->Release();
            result = 1;
            return;
        }

        OutFolder.vt = VT_BSTR;
        OutFolder.bstrVal = lpFolder;
        pISD->NameSpace(OutFolder, &pDestination);
        if (!pDestination) {
            pZippedFile->Release();
            pISD->Release();
            result = 1;
            return;
        }

        pZippedFile->Items(&pFilesInside);
        if (!pFilesInside) {
            pDestination->Release();
            pZippedFile->Release();
            pISD->Release();
            result = 1;
            return;
        }

        pFilesInside->QueryInterface(IID_IDispatch, (void**)&pItem);
        Item.vt = VT_DISPATCH;
        Item.pdispVal = pItem;

        Options.vt = VT_I4;
        Options.lVal = 0;

        bool retval = pDestination->CopyHere(Item, Options) == S_OK;
        pItem->Release();
        pFilesInside->Release();
        pDestination->Release();
        pZippedFile->Release();
        pISD->Release();
    }
    __finally {
        CoUninitialize();
    }
}

void removeFilesAndFolders(const std::filesystem::path &directory) {
    try {
        for (const auto &entry : std::filesystem::directory_iterator(directory)) {
            if (std::filesystem::is_directory(entry)) {
                removeFilesAndFolders(entry.path());
            } else {
                std::filesystem::remove(entry.path());
            }
        }
        std::filesystem::remove(directory);
    } catch (const std::exception &e) {
        std::cerr << "Error while removing files and folders: " << e.what() << std::endl;
    }
}

std::string getChecksum(std::string filePath) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256Context;

    FILE *file = fopen(filePath.c_str(), "rb");
    if (!file) {
        return "";
    }

    SHA256_Init(&sha256Context);
    const size_t bufferSize = 8192;
    char buffer[bufferSize];

    while (size_t bytesRead = fread(buffer, 1, bufferSize, file)) {
        SHA256_Update(&sha256Context, buffer, bytesRead);
    }

    SHA256_Final(hash, &sha256Context);
    fclose(file);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }

    std::string result = ss.str();
    for (char &c : result) {
        c = std::toupper(c);
    }
    return result;
}

int copyFolder(const std::filesystem::path &source, const std::filesystem::path &destination) {
    try {
        std::filesystem::create_directories(destination);
        for (const auto &entry : std::filesystem::recursive_directory_iterator(source)) {
            const auto &path = entry.path();
            auto relativePath = std::filesystem::relative(path, source);
            auto destPath = destination / relativePath;

            if (std::filesystem::is_directory(path)) {
                std::filesystem::create_directories(destPath);
            } else if (std::filesystem::is_regular_file(path)) {
                std::filesystem::copy_file(path, destPath, std::filesystem::copy_options::overwrite_existing);
            }
        }
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error while copying folder: " << e.what() << std::endl;
        return -1;
    }
}

std::vector<std::filesystem::path> findAllExtensionFiles(const std::filesystem::path &directory, const std::string extension) {
    std::vector<std::filesystem::path> files;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == extension) {
            files.push_back(entry.path());
        }
    }
    return files;
}

std::optional<std::filesystem::path> findFile(const std::filesystem::path &directory, const std::string &filename) {
    for (const auto &entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().filename() == filename) {
            return entry.path();
        }
    }
    return std::nullopt;
}

std::string tcharToString(const TCHAR *tcharString) {
#ifdef _UNICODE
    int length = WideCharToMultiByte(CP_UTF8, 0, tcharString, -1, nullptr, 0, nullptr, nullptr);
    std::string result(length, 0);
    WideCharToMultiByte(CP_UTF8, 0, tcharString, -1, &result[0], length, nullptr, nullptr);
#else
    std::string result(tcharString);
#endif
    return result;
}

std::string getDateFromDateTimeStr(const std::string &dateTimeString) {
    if (dateTimeString.size() < 14) {
        std::cerr << "Invalid date time string: " << dateTimeString << std::endl;
        return "";
    }
    std::string year = dateTimeString.substr(0, 4);
    std::string month = dateTimeString.substr(4, 2);
    std::string day = dateTimeString.substr(6, 2);

    return month + "/" + day + "/" + year;
}

std::string getDateFromISODate(const std::string &isoDate) {
    std::tm tm = {};
    std::stringstream ss(isoDate);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) {
        return "";
    }

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << tm.tm_mday << "."
        << std::setw(2) << std::setfill('0') << (tm.tm_mon + 1) << "."
        << (tm.tm_year + 1900);
    return oss.str();
}
