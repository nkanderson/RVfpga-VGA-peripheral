////////////////////////////////////////////////////////////////////////////////
// Engineer:     Chris Kane-Pardy
// Create Date:  05/31/2026
// File Name:    key.c
// Project Name: Note Feller
//
// Description:
//   Lane/key management for the four gameplay lanes.
//   Enter / INPUT_LANE_4 is reserved for menu/start control, not gameplay.
////////////////////////////////////////////////////////////////////////////////

#include "key.h"

Key keys[NUMBER_INPUT_LANES];

void key_init(Key* key, uint32_t button_mask, uint8_t audio_voice, Sprite sprite)
{
    key->button      = button_mask;
    key->audio_voice = audio_voice;
    key->sprite      = sprite;
}

void key_init_keys(void)
{
    for (int i = 0; i < NUMBER_INPUT_LANES; i++) {
        Sprite sprite;

        sprite.reg         = KEY_SPRITE_OFFSET + i;
        sprite.sprite_id   = SPRITE_FORM_CHORD_CIRCLE_HOLLOW;
        sprite.sprite_type = VGA_SPRITE_32x32;
        sprite.color       = lane_color_palette[i];
        sprite.pos_x       = lane_locations[i];
        sprite.pos_y       = KEY_Y;

        key_init(
            &keys[i],
            INPUT_LANE_0 << i,
            AUDIO_VOICE_C4 + i,
            sprite
        );
    }
}

void key_draw(Key* key)
{
    vga_set_sprite(&key->sprite);
}

void key_draw_all(void)
{
    for (int i = 0; i < NUMBER_INPUT_LANES; i++) {
        key_draw(&keys[i]);
    }
}

void key_update_sprite(uint8_t key_index,
                       uint8_t new_sprite_id,
                       uint8_t new_sprite_type,
                       uint16_t new_color)
{
    keys[key_index].sprite.color       = new_color;
    keys[key_index].sprite.sprite_id   = new_sprite_id;
    keys[key_index].sprite.sprite_type = new_sprite_type;

    key_draw(&keys[key_index]);
}