#include "audio.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Internal state
// ---------------------------------------------------------------------------

typedef struct
{
	uint8_t* data;
	uint32_t size;
	SDL_AudioSpec spec;
} sound;

typedef struct
{
	SDL_AudioStream* stream;
	bool active;
} channel;

static struct
{
	SDL_AudioDeviceID device;
	sound sounds[AUDIO_MAX_SOUNDS];
	uint32_t sound_count;
	channel channels[AUDIO_MAX_CHANNELS];
	float master_volume;
	bool initialized;
} s;

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool audio_init(void)
{
	if (s.initialized) return true;

	if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
	{
		fprintf(stderr, "[audio] SDL audio init failed: %s\n", SDL_GetError());
		return false;
	}

	s.device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
	if (!s.device)
	{
		fprintf(stderr, "[audio] Failed to open audio device: %s\n", SDL_GetError());
		return false;
	}

	s.master_volume = 1.0f;
	s.initialized = true;
	fprintf(stderr, "[audio] Audio initialized\n");
	return true;
}

void audio_shutdown(void)
{
	if (!s.initialized) return;

	audio_stop_all();

	for (uint32_t i = 0; i < s.sound_count; i++)
		SDL_free(s.sounds[i].data);

	if (s.device)
		SDL_CloseAudioDevice(s.device);

	s.device = 0;
	s.sound_count = 0;
	s.initialized = false;
}

uint32_t audio_load_wav(const uint8_t* wav_data, uint32_t wav_size)
{
	if (!s.initialized || s.sound_count >= AUDIO_MAX_SOUNDS)
		return UINT32_MAX;

	SDL_IOStream* io = SDL_IOFromMem((void*)wav_data, wav_size);
	if (!io) return UINT32_MAX;

	SDL_AudioSpec spec;
	uint8_t* buf = NULL;
	uint32_t len = 0;

	if (!SDL_LoadWAV_IO(io, true, &spec, &buf, &len))
	{
		fprintf(stderr, "[audio] Failed to load WAV: %s\n", SDL_GetError());
		return UINT32_MAX;
	}

	uint32_t idx = s.sound_count++;
	s.sounds[idx].data = buf;
	s.sounds[idx].size = len;
	s.sounds[idx].spec = spec;
	return idx;
}

bool audio_play(uint32_t sound_handle, float volume)
{
	if (!s.initialized || sound_handle >= s.sound_count)
		return false;

	// Clean up finished channels
	for (int i = 0; i < AUDIO_MAX_CHANNELS; i++)
	{
		if (s.channels[i].active && s.channels[i].stream)
		{
			int queued = SDL_GetAudioStreamQueued(s.channels[i].stream);
			if (queued <= 0)
			{
				SDL_DestroyAudioStream(s.channels[i].stream);
				s.channels[i].stream = NULL;
				s.channels[i].active = false;
			}
		}
	}

	// Find free channel
	int ch = -1;
	for (int i = 0; i < AUDIO_MAX_CHANNELS; i++)
	{
		if (!s.channels[i].active) { ch = i; break; }
	}
	if (ch < 0) return false; // all channels busy

	sound* snd = &s.sounds[sound_handle];

	// Create stream and bind to output device
	SDL_AudioStream* stream = SDL_CreateAudioStream(&snd->spec, NULL);
	if (!stream) return false;

	if (!SDL_BindAudioStream(s.device, stream))
	{
		SDL_DestroyAudioStream(stream);
		return false;
	}

	// Set volume
	SDL_SetAudioStreamGain(stream, volume * s.master_volume);

	// Queue the sound data
	SDL_PutAudioStreamData(stream, snd->data, (int)snd->size);
	SDL_FlushAudioStream(stream);

	s.channels[ch].stream = stream;
	s.channels[ch].active = true;
	return true;
}

void audio_stop_all(void)
{
	for (int i = 0; i < AUDIO_MAX_CHANNELS; i++)
	{
		if (s.channels[i].stream)
		{
			SDL_DestroyAudioStream(s.channels[i].stream);
			s.channels[i].stream = NULL;
		}
		s.channels[i].active = false;
	}
}

void audio_set_volume(float volume)
{
	s.master_volume = volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume);
}
