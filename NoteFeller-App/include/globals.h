////////////////////////////////////////////////////////////////////////////////
// Engineer:     Sajida Sayyad
// Create Date:  26/05/2026
// File Name:    globals.h
// Project Name: Single-Lane Rhythm Game
// Description:
//   Declares shared game state and constants. Ensures no duplicate definitions.
////////////////////////////////////////////////////////////////////////////////

#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>
#include "vga_sprite.h"        // For Sprite struct and VGA_COLOR macro
#include "audio.h"            // For audio control (AUDIO_VOICE_*)
#include "input_controller.h" // For input button masks (INPUT_LANE_*)

// --- Game Constants ---
#define NOTE_SPEED      500000  // Delay between LED updates (higher = slower)
#define HIT_LED_DURATION 300000 // Duration for hit LED blink
#define MAX_LED_POSITION 10     // Note resets after reaching this LED

// Key lane layout: lanes are centered as a group on the 640px screen.
// KEY_LANE_START is the x pixel of the left edge of lane 0.
// Each lane is KEY_LANE_W pixels wide, so lane i starts at KEY_LANE_START + i * KEY_LANE_W.
// KEY_Y is the fixed y position of all key sprites (hit zone row).
#define NUMBER_INPUT_LANES 4       // Total number of input lanes (buttons)
#define MAX_NOTES_IN_GAME 12          // Maximum number of notes that can be active at once

#define SCREEN_W       640
#define SCREEN_H       480
#define KEY_LANE_W     32    // One lane = one 32x32 sprite width
#define KEY_SPRITE_H   32    // Height of the key (hit box) sprite in pixels
#define KEY_LANE_START ((SCREEN_W - NUMBER_INPUT_LANES * KEY_LANE_W) / 2)
#define KEY_Y          400

// Pixel x-coordinate of the left edge of each lane. Supports up to 4 lanes.
static const uint16_t lane_locations[4] = {
    KEY_LANE_START + 0 * KEY_LANE_W,
    KEY_LANE_START + 1 * KEY_LANE_W,
    KEY_LANE_START + 2 * KEY_LANE_W,
    KEY_LANE_START + 3 * KEY_LANE_W,
};

// SPRITE OFFSETS
#define KEY_SPRITE_OFFSET  0                  // Offset for sprite register indices
#define NOTE_SPRITE_OFFSET NUMBER_INPUT_LANES // Offset for note sprite register indices


static const uint16_t lane_color_palette[NUMBER_INPUT_LANES] = {
    VGA_GREEN,    // Lane 0
    VGA_RED,      // Lane 1
    VGA_BLUE,     // Lane 2
    VGA_YELLOW,   // Lane 3
};

// --- Game State (Declarations Only) ---
extern int game_started;       // 1 if the game is running, 0 otherwise
extern int note_position;      // Current LED position of the note (0-10)
extern int note_active;        // 1 if a note is active, 0 otherwise

#endif // GLOBALS_H