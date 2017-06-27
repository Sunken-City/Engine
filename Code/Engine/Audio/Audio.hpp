#ifndef INCLUDED_AUDIO
#define INCLUDED_AUDIO
#pragma once
#undef PlaySound

#pragma comment( lib, "ThirdParty/fmod/fmodex_vc" ) // Link in the fmodex_vc.lib static library

#ifdef TAGLIB_INCLUDED
#ifdef _DEBUG
#pragma comment( lib, "ThirdParty/taglib/lib/tag_debug" )
#else
#pragma comment( lib, "ThirdParty/taglib/lib/tag" )
#endif

#include "ThirdParty/taglib/include/taglib/taglib.h"
#include "ThirdParty/taglib/include/taglib/fileref.h"
#include "ThirdParty/taglib/include/taglib/id3v2tag.h"
#include "ThirdParty/taglib/include/taglib/id3v2frame.h"
#include "ThirdParty/taglib/include/taglib/id3v2header.h"
#include "ThirdParty/taglib/include/taglib/mpegfile.h"
#include "ThirdParty/taglib/include/taglib/attachedpictureframe.h"
#include "ThirdParty/taglib/include/taglib/tag.h"
#endif

#include "ThirdParty/fmod/fmod.hpp"
#include <string>
#include <vector>
#include <map>

typedef unsigned int SoundID;
typedef void* AudioChannelHandle;
const unsigned int MISSING_SOUND_ID = 0xffffffff;

//-----------------------------------------------------------------------------------
class AudioSystem
{
public:
    AudioSystem();
    virtual ~AudioSystem();
    SoundID CreateOrGetSound( const std::string& soundFileName );
    void PlaySound(SoundID soundID, float volumeLevel = 1.f);
    void PlayLoopingSound(SoundID soundID, float volumeLevel = 1.f);
    void SetLooping(SoundID sound, bool isLooping = true);
    void Update(float deltaSeconds); // Must be called at regular intervals (e.g. every frame)
    void StopChannel(AudioChannelHandle channel);
    void StopSound(SoundID soundID);
    void MultiplyCurrentFrequency(SoundID soundID, float multiplier);
    void SetFrequency(SoundID soundID, float frequency); //Do not use this if you're trying to use a small value!
    float GetFrequency(SoundID soundID);
    AudioChannelHandle GetChannel(SoundID m_currentlyPlayingSong);
    bool IsPlaying(AudioChannelHandle channel);
    unsigned int GetPlaybackPositionMS(AudioChannelHandle channel);

    static AudioSystem* instance;

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    static constexpr float RPM_45_AT_33_FREQUENCY_MULTIPLIER = 33.333333f / 45.0f; //33.333RPM / 45RPM
    static constexpr float RPM_33_AT_45_FREQUENCY_MULTIPLIER = 1.0f / RPM_45_AT_33_FREQUENCY_MULTIPLIER; //45RPM / 33.333RPM
    static constexpr float RPM_16_AT_33_FREQUENCY_MULTIPLIER = 33.333333f / 16.66666f;
    static constexpr float RPM_33_AT_16_FREQUENCY_MULTIPLIER = 1.0f / RPM_16_AT_33_FREQUENCY_MULTIPLIER;
    static constexpr float RPM_16_AT_45_FREQUENCY_MULTIPLIER = 45.0f / 16.66666f;
    static constexpr float RPM_45_AT_16_FREQUENCY_MULTIPLIER = 1.0f / RPM_16_AT_45_FREQUENCY_MULTIPLIER;
    //TODO: Add 76 and 80RPM

protected:
    void InitializeFMOD();
    void ValidateResult( FMOD_RESULT result );
protected:
    FMOD::System*							m_fmodSystem;
    std::map< std::string, SoundID >		m_registeredSoundIDs;
    std::vector< FMOD::Sound* >				m_registeredSounds;
    std::map<SoundID, AudioChannelHandle>	m_channels;
};

#endif // INCLUDED_AUDIO
