////////////////////////////////////////////////////////////////////////////////
// Engineer: Niklas Anderson
// Create Date: 05/26/2026
// File Name: audio.h
// Project Name: Note Feller
//
// Description:
//   Header file for the Note Feller audio software driver. This driver
//   provides a clean game-facing API for the memory-mapped wb_audio
//   peripheral, supporting 8-voice polyphony (C4..C5) with per-voice
//   volume control.
//
//   Each of the 8 voices plays its own fixed scale note and can be turned
//   on or off independently. Multiple voices playing simultaneously
//   produce a chord; per-voice volume control allows musical balance
//   within the chord (e.g. emphasizing the root or top of a triad).
//
// Register Base:
//   0x80004000
//
// LLM Acknowledgment: This code was generated with the assistance of a
// language model, which provided the initial structure and logic.The
// final implementation was reviewed and edited by the author to ensure
// correctness and functionality on the target hardware platform.
////////////////////////////////////////////////////////////////////////////////

#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

#define AUDIO_NUM_VOICES 8

// Voice indices correspond to scale notes (C4 .. C5)
#define AUDIO_VOICE_C4 0
#define AUDIO_VOICE_D4 1
#define AUDIO_VOICE_E4 2
#define AUDIO_VOICE_F4 3
#define AUDIO_VOICE_G4 4
#define AUDIO_VOICE_A4 5
#define AUDIO_VOICE_B4 6
#define AUDIO_VOICE_C5 7

#define AUDIO_VOL_MAX 15
#define AUDIO_VOL_MIN 0

// Initialize the peripheral: silence all voices, set every voice's volume
// to default_voice_volume (0..15), then enable the amp.
// Call once at program start.
void audio_init(uint8_t default_voice_volume);

// Stop all voices. Amp stays enabled.
void audio_silence(void);

// Set all 8 voice on/off states in one write. Bit i = voice i playing.
void audio_set_voices(uint8_t voice_mask);

// Read back the current voice mask.
uint8_t audio_get_voices(void);

// Toggle a single voice on/off (read-modify-write).
void audio_set_voice(uint8_t voice_idx, int on);

// Set one voice's volume (read-modify-write of the packed VOL register).
void audio_set_voice_volume(uint8_t voice_idx, uint8_t volume);

// Set all 8 voice volumes in one write. `volumes` must be length 8.
void audio_set_all_volumes(const uint8_t volumes[AUDIO_NUM_VOICES]);

#endif // AUDIO_H
