#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Events/EventSystem.hpp"
#include <windows.h>
#include <strsafe.h>
#include <fstream>
#include <deque>

//-----------------------------------------------------------------------------------
bool LoadBufferFromBinaryFile(std::vector<unsigned char>& out_buffer, const std::string& filePath)
{
    FILE* file = nullptr;
    errno_t errorCode = fopen_s(&file, filePath.c_str(), "rb"); //returns a failure state. If failure, return false.
    if(errorCode != 0x0) 
    { 
        return false; 
    };
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    out_buffer.resize(size);
    fread(&out_buffer[0], sizeof(unsigned char), size, file);
    fclose(file);
    return true;
}

//-----------------------------------------------------------------------------------
bool SaveBufferToBinaryFile(const std::vector<unsigned char>& buffer, const std::string& filePath)
{
    FILE* file = nullptr;
    errno_t errorCode = fopen_s(&file, filePath.c_str(), "wb"); //returns a failure state. If failure, return false.
    if (errorCode != 0x0)
    {
        return false;
    };
    fwrite(&buffer[0], sizeof(unsigned char), buffer.size(), file);
    fclose(file);
    return true;
}

//-----------------------------------------------------------------------------------
bool EnsureDirectoryExists(const std::string& directoryPath)
{
    std::wstring wideDirectoryPath = std::wstring(directoryPath.begin(), directoryPath.end());
    LPCWSTR wideDirectoryPathCStr = wideDirectoryPath.c_str();
    bool success = (CreateDirectory(wideDirectoryPathCStr, NULL) == TRUE);
    int error = GetLastError();
    if (error == ERROR_ALREADY_EXISTS)
    {
        success = true;
    }
    else if (error == ERROR_PATH_NOT_FOUND)
    {
        ERROR_RECOVERABLE("Attempted to ensure that a directory existed, but wasn't able to find the path specified.");
    }
    return success;
}

//-----------------------------------------------------------------------------------
bool EnsureFileExists(const std::string& fullFilePath)
{
    bool success = true;
    std::wstring wideFilePath = std::wstring(fullFilePath.begin(), fullFilePath.end());
    LPCWSTR wideFilePathCStr = wideFilePath.c_str();
    HANDLE outFileHandle = CreateFile(wideFilePathCStr, GENERIC_WRITE|GENERIC_READ, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (outFileHandle == INVALID_HANDLE_VALUE)
    {
        DWORD errorCode = GetLastError();
        if (errorCode == ERROR_FILE_EXISTS)
        {
            success = true;
        }
        else if (errorCode == ERROR_FILE_NOT_FOUND)
        {
            ERROR_RECOVERABLE("Attempted to ensure a file existed, but file was not found.");
            success = false;
        }
        else
        {
            ERROR_RECOVERABLE("Attempted to ensure a file existed, but failed. (Check the error code from GetLastError)");
            success = false;
        }
    }

    return success;
}

//-----------------------------------------------------------------------------------
bool ReadTextFileIntoVector(std::vector<std::string>& outBuffer, const std::string& filePath)
{
    std::ifstream file(filePath);
    std::string currentLine;
    while (std::getline(file, currentLine))
    {
        outBuffer.push_back(currentLine);
    }
    return true;
}

//-----------------------------------------------------------------------------------
char* FileReadIntoNewBuffer(const std::string& filePath)
{
    FILE* file = nullptr;
    errno_t errorCode = fopen_s(&file, filePath.c_str(), "rb"); //returns a failure state. If failure, return false.
    if (errorCode != 0x0)
    {
        return false;
    };
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char* buffer = new char[size + 1];
    fread(buffer, sizeof(unsigned char), size, file);
    fclose(file);
    buffer[size] = '\0';
    return buffer;
}

//-----------------------------------------------------------------------------------
std::wstring RelativeToFullPath(const std::wstring& relativePath)
{
    TCHAR tcharFilePath[MAX_PATH] = TEXT("");
    TCHAR buffer[MAX_PATH] = TEXT("");
    StringCchCopy(tcharFilePath, MAX_PATH, relativePath.c_str());
    GetFullPathName(tcharFilePath, MAX_PATH, buffer, nullptr);
    return std::wstring(buffer);
}

//-----------------------------------------------------------------------------------
std::string GetAppDataDirectory()
{
    return std::string(getenv("APPDATA"));
}

//-----------------------------------------------------------------------------------
std::vector<std::string> EnumerateFiles(const std::string& baseFolder, const std::string& filePattern, bool recurseSubfolders, const char* eventToFire)
{
    std::wstring filePatternWStr = std::wstring(filePattern.begin(), filePattern.end());
    std::wstring baseDirectory = std::wstring(baseFolder.begin(), baseFolder.end());
    return EnumerateFiles(baseDirectory, filePatternWStr, recurseSubfolders, eventToFire);
}

//-----------------------------------------------------------------------------------
std::vector<std::string> EnumerateFiles(const std::wstring& baseDirectory, const std::wstring& filePatternWStr, bool recurseSubfolders, const char* eventToFire)
{
    WIN32_FIND_DATA finder;
    HANDLE handleToResults = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;
    TCHAR tcharFilePath[MAX_PATH];
    std::vector<std::string> fileNames;
    fileNames.reserve(20);
    std::deque<std::wstring> directories;
    directories.push_back(baseDirectory);

    if (recurseSubfolders)
    {
        std::vector<std::string> recursiveDirectories = EnumerateDirectories(baseDirectory, true);
        for (std::string& directoryPath : recursiveDirectories)
        {
            std::wstring directoryWString = baseDirectory + L"\\" + std::wstring(directoryPath.begin(), directoryPath.end());
            directories.push_back(directoryWString);
        }
    }

    while (!directories.empty())
    {
        std::wstring path = directories.back();
        directories.pop_back();
        std::wstring wideFilePath = path + L"\\" + filePatternWStr;
        LPCWSTR wideFilePathCStr = wideFilePath.c_str();
        StringCchCopy(tcharFilePath, MAX_PATH, wideFilePathCStr);

        //Find the first file in the folder.
        handleToResults = FindFirstFile(tcharFilePath, &finder);

        if (INVALID_HANDLE_VALUE == handleToResults)
        {
            //Empty List
            return fileNames;
        }

        do //Add each file name in the folder to the list
        {
            if (wcscmp(finder.cFileName, L".") == 0 || wcscmp(finder.cFileName, L"..") == 0)
            {
                continue;
            }
            std::wstring wideFileName = std::wstring(finder.cFileName);
            std::string fileName = std::string(wideFileName.begin(), wideFileName.end());
            std::string fullFileName = fileName;
            if (path != baseDirectory)
            {
                auto uniquePathStringBegin = path.begin() + baseDirectory.length() + 1;
                fullFileName = std::string(uniquePathStringBegin, path.end()) + "\\" + fileName;
                fileNames.push_back(fullFileName);
            }
            else
            {
                fileNames.push_back(fileName);
            }
            if (eventToFire)
            {
                auto fileExtensionLocation = fileName.find_last_of('.');
                std::string fileNameWithoutExtension = fileName.substr(0, fileExtensionLocation);
                std::string fileExtension = fileName.substr(fileExtensionLocation, fileName.size() - fileExtensionLocation);
                std::wstring fullPathWStr = RelativeToFullPath(path + L"\\" + wideFileName);
                std::string fullPathStr = std::string(fullPathWStr.begin(), fullPathWStr.end());

                NamedProperties properties;
                properties.Set<std::string>("FileName", fileName);
                properties.Set<std::string>("FileExtension", fileExtension);
                properties.Set<std::string>("FileNameWithoutExtension", fileNameWithoutExtension);
                properties.Set<std::string>("FileRelativePath", fullFileName);
                properties.Set<std::string>("FileAbsolutePath", fullPathStr);

                EventSystem::FireEvent(eventToFire, properties);
            }

        } while (FindNextFile(handleToResults, &finder) != 0);

        dwError = GetLastError();
        if (dwError != ERROR_NO_MORE_FILES)
        {
            ERROR_AND_DIE("Error while reading files in folder, code: " + dwError);
        }
        FindClose(handleToResults);
    }
    return fileNames;
}

//-----------------------------------------------------------------------------------
std::vector<std::string> EnumerateDirectories(const std::string& baseFolder, bool recurseSubfolders /*= false*/)
{
    std::wstring baseDirectory = std::wstring(baseFolder.begin(), baseFolder.end());
    return EnumerateDirectories(baseDirectory, recurseSubfolders);
}

//-----------------------------------------------------------------------------------
std::vector<std::string> EnumerateDirectories(const std::wstring& baseDirectory, bool recurseSubfolders /*= false*/)
{
    WIN32_FIND_DATA finder;
    HANDLE handleToResults = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;
    TCHAR tcharFilePath[MAX_PATH];
    std::vector<std::string> fileNames;
    fileNames.reserve(20);
    std::deque<std::wstring> directories;
    directories.push_back(baseDirectory);

    while (!directories.empty())
    {
        std::wstring path = directories.back();
        directories.pop_back();
        std::wstring wideFilePath = path + L"\\*";
        LPCWSTR wideFilePathCStr = wideFilePath.c_str();
        StringCchCopy(tcharFilePath, MAX_PATH, wideFilePathCStr);

        //Find the first file in the folder.
        handleToResults = FindFirstFile(tcharFilePath, &finder);

        if (INVALID_HANDLE_VALUE == handleToResults)
        {
            //Empty List
            return fileNames;
        }

        do //Add each file name in the folder to the list
        {
            if (wcscmp(finder.cFileName, L".") != 0 && wcscmp(finder.cFileName, L"..") != 0 && (finder.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                if (recurseSubfolders)
                {
                    directories.push_back(path + L"\\" + std::wstring(finder.cFileName));
                }
                std::wstring wideFileName = std::wstring(finder.cFileName);
                std::string fileName = std::string(wideFileName.begin(), wideFileName.end());
                if (path != baseDirectory)
                {
                    std::string fullFileName = std::string(path.begin(), path.end()) + "\\" + fileName;
                    fileNames.push_back(fullFileName);
                }
                else
                {
                    fileNames.push_back(fileName);
                }
            }

        } while (FindNextFile(handleToResults, &finder) != 0);

        dwError = GetLastError();
        if (dwError != ERROR_NO_MORE_FILES)
        {
            ERROR_AND_DIE("Error while reading files in folder, code: " + dwError);
        }
        FindClose(handleToResults);
    }
    return fileNames;
}

//-----------------------------------------------------------------------------------
//Modified from http://www.cplusplus.com/forum/general/1796/
bool FileExists(const std::string& filename)
{
#pragma todo("check if Fopen fails here instead")
    std::ifstream ifile(filename);
    return (bool)ifile;
}

//-----------------------------------------------------------------------------------
//Modified from https://stackoverflow.com/a/6218445/2619871
bool DirectoryExists(const std::wstring& directoryPath)
{
    LPCWSTR wideDirectoryPath = directoryPath.c_str();
    DWORD dwAttrib = GetFileAttributes(wideDirectoryPath);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

//-----------------------------------------------------------------------------------
std::string GetFileExtension(const std::string& fileName)
{
    //Find the file extension.
    //Use fileName.rfind to find the first period, marking the extension.
    //Get the rest of the characters after the period and return a lowercase string.

    unsigned extensionPos = fileName.rfind('.');

    if (extensionPos != std::string::npos && extensionPos != fileName.length())
    {
        std::string fileExtension = fileName.substr((extensionPos + 1), fileName.length());
        char* fileExtensionChar = new char[fileExtension.length() + 1];
        strcpy(fileExtensionChar, fileExtension.c_str());
        for (unsigned i = 0; i < fileExtension.length(); ++i)
        {
            if (!islower(fileExtensionChar[i]))
            {
                fileExtensionChar[i] = (char)tolower(fileExtensionChar[i]);
            }
        }
        std::string lowerFileExtension(fileExtensionChar);
        delete[] fileExtensionChar;

        return lowerFileExtension;
    }

    return "ERROR";
}

//-----------------------------------------------------------------------------------
std::string GetFileName(const std::string& filePath)
{
    //Finds the file name.
    //Use filePath.rfind to find the first period, which marks the stopping position for the substring.
    //Use filePath.rfind to find the first slash (/ or \) marking the start position.
    //Returns the file name (case sensitive)

    unsigned extensionPos = filePath.rfind('.');

    if (extensionPos != std::string::npos && extensionPos != filePath.length())
    {
        //If we end up using \ to escape characters in the console this will not work, in this case for windows directories
        unsigned directoryPos = filePath.rfind('\\');
        if (directoryPos != std::string::npos && directoryPos != filePath.length())
        {
            std::string fileName = filePath.substr((directoryPos + 1), (extensionPos + 1));
            return fileName;
        }
        else
        {
            directoryPos = filePath.rfind('/');
            if (directoryPos != std::string::npos && directoryPos != filePath.length())
            {
                std::string fileName = filePath.substr((directoryPos + 1), (extensionPos + 1));
                return fileName;
            }
        }
    }

    return "ERROR";
}

//-----------------------------------------------------------------------------------
std::string GetFileDirectory(const std::string& filePath)
{
    unsigned extensionPos = filePath.rfind('.');

    if (extensionPos != std::string::npos && extensionPos != filePath.length())
    {
        //If we end up using \ to escape characters in the console this will not work, in this case for windows directories
        unsigned directoryPos = filePath.rfind('\\');
        if (directoryPos != std::string::npos && directoryPos != filePath.length())
        {
            std::string directoryPath = filePath.substr(0, (directoryPos + 1));
            return directoryPath;
        }
        else
        {
            directoryPos = filePath.rfind('/');
            if (directoryPos != std::string::npos && directoryPos != filePath.length())
            {
                std::string directoryPath = filePath.substr(0, (directoryPos + 1));
                return directoryPath;
            }
        }
    }

    return "ERROR";
}
