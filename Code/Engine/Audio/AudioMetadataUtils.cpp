#include "Engine/Audio/AudioMetadataUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "ThirdParty/taglib/include/taglib/mpegfile.h"
#include "ThirdParty/taglib/include/taglib/id3v2tag.h"
#include "ThirdParty/taglib/include/taglib/attachedpictureframe.h"
#include "ThirdParty/taglib/include/taglib/flacfile.h"
#include "ThirdParty/taglib/include/taglib/unknownframe.h"
#include "ThirdParty/taglib/include/taglib/tfile.h"
#include "ThirdParty/taglib/include/taglib/tpropertymap.h"
#include "ThirdParty/taglib/include/taglib/fileref.h"
#include "ThirdParty/taglib/include/taglib/wavfile.h"
#include "ThirdParty/taglib/include/taglib/rifffile.h"
#include "ThirdParty/taglib/include/taglib/oggflacfile.h"
#include "ThirdParty/taglib/include/taglib/vorbisfile.h"
#include "ThirdParty/taglib/include/taglib/oggfile.h"
#include "Engine/Input/Console.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include <string>

//-----------------------------------------------------------------------------------
bool IncrementPlaycount(const std::wstring& fileName)
{
    //Increment the playcount for each file type. 
    //Since the default implementation of setProperties (by using a generic TagLib::FileRef)
    //only supports writing a few common tags, we need to use each file type's specific setProperties method.
    //Grab the property map from the file and try to find the PCNT frame. If it doesn't exist, insert it with
    //an initial value. Set the properties on the file and save.

    static const char* PlaycountFrameId = "PCNT";
    std::string fileExtension = GetFileExtension(std::string(fileName.begin(), fileName.end()));

    if (fileExtension == "flac")
    {
        TagLib::FLAC::File flacFile(fileName.c_str());
        TagLib::PropertyMap map = flacFile.properties();
        auto playcountPropertyIter = map.find(PlaycountFrameId);
        if (playcountPropertyIter != map.end())
        {
            bool wasInt = false;
            int currentPlaycount = playcountPropertyIter->second.toString().toInt(&wasInt);
            ASSERT_OR_DIE(&wasInt, "Tried to grab the playcount, but found a non-integer value in the PCNT field.");
            map.replace(PlaycountFrameId, TagLib::String(std::to_string(currentPlaycount + 1)));
        }
        else
        {
            map.insert(PlaycountFrameId, TagLib::String("1"));
        }

        //Create the Xiph comment if it doesn't already exist
        if (!flacFile.hasXiphComment())
        {
            flacFile.xiphComment(1);
        }

        TagLib::Ogg::XiphComment* flacTags = flacFile.xiphComment();
        flacTags->setProperties(map);
        flacFile.save();
    }
    else if (fileExtension == "wav")
    {
        TagLib::RIFF::WAV::File wavFile(fileName.c_str());
        TagLib::PropertyMap map = wavFile.properties();
        auto playcountPropertyIter = map.find(PlaycountFrameId);
        if (playcountPropertyIter != map.end())
        {
            bool wasInt = false;
            int currentPlaycount = playcountPropertyIter->second.toString().toInt(&wasInt);
            ASSERT_OR_DIE(&wasInt, "Tried to grab the playcount, but found a non-integer value in the PCNT field.");
            map.replace(PlaycountFrameId, TagLib::String(std::to_string(currentPlaycount + 1)));
        }
        else
        {
            map.insert(PlaycountFrameId, TagLib::String("1"));
        }

        wavFile.setProperties(map);
        wavFile.save();
    }
    else if (fileExtension == "mp3")
    {
        TagLib::MPEG::File mp3File(fileName.c_str());
        TagLib::PropertyMap map = mp3File.properties();
        auto playcountPropertyIter = map.find(PlaycountFrameId);
        if (playcountPropertyIter != map.end())
        {
            bool wasInt = false;
            int currentPlaycount = playcountPropertyIter->second.toString().toInt(&wasInt);
            ASSERT_OR_DIE(&wasInt, "Tried to grab the playcount, but found a non-integer value in the PCNT field.");
            map.replace(PlaycountFrameId, TagLib::String(std::to_string(currentPlaycount + 1)));
        }
        else
        {
            map.insert(PlaycountFrameId, TagLib::String("1"));
        }

        if (!mp3File.hasID3v2Tag())
        {
            mp3File.ID3v2Tag(1);
        }

        mp3File.setProperties(map);
        mp3File.save();
    }
    else if (fileExtension == "ogg")
    {
        TagLib::Ogg::Vorbis::File oggFile(fileName.c_str());
        TagLib::PropertyMap map = oggFile.properties();
        auto playcountPropertyIter = map.find(PlaycountFrameId);
        if (playcountPropertyIter != map.end())
        {
            bool wasInt = false;
            int currentPlaycount = playcountPropertyIter->second.toString().toInt(&wasInt);
            ASSERT_OR_DIE(&wasInt, "Tried to grab the playcount, but found a non-integer value in the PCNT field.");
            map.replace(PlaycountFrameId, TagLib::String(std::to_string(currentPlaycount + 1)));
        }
        else
        {
            map.insert(PlaycountFrameId, TagLib::String("1"));
        }

        TagLib::Ogg::XiphComment* oggTags = oggFile.tag();
        oggTags->setProperties(map);
        oggFile.save();
    }

    return true;
}

//-----------------------------------------------------------------------------------
long GetSongDurationSeconds(const std::wstring& fileName)
{
    TagLib::FileRef file(std::string(fileName.begin(), fileName.end()).c_str());
    return file.audioProperties()->lengthInSeconds();
}

//-----------------------------------------------------------------------------------
Texture* GetImageFromFileMetadata(const std::wstring& filePath, const std::string& textureName)
{
    //Determine the filetype of fileName
    //If that's an unsupported type (even unsupported for the current point in time), error and return nullptr
    //Else, based on the type, construct the specific object from that filename (ex: TagLib::MPEG::File for mp3 files)
    //Then, grab the picture data (as demonstrated by my hacky code)
    //Pass the image data and size of the image data into CreateUnregisteredTextureFromData
    //Return the result

    unsigned char* srcImage = nullptr;
    unsigned long size;

    //Find the file extension
    std::string strFileName = std::string(filePath.begin(), filePath.end());
    std::string fileExtension = GetFileExtension(strFileName);
    if (fileExtension == "mp3")
    {
        TagLib::MPEG::File audioFile(filePath.c_str());

        if (audioFile.hasID3v2Tag())
        {
            static const char* IdPicture = "APIC";
            TagLib::ID3v2::Tag* id3v2tag = audioFile.ID3v2Tag();
            TagLib::ID3v2::FrameList Frame;
            TagLib::ID3v2::AttachedPictureFrame* PicFrame;

            //Picture frame
            Frame = id3v2tag->frameListMap()[IdPicture];
            if (!Frame.isEmpty())
            {
                for (TagLib::ID3v2::FrameList::ConstIterator it = Frame.begin(); it != Frame.end(); ++it)
                {
                    PicFrame = (TagLib::ID3v2::AttachedPictureFrame*)(*it);
                    if (PicFrame->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover)
                    {
                        //Extract image (in it’s compressed form)
                        TagLib::ByteVector pictureData = PicFrame->picture();
                        size = pictureData.size();
                        srcImage = (unsigned char*)pictureData.data();
                        if (srcImage)
                        {
                            return Texture::CreateTextureFromImageFileData(textureName, srcImage, size);
                        }
                    }
                }
            }
        }
    }
    else if (fileExtension == "flac")
    {
        TagLib::FLAC::File audioFile(filePath.c_str());
        auto pictureList = audioFile.pictureList();

        if (!pictureList.isEmpty())
        {
            for (unsigned i = 0; i < audioFile.pictureList().size(); ++i)
            {
                if (pictureList[i]->type() == TagLib::FLAC::Picture::Type::FrontCover)
                {
                    TagLib::ByteVector pictureData = pictureList[i]->data();
                    size = pictureData.size();
                    srcImage = (unsigned char*)pictureData.data();
                    if (srcImage)
                    {
                        return Texture::CreateTextureFromImageFileData(textureName, srcImage, size);
                    }
                }
            }
        }
        else if (audioFile.hasID3v2Tag())
        {
            static const char* IdPicture = "APIC";
            TagLib::ID3v2::Tag* id3v2tag = audioFile.ID3v2Tag();
            TagLib::ID3v2::FrameList Frame;
            TagLib::ID3v2::AttachedPictureFrame* PicFrame;

            //Picture frame
            Frame = id3v2tag->frameListMap()[IdPicture];
            if (!Frame.isEmpty())
            {
                for (TagLib::ID3v2::FrameList::ConstIterator it = Frame.begin(); it != Frame.end(); ++it)
                {
                    PicFrame = (TagLib::ID3v2::AttachedPictureFrame*)(*it);
                    if (PicFrame->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover)
                    {
                        //Extract image (in it’s compressed form)
                        TagLib::ByteVector pictureData = PicFrame->picture();
                        size = pictureData.size();
                        srcImage = (unsigned char*)pictureData.data();
                        if (srcImage)
                        {
                            return Texture::CreateTextureFromImageFileData(textureName, srcImage, size);
                        }
                    }
                }
            }
        }
    }
    else if (fileExtension == "wav")
    {
        static const char* IdPicture = "APIC";
        TagLib::RIFF::WAV::File audioFile(filePath.c_str());
        TagLib::ID3v2::Tag* id3v2tag = audioFile.ID3v2Tag();
        TagLib::ID3v2::FrameList Frame;
        TagLib::ID3v2::AttachedPictureFrame* PicFrame;

        if (audioFile.hasID3v2Tag())
        {
            //Picture frame
            Frame = id3v2tag->frameListMap()[IdPicture];
            if (!Frame.isEmpty())
            {
                for (TagLib::ID3v2::FrameList::ConstIterator it = Frame.begin(); it != Frame.end(); ++it)
                {
                    PicFrame = (TagLib::ID3v2::AttachedPictureFrame*)(*it);
                    if (PicFrame->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover)
                    {
                        //Extract image (in it’s compressed form)
                        TagLib::ByteVector pictureData = PicFrame->picture();
                        size = pictureData.size();
                        srcImage = (unsigned char*)pictureData.data();
                        if (srcImage)
                        {
                            return Texture::CreateTextureFromImageFileData(textureName, srcImage, size);
                        }
                    }
                }
            }
        }
    }
    else if (fileExtension == "ogg")
    {
        TagLib::Ogg::Vorbis::File audioFile(filePath.c_str());
        auto pictureList = audioFile.tag()->pictureList();

        if (!pictureList.isEmpty())
        {
            for (unsigned i = 0; i < pictureList.size(); ++i)
            {
                if (pictureList[i]->type() == TagLib::FLAC::Picture::Type::FrontCover)
                {
                    TagLib::ByteVector pictureData = pictureList[i]->data();
                    size = pictureData.size();
                    srcImage = (unsigned char*)pictureData.data();
                    if (srcImage)
                    {
                        return Texture::CreateTextureFromImageFileData(textureName, srcImage, size);
                    }
                }
            }
        }
    }


    //Attempt to grab the first image file in the folder to use as album art
    std::string directoryName = GetFileDirectory(std::string(filePath.begin(), filePath.end()));

    std::vector<std::string> pngFiles = EnumerateFiles(directoryName, "*.png");
    if (pngFiles.size() > 0)
    {
        return Texture::CreateOrGetTexture(std::string(directoryName + pngFiles[0]));
    }

    std::vector<std::string> jpgFiles = EnumerateFiles(directoryName, "*.jpg");
    if (jpgFiles.size() > 0)
    {
        return Texture::CreateOrGetTexture(std::string(directoryName + jpgFiles[0]));
    }

    std::vector<std::string> jpegFiles = EnumerateFiles(directoryName, "*.jpeg");
    if (jpegFiles.size() > 0)
    {
        return Texture::CreateOrGetTexture(std::string(directoryName + jpegFiles[0]));
    }

    return nullptr;
}

//-----------------------------------------------------------------------------------
std::vector<std::wstring> GetSupportedAudioFiles(const std::wstring& folder)
{
    std::vector<std::wstring> mp3s = EnumerateWideFiles(folder, L"*.mp3");
    std::vector<std::wstring> flacs = EnumerateWideFiles(folder, L"*.flac");
    std::vector<std::wstring> oggs = EnumerateWideFiles(folder, L"*.ogg");
    std::vector<std::wstring> wavs = EnumerateWideFiles(folder, L"*.wav");

    std::vector<std::wstring> songs;
    songs.reserve(mp3s.size() + flacs.size() + oggs.size() + wavs.size());
    songs.insert(songs.end(), mp3s.begin(), mp3s.end());
    songs.insert(songs.end(), flacs.begin(), flacs.end());
    songs.insert(songs.end(), oggs.begin(), oggs.end());
    songs.insert(songs.end(), wavs.begin(), wavs.end());

    return songs;
}

//-----------------------------------------------------------------------------------
unsigned int GetUncompressedFilesize(const std::wstring& fileName)
{
    std::string filePathStr = std::string(fileName.begin(), fileName.end());
    std::string fileExtension = GetFileExtension(filePathStr);

    if (fileExtension == "wav")
    {
        TagLib::RIFF::WAV::File wavFile(fileName.c_str());
        TagLib::RIFF::WAV::Properties* audioProperties = wavFile.audioProperties();
        int numChannels = audioProperties->channels();
        int bitsPerSample = audioProperties->bitsPerSample();
        int sampleRate = audioProperties->sampleRate();
        int lengthInSeconds = audioProperties->lengthInSeconds();

        return (numChannels * bitsPerSample * sampleRate * lengthInSeconds) / 8; //Divide by 8 to get bytes instead of bits
    }
    else if (fileExtension == "mp3")
    {
        TagLib::MPEG::File mp3File(fileName.c_str());
        TagLib::MPEG::Properties* audioProperties = mp3File.audioProperties();
        int numChannels = audioProperties->channels();
        int bitsPerSample = 16; //This function doesn't exist in taglib for mp3, assume 16bit
        int sampleRate = audioProperties->sampleRate();
        int lengthInSeconds = audioProperties->lengthInSeconds();

        return (numChannels * bitsPerSample * sampleRate * lengthInSeconds) / 8;
    }
    else if (fileExtension == "flac")
    {
        TagLib::FLAC::File flacFile(fileName.c_str());
        TagLib::FLAC::Properties* audioProperties = flacFile.audioProperties();
        int numChannels = audioProperties->channels();
        int bitsPerSample = audioProperties->bitsPerSample();
        int sampleRate = audioProperties->sampleRate();
        int lengthInSeconds = audioProperties->lengthInSeconds();

        return (numChannels * bitsPerSample * sampleRate * lengthInSeconds) / 8;
    }
    else if (fileExtension == "ogg")
    {
        TagLib::Ogg::Vorbis::File oggFile(fileName.c_str());
        TagLib::Ogg::Vorbis::AudioProperties* audioProperties = oggFile.audioProperties();
        int numChannels = audioProperties->channels();
        int bitsPerSample = 16; //Same as for mp3; assume 16bit
        int sampleRate = audioProperties->sampleRate();
        int lengthInSeconds = audioProperties->lengthInSeconds();

        return (numChannels * bitsPerSample * sampleRate * lengthInSeconds) / 8;
    }

    return 0; //File type isn't supported
}