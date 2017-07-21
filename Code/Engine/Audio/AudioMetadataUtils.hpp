#pragma once

#include <string>

//If we're using this header, we're using Taglib.
#ifndef TAGLIB_INCLUDED
#define TAGLIB_INCLUDED
#endif

class Texture;

Texture* GetImageFromFileMetadata(const std::string& fileName);
bool IncrementPlaycount(const std::string& fileName);