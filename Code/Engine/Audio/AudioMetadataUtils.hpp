#pragma once

#include <string>

//If we're using this header, we're using Taglib.
#ifndef TAGLIB_INCLUDED
#define TAGLIB_INCLUDED
#endif

class Texture;

Texture* GetImageFromFileMetadata(const std::wstring& fileName);
bool IncrementPlaycount(const std::wstring& fileName);
std::vector<std::wstring> GetSupportedFiles(const std::wstring& folder);