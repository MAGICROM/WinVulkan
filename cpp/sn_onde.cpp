// Including the necessary libraries for OpenAL and file input/output.
/**/
//#define ONDECORE_CODE for using the library
#define ONDECORE_IMPLEMENTATION
#include <onde/OndeCore.h>

#include <al.h>
#include <alc.h>

#include <conio.h>
#include <cstdio>
#include <cstring>

#include <iostream>
#include <vector>

#include <chrono>
#include <ctime>
#include <thread>
#include <vector>
#include <iomanip>

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

struct SoundBuffer
{
	ALuint SoundBuffer;
	void Load(const char* filename);
	void Release();
	operator ALuint(){return SoundBuffer;} 
};

struct Sound
{
	ALuint Source;
	float* _pos;
	float* _vel;
	void Update();
	void Release();
	
};

#define NUMBUFFERS              (4)
#define	SERVICE_UPDATE_PERIOD	(20)

size_t ov_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	return fread(ptr, size, nmemb, (FILE*)datasource);
}

int ov_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
	return fseek((FILE*)datasource, (long)offset, whence);
}

int ov_close_func(void *datasource)
{
   return fclose((FILE*)datasource);
}

long ov_tell_func(void *datasource)
{
	return ftell((FILE*)datasource);
}

void Swap(short &s1, short &s2)
{
	short sTemp = s1;
	s1 = s2;
	s2 = sTemp;
}

unsigned long DecodeOggVorbis(OggVorbis_File *psOggVorbisFile, char *pDecodeBuffer, unsigned long ulBufferSize, unsigned long ulChannels)
{
	int current_section;
	long lDecodeSize;
	unsigned long ulSamples;
	short *pSamples;

	unsigned long ulBytesDone = 0;
	while (1)
	{
		lDecodeSize = ov_read(psOggVorbisFile, pDecodeBuffer + ulBytesDone, ulBufferSize - ulBytesDone, 0, 2, 1, &current_section);
		if (lDecodeSize > 0)
		{
			ulBytesDone += lDecodeSize;

			if (ulBytesDone >= ulBufferSize)
				break;
		}
		else
		{
			break;
		}
	}

	// Mono, Stereo and 4-Channel files decode into the same channel order as WAVEFORMATEXTENSIBLE,
	// however 6-Channels files need to be re-ordered
	if (ulChannels == 6)
	{		
		pSamples = (short*)pDecodeBuffer;
		for (ulSamples = 0; ulSamples < (ulBufferSize>>1); ulSamples+=6)
		{
			// WAVEFORMATEXTENSIBLE Order : FL, FR, FC, LFE, RL, RR
			// OggVorbis Order            : FL, FC, FR,  RL, RR, LFE
			Swap(pSamples[ulSamples+1], pSamples[ulSamples+2]);
			Swap(pSamples[ulSamples+3], pSamples[ulSamples+5]);
			Swap(pSamples[ulSamples+4], pSamples[ulSamples+5]);
		}
	}

	return ulBytesDone;
}

struct Music
{
	ALuint		    uiBuffers[NUMBUFFERS];
	ALuint		    uiSource;
	ALuint			uiBuffer;
	ALint			iState;
	
	ALint			iBuffersProcessed, iTotalBuffersProcessed, iQueuedBuffers;
	unsigned long	ulFrequency = 0;
	unsigned long	ulFormat = 0;
	unsigned long	ulChannels = 0;
	unsigned long	ulBufferSize = 0;
	unsigned long	ulBytesWritten = 0;
	char*			pDecodeBuffer;

	// Open Ogg Stream
	FILE *pOggVorbisFile = nullptr;
	ov_callbacks	sCallbacks;
	OggVorbis_File	sOggVorbisFile;
	vorbis_info* psVorbisInfo;

	void Release();
	int Continue();
	void LoadAndStart(const char* filename);
};
void Music::Release()
{
	// Stop the Source and clear the Queue
	alSourceStop(uiSource);
	alSourcei(uiSource, AL_BUFFER, 0);

	if (pDecodeBuffer)
	{
		free(pDecodeBuffer);
		pDecodeBuffer = NULL;
	}

	// Clean up buffers and sources
	alDeleteSources( 1, &uiSource );
	alDeleteBuffers( NUMBUFFERS, uiBuffers );
	// Close OggVorbis stream
	ov_clear(&sOggVorbisFile);
}
int Music::Continue()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(20));

				// Request the number of OpenAL Buffers have been processed (played) on the Source
				iBuffersProcessed = 0;
				alGetSourcei(uiSource, AL_BUFFERS_PROCESSED, &iBuffersProcessed);

				// Keep a running count of number of buffers processed (for logging purposes only)
				iTotalBuffersProcessed += iBuffersProcessed;
				

				// For each processed buffer, remove it from the Source Queue, read next chunk of audio
				// data from disk, fill buffer with new data, and add it to the Source Queue
				while (iBuffersProcessed)
				{
					
					// Remove the Buffer from the Queue.  (uiBuffer contains the Buffer ID for the unqueued Buffer)
					uiBuffer = 0;
					alSourceUnqueueBuffers(uiSource, 1, &uiBuffer);
					

					// Read more audio data (if there is any)
					ulBytesWritten = DecodeOggVorbis(&sOggVorbisFile, pDecodeBuffer, ulBufferSize, ulChannels);
					if (ulBytesWritten)
					{
						
						alBufferData(uiBuffer, ulFormat, pDecodeBuffer, ulBytesWritten, ulFrequency);
						alSourceQueueBuffers(uiSource, 1, &uiBuffer);
					}

					iBuffersProcessed--;
				}

				// Check the status of the Source.  If it is not playing, then playback was completed,
				// or the Source was starved of audio data, and needs to be restarted.
				alGetSourcei(uiSource, AL_SOURCE_STATE, &iState);
				if (iState != AL_PLAYING)
				{
					std::cout << "iState != AL_PLAYING : "<< iTotalBuffersProcessed << std::endl;
					// If there are Buffers in the Source Queue then the Source was starved of audio
					// data, so needs to be restarted (because there is more audio data to play)
					alGetSourcei(uiSource, AL_BUFFERS_QUEUED, &iQueuedBuffers);
					if (iQueuedBuffers)
					{
						alSourcePlay(uiSource);
						std::cout << "Queue "<< iTotalBuffersProcessed << std::endl;
					}
					else
					{
						// Finished playing
						return 1;
					}
				}
				return 0;
}
void Music::LoadAndStart(const char* filename)
{
	sCallbacks.read_func = ov_read_func;
	sCallbacks.seek_func = ov_seek_func;
	sCallbacks.close_func = ov_close_func;
	sCallbacks.tell_func = ov_tell_func;

// Open the OggVorbis file
	pOggVorbisFile = fopen(filename, "rb");
	if (!pOggVorbisFile)
	{
		return;
	}
	
	// Create an OggVorbis file stream
	if (ov_open_callbacks(pOggVorbisFile, &sOggVorbisFile, NULL, 0, sCallbacks) == 0)
	{
	// Get some information about the file (Channels, Format, and Frequency)
	psVorbisInfo = ov_info(&sOggVorbisFile, -1);
	if (psVorbisInfo)
	{
		ulFrequency = psVorbisInfo->rate;
		ulChannels = psVorbisInfo->channels;
		if (psVorbisInfo->channels == 1)
		{
			ulFormat = AL_FORMAT_MONO16;
			// Set BufferSize to 250ms (Frequency * 2 (16bit) divided by 4 (quarter of a second))
			ulBufferSize = ulFrequency >> 1;
			// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
			ulBufferSize -= (ulBufferSize % 2);
		}
		else if (psVorbisInfo->channels == 2)
		{
			ulFormat = AL_FORMAT_STEREO16;
			// Set BufferSize to 250ms (Frequency * 4 (16bit stereo) divided by 4 (quarter of a second))
			ulBufferSize = ulFrequency;
			// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
			ulBufferSize -= (ulBufferSize % 4);
		}
		else if (psVorbisInfo->channels == 4)
		{
			ulFormat = alGetEnumValue("AL_FORMAT_QUAD16");
			// Set BufferSize to 250ms (Frequency * 8 (16bit 4-channel) divided by 4 (quarter of a second))
			ulBufferSize = ulFrequency * 2;
			// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
			ulBufferSize -= (ulBufferSize % 8);
		}
		else if (psVorbisInfo->channels == 6)
		{
			ulFormat = alGetEnumValue("AL_FORMAT_51CHN16");
			// Set BufferSize to 250ms (Frequency * 12 (16bit 6-channel) divided by 4 (quarter of a second))
			ulBufferSize = ulFrequency * 3;
			// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
			ulBufferSize -= (ulBufferSize % 12);
		}
	}
	
	if (ulFormat != 0)
	{
		// Allocate a buffer to be used to store decoded data for all Buffers
		pDecodeBuffer = (char*)malloc(ulBufferSize);
		if (!pDecodeBuffer)
		{
			ov_clear(&sOggVorbisFile);
			return;
		}

		// Generate some AL Buffers for streaming
		alGenBuffers( NUMBUFFERS, uiBuffers );

		// Generate a Source to playback the Buffers
		alGenSources( 1, &uiSource );

		ALint iLoop;
		// Fill all the Buffers with decoded audio data from the OggVorbis file
		for (iLoop = 0; iLoop < NUMBUFFERS; iLoop++)
		{
			ulBytesWritten = DecodeOggVorbis(&sOggVorbisFile, pDecodeBuffer, ulBufferSize, ulChannels);
			if (ulBytesWritten)
			{
				alBufferData(uiBuffers[iLoop], ulFormat, pDecodeBuffer, ulBytesWritten, ulFrequency);
				alSourceQueueBuffers(uiSource, 1, &uiBuffers[iLoop]);
			}
		}
		// Start playing source
		alSourcePlay(uiSource);
		iTotalBuffersProcessed = 0;
	}
	}
};
class clOndeALVorbis : public Visible
{
	ALCdevice* device;
	ALCcontext* context;
	std::vector<SoundBuffer> soundbuffers;
	std::vector<Sound*> sounds;
	std::vector<Music*> musics;
public:
	void Start();
	void Loop();
	void Stop();
	void AddSound(const char* filename);
	void PlayMusic(const char* filename);
	int SoundCardsEnumeration();
	int Run(clOndeALVorbis &Me);
	void Play(uint32_t s,float* pos,float* vel);
	void Update();
	void Release();
	
	~clOndeALVorbis()
	{
	}
	clOndeALVorbis() :Visible(ocmclass_openAL)
	{
	}
};

//OpenAL error checking
#define OpenAL_ErrorCheck(message)\
{\
	ALenum error = alGetError();\
	if( error != AL_NO_ERROR)\
	{\
		std::cerr << "OpenAL Error: " << error << " with call for " << #message << std::endl;\
	}\
}

#define alec(FUNCTION_CALL)\
FUNCTION_CALL;\
OpenAL_ErrorCheck(FUNCTION_CALL)

struct StartSound
{
	uint32_t sound;
	float* _pos;
	float* _vel;
};

void clOndeALVorbis::Loop()
{
	bool LOOP = true;
	do
	{
		Update();
		OndeMsg x;
		ALfloat value;
		while(GetMsg(&x))
		{
			if(x.port == 888)
			{
				if(x.msg == msg_stop)LOOP = false;
				if(x.msg == msg_play)
				{
					uint32_t* pnum=(uint32_t*)x.ptr;
					Play(*pnum,nullptr,nullptr);
				}
				if(x.msg == msg_music)
				{
				PlayMusic((char*)x.ptr);
				}
			}
		}
	} while (LOOP);

};

void clOndeALVorbis::Release(){
		for(auto soundbuf : soundbuffers)soundbuf.Release();
}
	
void Sound::Update()
{
	if(_pos)alec(alSource3f(Source, AL_POSITION, _pos[0], _pos[1], _pos[2])); //NOTE: this does not work like mono sound positions!
	if(_vel)alec(alSource3f(Source, AL_VELOCITY, _vel[0], _vel[1], _vel[2])); 
}

void Sound::Release()
{
		alec(alDeleteSources(1, &Source));
}
	
void clOndeALVorbis::PlayMusic(const char* filename)
{
	Music* music = new Music;
	music->LoadAndStart(filename);
	musics.push_back(music);
}

void clOndeALVorbis::AddSound(const char* filename)
{
	SoundBuffer sound;
	sound.Load(filename);
	soundbuffers.push_back(sound);
}
    
void clOndeALVorbis::Start()
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// find the default audio device
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const ALCchar* defaultDeviceString = alcGetString(/*device*/nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
	device = alcOpenDevice(defaultDeviceString);
	if (!device)
	{
		std::cerr << "failed to get the default device for OpenAL" << std::endl;
		return;
	}
	std::cout << "OpenAL Device: " << alcGetString(device, ALC_DEVICE_SPECIFIER) << std::endl;
	//OpenAL_ErrorCheck(device);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create an OpenAL audio context from the device
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	context = alcCreateContext(device, /*attrlist*/ nullptr);
	//OpenAL_ErrorCheck(context);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Activate this context so that OpenAL state modifications are applied to the context
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (!alcMakeContextCurrent(context))
	{
		std::cerr << "failed to make the OpenAL context the current context" << std::endl;
		return;
	}
	//OpenAL_ErrorCheck("Make context current");


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create a listener in 3d space (ie the player); (there always exists as listener, you just configure data on it)
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	alec(alListener3f(AL_POSITION, 0.f, 0.f, 0.f));
	alec(alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f));
	ALfloat forwardAndUpVectors[] = {
		/*forward = */ 1.f, 0.f, 0.f,
		/* up = */ 0.f, 1.f, 0.f
	};
	alec(alListenerfv(AL_ORIENTATION, forwardAndUpVectors));
}

void clOndeALVorbis::Update()
{
	uint32_t remove_sound = sounds.size();
	uint32_t remove_music = musics.size();
	ALint sourceState;
	for(uint32_t i=0;i<sounds.size();i++)
	{
	//alec(alSourcePlay(sounds[i]->Source));
	alec(alGetSourcei(sounds[i]->Source, AL_SOURCE_STATE, &sourceState));
	if(sourceState == AL_PLAYING)
		sounds[i]->Update();
	else
		remove_sound = i;
	}
	for(uint32_t i=0;i<musics.size();i++)
	{
		if(musics[i]->Continue() == 1)
		{
			musics[i]->Release();
			remove_music = i;
		}
	}
	if(remove_sound < sounds.size())
		{
			sounds[remove_sound]->Release();
			delete sounds[remove_sound];
			sounds.erase(sounds.begin() + remove_sound);
		}
	if(remove_music < musics.size())
		{
			delete musics[remove_music];
			musics.erase(musics.begin() + remove_music);
		}
}

void clOndeALVorbis::Play(uint32_t s,float* pos,float* vel)
	{
		Sound* sound = new Sound;
		sound->_pos = pos;
		sound->_vel = vel;
		
		alec(alGenSources(1, &sound->Source));
		alec(alSourcef(sound->Source, AL_PITCH, 1.f));
		alec(alSourcef(sound->Source, AL_GAIN, 1.f));
		alec(alSourcei(sound->Source, AL_LOOPING, AL_FALSE));
		alec(alSourcei(sound->Source, AL_BUFFER, soundbuffers[s]));
		sound->Update();
		alec(alSourcePlay(sound->Source));
		sounds.push_back(sound);
	}	
	
void SoundBuffer::Load(const char* filename)
{
	drwav wav;
    if (!drwav_init_file(&wav, filename, NULL)) {
        return;
    }
	size_t size = wav.totalPCMFrameCount * wav.channels * sizeof(drwav_int16); 
	drwav_int16* pDecodedInterleavedPCMFrames = (drwav_int16*)malloc(size);
    size_t numberOfSamplesActuallyDecoded = drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, pDecodedInterleavedPCMFrames);
	
	alec(alGenBuffers(1, &SoundBuffer));
	alec(alBufferData(SoundBuffer, AL_FORMAT_STEREO16, pDecodedInterleavedPCMFrames, size, wav.sampleRate));
	
	drwav_uninit(&wav);
	free(pDecodedInterleavedPCMFrames);
};

void clOndeALVorbis::Stop()
{
	for (auto sound : sounds)
		{
			sound->Release();
			delete sound;
		}
	for (auto soundbuffer : soundbuffers)soundbuffer.Release();
	
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);
};

void SoundBuffer::Release()
{
	alec(alDeleteBuffers(1, &SoundBuffer));
};