#include "Engine/Audio/AudioMetadataUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "ThirdParty/taglib/include/taglib/mpegfile.h"
#include "ThirdParty/taglib/include/taglib/id3v2tag.h"
#include "ThirdParty/taglib/include/taglib/attachedpictureframe.h"
#include "Engine/Input/Console.hpp"

//-----------------------------------------------------------------------------------
Texture* GetImageFromFileMetadata(const std::string& fileName)
{
    //Determine the filetype of fileName
    //If that's an unsupported type (even unsupported for the current point in time), error and return nullptr
    //Else, based on the type, construct the specific object from that filename (ex: TagLib::MPEG::File for mp3 files)
    //Then, grab the picture data (as demonstrated by my hacky code)
    //Pass the image data and size of the image data into CreateUnregisteredTextureFromData
    //Return the result

	// Find the file extension
	int extensionPos = fileName.rfind('.');

	if (extensionPos != std::string::npos && extensionPos != fileName.length())
	{
		std::string fileExtension = fileName.substr((extensionPos + 1), fileName.length());
		if (fileExtension == "mp3")
		{
			static const char* IdPicture = "APIC";
			TagLib::MPEG::File audioFile(fileName.c_str());
			TagLib::ID3v2::Tag* id3v2tag = audioFile.ID3v2Tag();
			TagLib::ID3v2::FrameList Frame;
			TagLib::ID3v2::AttachedPictureFrame* PicFrame;
			unsigned long size;
			unsigned char* srcImage = nullptr;

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
	}

	Console::instance->PrintLine("Could not load album art from song!", RGBA::RED);
    return nullptr;
}

