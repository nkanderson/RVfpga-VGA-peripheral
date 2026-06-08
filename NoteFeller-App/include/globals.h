////////////////////////////////////////////////////////////////////////////////
// Engineer:     Chris Kane-Pardy & Jacob Bertenshaw
// Create Date:  26/05/2026
// File Name:    globals.h
// Project Name: Note Feller
//
// Description:
//   Shared constants for Note Feller gameplay layout, sprite allocation,
//   lane colors, and screen dimensions.
////////////////////////////////////////////////////////////////////////////////

#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>

#include "vga_sprite.h"
#include "audio.h"
#include "input_controller.h"

// -----------------------------------------------------------------------------
// Game layout constants
// -----------------------------------------------------------------------------

#define NUMBER_INPUT_LANES 4
#define MAX_NOTES_IN_GAME  24

#define NUMBER_COMBO_SPRITES 2

#define SCREEN_W       640
#define SCREEN_H       480

#define KEY_LANE_W     32
#define KEY_SPRITE_H   32
#define KEY_LANE_START ((SCREEN_W - NUMBER_INPUT_LANES * KEY_LANE_W) / 2)
#define KEY_Y          400

// -----------------------------------------------------------------------------
// Lane positions
// -----------------------------------------------------------------------------

static const uint16_t lane_locations[NUMBER_INPUT_LANES] = {
    KEY_LANE_START + 0 * KEY_LANE_W,
    KEY_LANE_START + 1 * KEY_LANE_W,
    KEY_LANE_START + 2 * KEY_LANE_W,
    KEY_LANE_START + 3 * KEY_LANE_W,
};

// -----------------------------------------------------------------------------
// Sprite register allocation
// -----------------------------------------------------------------------------

#define KEY_SPRITE_OFFSET    0
#define NOTE_SPRITE_OFFSET   NUMBER_INPUT_LANES
#define COMBO_SPRITE_OFFSET  (NOTE_SPRITE_OFFSET + MAX_NOTES_IN_GAME)

// -----------------------------------------------------------------------------
// Lane colors
// -----------------------------------------------------------------------------

static const uint16_t lane_color_palette[NUMBER_INPUT_LANES] = {
    VGA_GREEN,
    VGA_RED,
    VGA_BLUE,
    VGA_YELLOW,
};

#endif // GLOBALS_H