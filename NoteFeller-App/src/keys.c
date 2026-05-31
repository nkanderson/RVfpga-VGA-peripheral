////////////////////////////////////////////////////////////////////////////////
// Engineer:    Sajida Sayyad
// Create Date: 26/05/2026
// File Name:   keys.c
// Project Name: Single-Lane Rhythm Game
// Description:
//   Defines the keys (buttons) for the game and their properties.
////////////////////////////////////////////////////////////////////////////////

#include "types.h"  // Include the types header

// Define the keys for the game
Key keys[4] = {
    {  // Key 0 (BTNL)
        .led = 13,          // LED to blink for hit
        .button = 1 << 0,   // Button mask (INPUT_LANE_0)
        .audio_voice = 0,   // Audio voice index (C4)
        .score_led = 15     // LED to blink for score increment
    },
    {  // Key 1 (BTNR)
        .led = 11,          // LED to blink for hit
        .button = 1 << 1,   // Button mask (INPUT_LANE_1)
        .audio_voice = 1,   // Audio voice index (D4)
        .score_led = 15     // LED to blink for score increment
    },
    {  // Key 2 (BTNU)
        .led = 12,          // LED to blink for hit
        .button = 1 << 2,   // Button mask (INPUT_LANE_2)
        .audio_voice = 2,   // Audio voice index (E4)
        .score_led = 15     // LED to blink for score increment
    },
    {  // Key 3 (BTND)
        .led = 14,          // LED to blink for hit
        .button = 1 << 3,   // Button mask (INPUT_LANE_3)
        .audio_voice = 3,   // Audio voice index (F4)
        .score_led = 15     // LED to blink for score increment
    }
};