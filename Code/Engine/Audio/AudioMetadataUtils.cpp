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
#include "Engine/Input/Console.hpp"
#include <string>

//-----------------------------------------------------------------------------------
std::string GetFileExtension(const std::string& fileName)
{
    // Find the file extension
    unsigned extensionPos = fileName.rfind('.');

    if (extensionPos != std::string::npos && extensionPos != fileName.length())
    {
        std::string fileExtension = fileName.substr((extensionPos + 1), fileName.length());
        char* fileExtensionChar = new char[fileName.length()];
        strcpy(fileExtensionChar, fileExtension.c_str());
        for (unsigned i = 0; i < fileExtension.length(); ++i)
        {
            if (!islower(fileExtensionChar[i]))
            {
                fileExtensionChar[i] = (char)tolower(fileExtensionChar[i]);
            }
        }
        std::string lowerFileExtension = fileExtensionChar;
        delete[] fileExtensionChar;

        return lowerFileExtension;
    }

    return "ERROR";
}

//-----------------------------------------------------------------------------------
bool IncrementPlaycount(const std::string& fileName)
{
    static const char* PlaycountFrameId = "PCNT";
    TagLib::FileRef audioFile(fileName.c_str());
    TagLib::PropertyMap map = audioFile.file()->properties();
    auto playcountPropertyIter = map.find(PlaycountFrameId);
    if (playcountPropertyIter != map.end())
    {
        bool wasInt = false;
        int currentPlaycount = playcountPropertyIter->second.toString().toInt(&wasInt);
        ASSERT_OR_DIE(&wasInt, "Tried to grab the playcount, but found a non-integer value in the PCNT field.");
        map.replace(PlaycountFrameId, TagLib::String(std::to_string(currentPlaycount)));
    }
    else
    {
        map.insert(PlaycountFrameId, TagLib::String(std::to_string(1)));
    }
    audioFile.file()->save();
    return true;
}

//-----------------------------------------------------------------------------------
Texture* GetImageFromFileMetadata(const std::string& fileName)
{
    //Determine the filetype of fileName
    //If that's an unsupported type (even unsupported for the current point in time), error and return nullptr
    //Else, based on the type, construct the specific object from that filename (ex: TagLib::MPEG::File for mp3 files)
    //Then, grab the picture data (as demonstrated by my hacky code)
    //Pass the image data and size of the image data into CreateUnregisteredTextureFromData
    //Return the result

    unsigned char* srcImage = nullptr;
    unsigned long size;

    // Find the file extension
    std::string fileExtension = GetFileExtension(fileName);
    if (fileExtension == "mp3")
    {
        static const char* IdPicture = "APIC";
        TagLib::MPEG::File audioFile(fileName.c_str());
        TagLib::ID3v2::Tag* id3v2tag = audioFile.ID3v2Tag();
        TagLib::ID3v2::FrameList Frame;
        TagLib::ID3v2::AttachedPictureFrame* PicFrame;

        if (id3v2tag)
        {
            // picture frame
            Frame = id3v2tag->frameListMap()[IdPicture];
            if (!Frame.isEmpty())
            {
                for (TagLib::ID3v2::FrameList::ConstIterator it = Frame.begin(); it != Frame.end(); ++it)
                {
                    PicFrame = (TagLib::ID3v2::AttachedPictureFrame *)(*it);
                    if (PicFrame->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover)
                    {
                        // extract image (in it’s compressed form)
                        TagLib::ByteVector pictureData = PicFrame->picture();
                        size = pictureData.size();
                        srcImage = (unsigned char*)pictureData.data();
                        if (srcImage)
                        {
                            return Texture::CreateUnregisteredTextureFromData(srcImage, size);
                        }
                    }
                }
            }
        }
    }
    else if (fileExtension == "flac")
    {
        TagLib::FLAC::File audioFile(fileName.c_str());
        if (!audioFile.pictureList().isEmpty())
        {
            for (unsigned i = 0; i < audioFile.pictureList().size(); ++i)
            {
                if (audioFile.pictureList()[i]->type() == TagLib::FLAC::Picture::Type::FrontCover)
                {
                    TagLib::ByteVector pictureData = audioFile.pictureList()[i]->data();
                    size = pictureData.size();
                    srcImage = (unsigned char*)pictureData.data();
                    if (srcImage)
                    {
                        return Texture::CreateUnregisteredTextureFromData(srcImage, size);
                    }
                }
            }
        }
    }
    else if (fileExtension == "wav")
    {

    }

    Console::instance->PrintLine("Could not load album art from song!", RGBA::RED);
    return nullptr;
}

