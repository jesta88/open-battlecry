#pragma once

#include <stdbool.h>
#include <stdint.h>

enum
{
	AUDIO_MAX_SOUNDS   = 128,
	AUDIO_MAX_CHANNELS = 16,
};

// Initialize the audio subsystem. Call once at startup.
bool audio_init(void);

// Shutdown and free all resources.
void audio_shutdown(void);

// Load a WAV from raw bytes in memory (e.g., from XCR resource).
// Returns a sound handle (index), or UINT32_MAX on failure.
uint32_t audio_load_wav(const uint8_t* wav_data, uint32_t wav_size);

// Play a loaded sound. volume: 0.0 to 1.0.
bool audio_play(uint32_t sound_handle, float volume);

// Stop all currently playing sounds.
void audio_stop_all(void);

// Set master volume (0.0 to 1.0).
void audio_set_volume(float volume);
