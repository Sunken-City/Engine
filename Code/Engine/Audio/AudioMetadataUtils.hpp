#pragma once

#include <string>
#include <vector>

//If we're using this header, we're using Taglib.
#ifndef TAGLIB_INCLUDED
#define TAGLIB_INCLUDED
#endif

class Texture;

Texture* GetImageFromFileMetadata(const std::wstring& filePath, const std::string& textureName);
bool IncrementPlaycount(const std::wstring& fileName);
std::vector<std::wstring> GetSupportedAudioFiles(const std::wstring& folder);
unsigned int GetUncompressedFilesize(const std::wstring& fileName);