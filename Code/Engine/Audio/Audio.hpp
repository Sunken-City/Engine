#ifndef INCLUDED_AUDIO
#define INCLUDED_AUDIO
#pragma once
#undef PlaySound

#pragma comment( lib, "ThirdParty/fmod/fmodex_vc" ) // Link in the fmodex_vc.lib static library
#include "ThirdParty/fmod/fmod.hpp"
#include "ThirdParty/taglib/include/taglib/taglib.h" // Temporary
#include "ThirdParty/taglib/include/taglib/fileref.h"
#include "ThirdParty/taglib/include/taglib/tag.h"
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
    void Update(float deltaSeconds); // Must be called at regular intervals (e.g. every frame)
    void StopChannel(AudioChannelHandle channel);
    void StopSound(SoundID soundID);
	void PrintTag(SoundID soundID);
    void MultiplyCurrentFrequency(SoundID soundID, float multiplier);
    void SetFrequency(SoundID soundID, float frequency);
    float GetFrequency(SoundID soundID);
    static AudioSystem* instance;

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
