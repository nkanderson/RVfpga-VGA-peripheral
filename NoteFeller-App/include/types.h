////////////////////////////////////////////////////////////////////////////////
// Engineer:    Sajida Sayyad
// Create Date: 26/05/2026
// File Name:   types.h
// Project Name: Single-Lane Rhythm Game
// Description:
//   Defines custom types (Key and Note) used in the game.
////////////////////////////////////////////////////////////////////////////////

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

// --- Key Structure ---
typedef struct {
    uint8_t led;          // LED to blink for this key
    uint32_t button;      // Button mask for this key
    uint8_t audio_voice;  // Audio voice to play when key is pressed
    uint8_t score_led;    // LED to blink for score increment
} Key;

// --- Note Structure ---
typedef struct {
    uint8_t active;       // 1 if the note is active, 0 otherwise
    uint8_t y;            // Current position of the note (LED index)
    uint8_t lane;         // Lane the note belongs to (0-3)
} Note;

#endif // TYPES_H