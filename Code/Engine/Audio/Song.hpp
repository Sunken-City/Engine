#pragma once

#include "Engine/Audio/Audio.hpp"
#include "ThirdParty/taglib/include/taglib/fileref.h"
#include <string>

class Song
{
public:
	Song(const std::string& fullPathToFile);
	~Song();

	void SetMetadataFromFile(const std::string& fileName);

	std::string m_filePath;
	std::string m_fileName;
	std::string m_fileExtension;
	TagLib::String m_artist;
	TagLib::String m_album;
	int m_year;
	TagLib::String m_genre;
	int m_trackNum;
	TagLib::String m_title;

	int m_playcount;
	int m_lengthInSeconds;
	int m_bitdepth;
	int m_samplerate;
	int m_bitrate;
	int m_numChannels;

	SoundID m_fmodID;
};
