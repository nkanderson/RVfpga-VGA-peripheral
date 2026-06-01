////////////////////////////////////////////////////////////////////////////////
// Engineer:    Sajida Sayyad
// Create Date: 26/05/2026
// File Name:   keys.c
// Project Name: Single-Lane Rhythm Game
// Description:
//   Defines the keys (buttons) for the game and their properties.
////////////////////////////////////////////////////////////////////////////////

#include "key.h"  // Include the types header

void key_init(Key* key, uint32_t button_mask, uint8_t  audio_voice, uint8_t sprite_id, uint8_t sprite_type, uint16_t color, uint16_t pos_x, uint16_t pos_y) {
    key->button = button_mask;
    key->audio_voice = audio_voice;
    key->sprite.sprite_id = sprite_id;
    key->sprite.sprite_type = sprite_type;
    key->sprite.color = color;
    key->sprite.pos_x = pos_x;
    key->sprite.pos_y = pos_y;
}

void key_init_keys(void) {
    uint32_t key_colors[NUMBER_INPUT_LANES] = {
        VGA_COLOR_GREEN,   // Lane 0
        VGA_COLOR_RED,     // Lane 1
        VGA_COLOR_BLUE,    // Lane 2
        VGA_COLOR_YELLOW   // Lane 3
    };

    for (int i = 0; i < NUMBER_INPUT_LANES; i++) {
        key_init(&keys[i], INPUT_LANE_0 << i, AUDIO_VOICE_E4 + i, SPRITE_FORM_NOTE_CIRCLE_SOLID, VGA_SPRITE_16x16, key_colors[i], 100 + i * 100, 400);
    }

    // Initialize all keys with their respective properties
    key_init(&keys[0], INPUT_LANE_0, AUDIO_VOICE_E4, SPRITE_FORM_NOTE_CIRCLE_SOLID, VGA_SPRITE_16x16, VGA_COLOR_GREEN);
    key_init(&keys[1], INPUT_LANE_1, AUDIO_VOICE_C4, SPRITE_FORM_NOTE_CIRCLE_SOLID, VGA_SPRITE_16x16, VGA_COLOR_RED);
    key_init(&keys[2], INPUT_LANE_2, AUDIO_VOICE_D4, SPRITE_FORM_NOTE_CIRCLE_SOLID, VGA_SPRITE_16x16, VGA_COLOR_BLUE);
    key_init(&keys[3], INPUT_LANE_3, AUDIO_VOICE_F4, SPRITE_FORM_NOTE_CIRCLE_SOLID, VGA_SPRITE_16x16, VGA_COLOR_YELLOW);
}
