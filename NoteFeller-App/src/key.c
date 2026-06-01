////////////////////////////////////////////////////////////////////////////////
// Engineer:    Sajida Sayyad
// Create Date: 26/05/2026
// File Name:   keys.c
// Project Name: Single-Lane Rhythm Game
// Description:
//   Defines the keys (buttons) for the game and their properties.
// AI Contributions: Copilot updated key x-position to be dynamically centered
//                   based on NUMBER_INPUT_LANES using the same lane-centering
//                   formula as the VGA Sprite Test reference.
////////////////////////////////////////////////////////////////////////////////

#include "key.h"

void key_init(Key* key, uint32_t button_mask, uint8_t audio_voice, Sprite sprite) {
    key->button             = button_mask;
    key->audio_voice        = audio_voice;
    key->sprite.reg         = sprite.reg;
    key->sprite.sprite_id   = sprite.sprite_id;
    key->sprite.sprite_type = sprite.sprite_type;
    key->sprite.color       = sprite.color;
    key->sprite.pos_x       = sprite.pos_x;
    key->sprite.pos_y       = sprite.pos_y;
}

void key_init_keys(void) {
    uint16_t key_colors[NUMBER_INPUT_LANES] = {
        VGA_GREEN,   // Lane 0
        VGA_RED,     // Lane 1
        VGA_BLUE,    // Lane 2
        VGA_YELLOW   // Lane 3
    };

    uint16_t key_locations[NUMBER_INPUT_LANES];
    for (int i = 0; i < NUMBER_INPUT_LANES; i++) {
        key_locations[i] = KEY_LANE_START + i * KEY_LANE_W;
    }

    for (int i = 0; i < NUMBER_INPUT_LANES; i++) {
        Sprite sprite;
        sprite.reg         = i + KEY_SPRITE_OFFSET;  // Assign sprite register index
        sprite.sprite_id   = SPRITE_FORM_CHORD_CIRCLE_SOLID;
        sprite.sprite_type = VGA_SPRITE_32x32;
        sprite.color       = key_colors[i];
        sprite.pos_x       = key_locations[i];
        sprite.pos_y       = KEY_Y;
        key_init(&keys[i], INPUT_LANE_0 << i, AUDIO_VOICE_E4 + i, sprite);
    }
}

void key_update_sprite_color(Key* key, uint16_t new_color){
    key->sprite.color = new_color;
}

void key_update_sprite_form(Key* key, uint8_t new_sprite_id, uint8_t new_sprite_type) {
    key->sprite.sprite_id = new_sprite_id;
    key->sprite.sprite_type = new_sprite_type;
}
