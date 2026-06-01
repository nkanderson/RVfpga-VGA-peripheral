////////////////////////////////////////////////////////////////////////////////
// Engineer:    Sajida Sayyad
// Create Date: 29/05/2026
// File Name:   keys.h
// Project Name: Guitar Hero FPGA
// Description:
//   Header file for lane/chord management. Defines the `Key` structure and
//   functions to initialize and retrieve lane data.
// AI Contributions: Copilot added key lane layout constants.
////////////////////////////////////////////////////////////////////////////////

#ifndef KEYS_H
#define KEYS_H

#include <stdint.h>
#include "globals.h"  // For NUMBER_INPUT_LANES
#include "vga_sprite.h"

typedef struct {
    uint32_t button;       // Button mask for this key
    uint8_t  audio_voice;  // Audio voice to play when key is pressed
    Sprite   sprite;
    int      reg;         // Sprite register index for this key
} Key;

static Key keys[NUMBER_INPUT_LANES];

void key_init(Key* key, uint32_t button_mask, uint8_t audio_voice, Sprite sprite);

void key_init_keys(void);

void key_update_sprite_color(Key* key, uint16_t new_color);

void key_update_sprite_form(Key* key, uint8_t new_sprite_id, uint8_t new_sprite_type);

#endif // KEYS_H