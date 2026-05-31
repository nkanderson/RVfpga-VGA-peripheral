////////////////////////////////////////////////////////////////////////////////
// Engineer:    Sajida Sayyad
// Create Date: 26/05/2026
// File Name:   notes.c
// Project Name: Single-Lane Rhythm Game
// Description:
//   Handles note spawning, movement, and hit detection.
////////////////////////////////////////////////////////////////////////////////

#include "types.h"          // Include the types header
#include "input_controller.h" // For input button masks
#include "audio.h"          // For audio control
#include "gpio.h"           // For GPIO control

// Define constants for note movement
#define SCREEN_BOTTOM 10    // Bottom LED position
#define HIT_LINE_Y 4        // LED position where hits are detected

// Initialize a note
void note_init(Note *note, uint8_t lane) {
    note->active = 1;       // Activate the note
    note->y = 0;            // Start at the top (LED 0)
    note->lane = lane;      // Set the lane
}

// Update note position
void note_update(Note *note) {
    if (note->active) {
        note->y++;          // Move the note down

        // Deactivate if note reaches the bottom
        if (note->y > SCREEN_BOTTOM) {
            note->active = 0;
        }
    }
}

// Check if a note is hit
uint8_t note_check_hit(Note *note, Key *key) {
    if (note->active && note->y == HIT_LINE_Y) {
        uint32_t input = input_poll_new_presses();
        if (input & key->button) {
            // Play audio and blink LEDs
            audio_silence();
            audio_set_voice(key->audio_voice, 1);
            gpio_set_led(key->led, 1);
            gpio_set_led(key->score_led, 1);

            // Deactivate the note
            note->active = 0;
            return 1;       // Hit detected
        }
    }
    return 0;               // No hit
}