#pragma once

#include <string>

class Texture;

Texture* GetImageFromFileMetadata(const std::string& fileName);
std::string GetFileExtension(const std::string& fileName);
std::string GetFileName(const std::string& filePath);
bool IncrementPlaycount(const std::string& fileName);