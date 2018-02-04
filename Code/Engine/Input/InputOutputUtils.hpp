#pragma once
#include <vector>
#include <string>

bool LoadBufferFromBinaryFile(std::vector<unsigned char>& out_buffer, const std::string& filePath);
bool SaveBufferToBinaryFile(const std::vector<unsigned char>& buffer, const std::string& filePath);
bool EnsureDirectoryExists(const std::string& directoryPath);
bool EnsureFileExists(const std::string& fullFilePath);
bool EnsureFileExists(const std::wstring& fullFilePath);
bool ReadTextFileIntoVector(std::vector<std::string>& outBuffer, const std::string& filePath);
char* FileReadIntoNewBuffer(const std::string& filePath);
std::vector<std::string> EnumerateFiles(const std::string& baseFolder, const std::string& filePattern, bool recurseSubfolders = false, const char* eventToFire = nullptr);
std::vector<std::string> EnumerateFiles(const std::wstring& baseFolder, const std::wstring& filePattern, bool recurseSubfolders = false, const char* eventToFire = nullptr);
std::vector<std::wstring> EnumerateWideFiles(const std::wstring& baseDirectory, const std::wstring& filePatternWStr, bool recurseSubfolders = false, const char* eventToFire = nullptr);
std::vector<std::string> EnumerateDirectories(const std::string& baseFolder, bool recurseSubfolders = false);
std::vector<std::string> EnumerateDirectories(const std::wstring& baseFolder, bool recurseSubfolders = false);
std::vector<std::wstring> EnumerateWideDirectories(const std::wstring& baseDirectory, bool recurseSubfolders = false);
bool FileExists(const std::string& filename);
bool FileExists(const std::wstring& filename);
bool DirectoryExists(const std::wstring& directoryPath);
std::wstring RelativeToFullPath(const std::wstring& relativePath);
std::string GetAppDataDirectory();
std::string GetFileExtension(const std::string& fileName);
std::wstring GetFileExtension(const std::wstring& fileName);
std::string GetFileName(const std::string& filePath);
std::string GetFileDirectory(const std::string& filePath);
bool IsDirectory(const std::wstring& path);

//-----------------------------------------------------------------------------------
template<typename T>
void ByteSwap(T* source, const size_t numBytes)
{
    byte* data = (byte*)source;
    byte* start = data;
    byte* end = data + numBytes - 1;

    while (start < end)
    {
        byte temp = *start;
        *start = *end;
        *end = temp;
        ++start;
        --end;
    }

    source = (T*)data;
}