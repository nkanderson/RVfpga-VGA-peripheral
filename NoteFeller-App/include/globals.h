////////////////////////////////////////////////////////////////////////////////
// Engineer:    Sajida Sayyad
// Create Date: 26/05/2026
// File Name:   globals.h
// Project Name: Single-Lane Rhythm Game
// Description:
//   Declares shared game state and constants. Ensures no duplicate definitions.
////////////////////////////////////////////////////////////////////////////////

#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>
#include "audio.h"            // For audio control (AUDIO_VOICE_*)
#include "input_controller.h" // For input button masks (INPUT_LANE_*)

// --- Game Constants ---
#define NOTE_SPEED      500000  // Delay between LED updates (higher = slower)
#define HIT_LED_DURATION 300000 // Duration for hit LED blink
#define MAX_LED_POSITION 10     // Note resets after reaching this LED

#define NUMBER_INPUT_LANES 4       // Total number of input lanes (buttons)

// --- Game State (Declarations Only) ---
extern int game_started;       // 1 if the game is running, 0 otherwise
extern int note_position;      // Current LED position of the note (0-10)
extern int note_active;        // 1 if a note is active, 0 otherwise

#endif // GLOBALS_H