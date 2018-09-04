//---------------------------------------------------------------------------
#include "Engine/Audio/Audio.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/Console.hpp"
#include "../Core/StringUtils.hpp"

AudioSystem* AudioSystem::instance = nullptr;

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(playsound)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("playsound <filename>", RGBA::RED);
        return;
    }
    std::string filepath = args.GetStringArgument(0);
    SoundID song = AudioSystem::instance->CreateOrGetSound(filepath);
    if (song == MISSING_SOUND_ID)
    {
        Console::instance->PrintLine("Could not find file.", RGBA::RED);
        return;
    }
    AudioSystem::instance->PlaySound(song);
}

//---------------------------------------------------------------------------
AudioSystem::AudioSystem()
    : m_fmodSystem( nullptr )
{
    InitializeFMOD();
}

//---------------------------------------------------------------------------
// FMOD startup code based on "GETTING STARTED With FMOD Ex Programmer’s API for Windows" document
//	from the FMOD programming API at http://www.fmod.org/download/
//
void AudioSystem::InitializeFMOD()
{
    const int MAX_AUDIO_DEVICE_NAME_LEN = 256;
    FMOD_RESULT result;
    unsigned int fmodVersion;
    int numDrivers;
    FMOD_SPEAKERMODE speakerMode;
    FMOD_CAPS deviceCapabilities;
    char audioDeviceName[ MAX_AUDIO_DEVICE_NAME_LEN ];

    // Create a System object and initialize.
    result = FMOD::System_Create( &m_fmodSystem );
    ValidateResult( result );

    result = m_fmodSystem->getVersion( &fmodVersion );
    ValidateResult( result );

    if( fmodVersion < FMOD_VERSION )
    {
        DebuggerPrintf( "AUDIO SYSTEM ERROR!  Your FMOD .dll is of an older version (0x%08x == %d) than that the .lib used to compile this code (0x%08x == %d).\n", fmodVersion, fmodVersion, FMOD_VERSION, FMOD_VERSION );
    }

    result = m_fmodSystem->getNumDrivers( &numDrivers );
    ValidateResult( result );

    if( numDrivers == 0 )
    {
        result = m_fmodSystem->setOutput( FMOD_OUTPUTTYPE_NOSOUND );
        ValidateResult( result );
    }
    else
    {
        result = m_fmodSystem->getDriverCaps( 0, &deviceCapabilities, 0, &speakerMode );
        ValidateResult( result );

        // Set the user selected speaker mode.
        result = m_fmodSystem->setSpeakerMode( speakerMode );
        ValidateResult( result );

        if( deviceCapabilities & FMOD_CAPS_HARDWARE_EMULATED )
        {
            // The user has the 'Acceleration' slider set to off! This is really bad
            // for latency! You might want to warn the user about this.
            result = m_fmodSystem->setDSPBufferSize( 1024, 10 );
            ValidateResult( result );
        }

        result = m_fmodSystem->getDriverInfo( 0, audioDeviceName, MAX_AUDIO_DEVICE_NAME_LEN, 0 );
        ValidateResult( result );

        if( strstr( audioDeviceName, "SigmaTel" ) )
        {
            // Sigmatel sound devices crackle for some reason if the format is PCM 16bit.
            // PCM floating point output seems to solve it.
            result = m_fmodSystem->setSoftwareFormat( 48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0, FMOD_DSP_RESAMPLER_LINEAR );
            ValidateResult( result );
        }
    }

    result = m_fmodSystem->init( 100, FMOD_INIT_NORMAL, 0 );
    if( result == FMOD_ERR_OUTPUT_CREATEBUFFER )
    {
        // Ok, the speaker mode selected isn't supported by this sound card. Switch it
        // back to stereo...
        result = m_fmodSystem->setSpeakerMode( FMOD_SPEAKERMODE_STEREO );
        ValidateResult( result );

        // ... and re-init.
        result = m_fmodSystem->init( 100, FMOD_INIT_NORMAL, 0 );
        ValidateResult( result );
    }
}

//---------------------------------------------------------------------------
AudioSystem::~AudioSystem()
{
// 	FMOD_RESULT result = FMOD_OK;
// 	result = FMOD_System_Close( m_fmodSystem );
// 	result = FMOD_System_Release( m_fmodSystem );
// 	m_fmodSystem = nullptr;
}

//---------------------------------------------------------------------------
void AudioSystem::StopChannel(AudioChannelHandle channel)
{
    if (channel != nullptr)
    {
        FMOD::Channel* fmodChannel = (FMOD::Channel*) channel;
        fmodChannel->stop();
    }
}

//---------------------------------------------------------------------------
void AudioSystem::StopSound(SoundID soundID)
{
    AudioChannelHandle channel = m_channels[soundID];
    if (channel != nullptr)
    {
        FMOD::Channel* fmodChannel = (FMOD::Channel*) channel;
        fmodChannel->stop();
    }
}

//-----------------------------------------------------------------------------------
void AudioSystem::MultiplyCurrentFrequency(SoundID soundID, float multiplier)
{
    SetFrequency(soundID, GetFrequency(soundID) * multiplier);
}

//-----------------------------------------------------------------------------------
void AudioSystem::SetFrequency(SoundID soundID, float frequency)
{
    AudioChannelHandle channel = m_channels[soundID];
    if (channel != nullptr)
    {
        FMOD::Channel* fmodChannel = (FMOD::Channel*) channel;
        fmodChannel->setFrequency(frequency);
    }
}

//-----------------------------------------------------------------------------------
void AudioSystem::SetFrequency(AudioChannelHandle channel, float frequency)
{
    if (channel != nullptr)
    {
        FMOD::Channel* fmodChannel = (FMOD::Channel*) channel;
        fmodChannel->setFrequency(frequency);
    }
}

//-----------------------------------------------------------------------------------
float AudioSystem::GetVolume(AudioChannelHandle channel)
{
    float volume0to1 = -1.0f;
    ((FMOD::Channel*)channel)->getVolume(&volume0to1);
    return volume0to1;
}


//-----------------------------------------------------------------------------------
void AudioSystem::SetVolume(AudioChannelHandle channel, float volume0to1)
{
    ((FMOD::Channel*)channel)->setVolume(volume0to1);
}

//-----------------------------------------------------------------------------------
float AudioSystem::GetFrequency(SoundID soundID)
{
    float frequency = -1.0f;
    AudioChannelHandle channel = m_channels[soundID];
    if (channel != nullptr)
    {
        FMOD::Channel* fmodChannel = (FMOD::Channel*) channel;
        fmodChannel->getFrequency(&frequency);
    }
    return frequency;
}

//-----------------------------------------------------------------------------------
void AudioSystem::SetMIDISpeed(SoundID soundID, float speedMultiplier)
{
    FMOD::Sound* sound = m_registeredSounds[soundID];
    if (!sound)
    {
        return;
    }

    sound->setMusicSpeed(speedMultiplier);
}

//-----------------------------------------------------------------------------------
void AudioSystem::SetMIDISpeed(RawSoundHandle songHandle, float speedMultiplier)
{
    FMOD::Sound* sound = static_cast<FMOD::Sound*>(songHandle);
    if (!sound)
    {
        return;
    }

    sound->setMusicSpeed(speedMultiplier);
}

//-----------------------------------------------------------------------------------
void AudioSystem::ReleaseRawSong(RawSoundHandle songHandle)
{
    if (songHandle)
    {
        ASSERT_OR_DIE(static_cast<FMOD::Sound*>(songHandle)->release() == FMOD_OK, "Failed to release a song.");
    }
}

//---------------------------------------------------------------------------
SoundID AudioSystem::CreateOrGetSound( const std::string& soundFileName )
{
    std::map<std::string, SoundID>::iterator found = m_registeredSoundIDs.find( soundFileName );
    if( found != m_registeredSoundIDs.end() )
    {
        return found->second;
    }
    else
    {
        FMOD::Sound* newSound = nullptr;
        m_fmodSystem->createSound( soundFileName.c_str(), FMOD_DEFAULT, nullptr, &newSound );
        if( newSound )
        {
            SoundID newSoundID = m_registeredSounds.size();
            m_registeredSoundIDs[ soundFileName ] = newSoundID;
            m_registeredSounds.push_back( newSound );
            return newSoundID;
        }
    }

    return MISSING_SOUND_ID;
}

//-----------------------------------------------------------------------------------
SoundID AudioSystem::CreateOrGetSound(const std::wstring& wideSoundFileName)
{
    char fileName[MAX_PATH * 8];
    WideCharToMultiByte(CP_UTF8, 0, wideSoundFileName.c_str(), -1, fileName, sizeof(fileName), NULL, NULL);

    std::string soundFileName(fileName);
    std::map<std::string, SoundID>::iterator found = m_registeredSoundIDs.find(soundFileName);
    if (found != m_registeredSoundIDs.end())
    {
        return found->second;
    }
    else
    {
        FMOD::Sound* newSound = nullptr;
        m_fmodSystem->createSound((char*)wideSoundFileName.c_str(), FMOD_DEFAULT | FMOD_UNICODE, nullptr, &newSound);
        if (newSound)
        {
            SoundID newSoundID = m_registeredSounds.size();
            m_registeredSoundIDs[soundFileName] = newSoundID;
            m_registeredSounds.push_back(newSound);
            return newSoundID;
        }
    }

    return MISSING_SOUND_ID;
}

//---------------------------------------------------------------------------
void AudioSystem::PlaySound( SoundID soundID, float volumeLevel )
{
    unsigned int numSounds = m_registeredSounds.size();
    if( soundID < 0 || soundID >= numSounds )
        return;

    FMOD::Sound* sound = m_registeredSounds[ soundID ];
    if( !sound )
        return;

    FMOD::Channel* channelAssignedToSound = nullptr;
    m_fmodSystem->playSound( FMOD_CHANNEL_FREE, sound, false, &channelAssignedToSound );
    if( channelAssignedToSound )
    {
        channelAssignedToSound->setVolume(volumeLevel);
    }
    m_channels[soundID] = channelAssignedToSound;
}

//-----------------------------------------------------------------------------------
void AudioSystem::PlayLoopingSound(SoundID soundID, float volumeLevel)
{
    PlaySound(soundID, volumeLevel);
    FMOD::Channel* channelAssignedToSound = static_cast<FMOD::Channel*>(m_channels[soundID]);
    channelAssignedToSound->setMode(FMOD_LOOP_NORMAL);
}

//-----------------------------------------------------------------------------------
void AudioSystem::SetLooping(SoundID soundID, bool isLooping)
{
    FMOD::Channel* channelAssignedToSound = static_cast<FMOD::Channel*>(m_channels[soundID]);
    channelAssignedToSound->setMode(isLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
}

//---------------------------------------------------------------------------
void AudioSystem::Update( float deltaSeconds )
{
    FMOD_RESULT result = m_fmodSystem->update();
    ValidateResult( result );
    //Unused
    (void)(deltaSeconds);
}

//---------------------------------------------------------------------------
void AudioSystem::ValidateResult(FMOD_RESULT result)
{
    if( result != FMOD_OK )
    {
        DebuggerPrintf( "AUDIO SYSTEM ERROR: Got error result code %d.\n", result );
        __debugbreak();
    }
}

//-----------------------------------------------------------------------------------
bool AudioSystem::IsPlaying(AudioChannelHandle channel)
{
    if (!channel)
    {
        return false;
    }

    bool isPlaying = false;
    FMOD::Channel* channelAssignedToSound = static_cast<FMOD::Channel*>(channel);
    channelAssignedToSound->isPlaying(&isPlaying);
    return isPlaying;
}

//-----------------------------------------------------------------------------------
//Returns the current waveform data from FMOD ranged from 0 to 1
void AudioSystem::GetWaveData(AudioChannelHandle channel, float* waveData)
{
    static const int LEFT_CHANNEL = 0;
    static const int RIGHT_CHANNEL = 1;
    float leftData[SPECTRUM_SIZE];
    float rightData[SPECTRUM_SIZE];

    //"This function allows retrieval of left and right data for a stereo sound individually. To combine them into one signal, simply add the entries of each seperate buffer together and then divide them by 2."
    FMOD::Channel* channelAssignedToSound = static_cast<FMOD::Channel*>(channel);
    FMOD_RESULT result = channelAssignedToSound->getWaveData(leftData, SPECTRUM_SIZE, LEFT_CHANNEL);
    ValidateResult(result);
    result = channelAssignedToSound->getWaveData(rightData, SPECTRUM_SIZE, RIGHT_CHANNEL);
    ValidateResult(result);

    //For each index, average the two together, then shift it up into the 0-1 range.
    for (int i = 0; i < SPECTRUM_SIZE; ++i)
    {
        waveData[i] = (((leftData[i] + rightData[i]) * 0.5f) + 1.0f) * 0.5f;
    }
}

//-----------------------------------------------------------------------------------
void AudioSystem::GetSpectrumData(AudioChannelHandle channel, float* spectrum)
{
    static const int LEFT_CHANNEL = 0;
    static const int RIGHT_CHANNEL = 1;
    static const int DOUBLE_SPECTRUM_SIZE = SPECTRUM_SIZE * 2;
    float leftData[DOUBLE_SPECTRUM_SIZE];
    float rightData[DOUBLE_SPECTRUM_SIZE];

    //"To get the spectrum for both channels of a stereo signal, call this function twice, once with channeloffset = 0, and again with channeloffset = 1."
    FMOD::Channel* channelAssignedToSound = static_cast<FMOD::Channel*>(channel);
    //"The functions Channel::getSpectrum and ChannelGroup::getSpectrum will give you a snapshot of the spectrum for the currently playing audio." ~ Fmod person from 2 hours of fourm digging
    FMOD_RESULT result = channelAssignedToSound->getSpectrum(leftData, DOUBLE_SPECTRUM_SIZE, LEFT_CHANNEL, FMOD_DSP_FFT_WINDOW_TRIANGLE);
    ValidateResult(result);
    result = channelAssignedToSound->getSpectrum(rightData, DOUBLE_SPECTRUM_SIZE, RIGHT_CHANNEL, FMOD_DSP_FFT_WINDOW_TRIANGLE);
    ValidateResult(result);

    //"Then add the spectrums together and divide by 2 to get the average spectrum for both channels."
    float biggestValue = 0.0f;
    for (int i = 0; i < SPECTRUM_SIZE; ++i)
    {
        float averageValue = (leftData[i] + rightData[i]) * 0.5f;
        spectrum[i] = averageValue;
        biggestValue = biggestValue < averageValue ? averageValue : biggestValue;
    }

    //Normalize the data: "find the maximum value in the resulting spectrum, and scale all values in the array by 1 / max. (ie if the max was 0.5f, then it would become 1)"
    float normalizingValue = (1.0f / biggestValue);
    for (int i = 0; i < SPECTRUM_SIZE; ++i)
    {
        spectrum[i] *= normalizingValue;
    }

    for (int i = 0; i < SPECTRUM_SIZE; ++i)
    {
        spectrum[i] = 20.0f * (float)log10(spectrum[i]);
        spectrum[i] = MathUtils::RangeMap(spectrum[i], -70.0f, 6.0f, 0.0f, 1.0f);
        spectrum[i] = MathUtils::Clamp(spectrum[i], 0.0f, 1.0f);
    }
}


//-----------------------------------------------------------------------------------
unsigned int AudioSystem::GetPlaybackPositionMS(AudioChannelHandle channel)
{
    unsigned int outTimestampMS = 0;
    ASSERT_OR_DIE(channel, "Channel passed to GetPlaybackPositionMS was null.");

    FMOD::Channel* channelAssignedToSound = static_cast<FMOD::Channel*>(channel);
    channelAssignedToSound->getPosition(&outTimestampMS, FMOD_TIMEUNIT_MS);
    return outTimestampMS;
}

//-----------------------------------------------------------------------------------
void AudioSystem::SetPlaybackPositionMS(AudioChannelHandle channel, unsigned int timestampMS)
{
    ASSERT_OR_DIE(channel, "Channel passed to SetPlaybackPositionMS was null.");

    FMOD::Channel* channelAssignedToSound = static_cast<FMOD::Channel*>(channel);
    channelAssignedToSound->setPosition(timestampMS, FMOD_TIMEUNIT_MS);
}

//-----------------------------------------------------------------------------------
unsigned int AudioSystem::GetSoundLengthMS(SoundID soundHandle)
{
    unsigned int outSoundLengthMS = 0;
    FMOD::Sound* sound = m_registeredSounds[soundHandle];

    sound->getLength(&outSoundLengthMS, FMOD_TIMEUNIT_MS);

    return outSoundLengthMS;
}

//-----------------------------------------------------------------------------------
unsigned int AudioSystem::GetSoundLengthMS(RawSoundHandle songHandle)
{
    unsigned int outSoundLengthMS = 0;
    FMOD::Sound* sound = static_cast<FMOD::Sound*>(songHandle);

    sound->getLength(&outSoundLengthMS, FMOD_TIMEUNIT_MS);

    return outSoundLengthMS;
}

//-----------------------------------------------------------------------------------
AudioChannelHandle AudioSystem::GetChannel(SoundID songHandle)
{
    AudioChannelHandle channelHandle = nullptr;

    auto foundChannel = m_channels.find(songHandle);
    if (foundChannel != m_channels.end())
    {
        channelHandle = (*foundChannel).second;
    }

    return channelHandle;
}

//-----------------------------------------------------------------------------------
RawSoundHandle AudioSystem::LoadRawSound(const std::wstring& wideSoundFileName, unsigned int& errorValue)
{
    char fileName[MAX_PATH * 8];
    WideCharToMultiByte(CP_UTF8, 0, wideSoundFileName.c_str(), -1, fileName, sizeof(fileName), NULL, NULL);

    FMOD::Sound* newSound = nullptr;
    errorValue = static_cast<unsigned int>(m_fmodSystem->createSound((char*)wideSoundFileName.c_str(), FMOD_DEFAULT | FMOD_UNICODE, nullptr, &newSound));
    return newSound;
}

//-----------------------------------------------------------------------------------
AudioChannelHandle AudioSystem::PlayRawSong(RawSoundHandle songHandle, float volumeLevel /*= 1.f*/)
{
    FMOD::Sound* sound = (FMOD::Sound*)songHandle;
    ASSERT_OR_DIE(sound, "Couldn't play the song handle from PlayRawSong");

    FMOD::Channel* channelAssignedToSound = nullptr;
    m_fmodSystem->playSound(FMOD_CHANNEL_FREE, sound, false, &channelAssignedToSound);
    if (channelAssignedToSound)
    {
        channelAssignedToSound->setVolume(volumeLevel);
    }
    return channelAssignedToSound;
}

//-----------------------------------------------------------------------------------
void AudioSystem::SetLooping(AudioChannelHandle rawSongChannel, bool isLooping)
{
    FMOD::Channel* channelAssignedToSound = static_cast<FMOD::Channel*>(rawSongChannel);
    channelAssignedToSound->setMode(isLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
}

//-----------------------------------------------------------------------------------
void AudioSystem::CreateDSPByType(FMOD_DSP_TYPE type, DSPHandle** dsp)
{
    bool success = true;
    FMOD_RESULT result = AudioSystem::instance->m_fmodSystem->createDSPByType(type, dsp);
    if (result)
    {
        success = false;
    }

    ASSERT_RECOVERABLE(success, "Couldn't create DSP");
}