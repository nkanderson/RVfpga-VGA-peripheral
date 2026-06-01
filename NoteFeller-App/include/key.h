////////////////////////////////////////////////////////////////////////////////
// Engineer:    Sajida Sayyad
// Create Date: 29/05/2026
// File Name:   keys.h
// Project Name: Guitar Hero FPGA
// Description:
//   Header file for lane/chord management. Defines the `Key` structure and
//   functions to initialize and retrieve lane data.
////////////////////////////////////////////////////////////////////////////////

#ifndef KEYS_H
#define KEYS_H

#include <stdint.h>
#include "globals.h"  // For NUMBER_INPUT_LANES
#include "vga_sprite.h"
#include "colors.h"

typedef struct {
    uint32_t button;       // Button mask for this key
    uint8_t  audio_voice;  // Audio voice to play when key is pressed
    Sprite   sprite;
} Key;

static Key keys[NUMBER_INPUT_LANES];  // Array of 4 keys (lanes)

// Initializes the lane data (LEDs, buttons, audio tones, score LEDs).
void key_init(Key* key, uint32_t button_mask, uint8_t  audio_voice, uint8_t sprite_id, uint8_t sprite_type, uint16_t color, uint16_t pos_x, uint16_t pos_y);

void key_init_keys(void);  // Initializes all keys with predefined properties

#endif // KEYS_H