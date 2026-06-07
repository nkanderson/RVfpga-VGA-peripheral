#ifndef KEY_H
#define KEY_H

#include <stdint.h>
#include <stdbool.h>

#include "globals.h"
#include "vga_sprite.h"

typedef struct {
    uint32_t button;
    uint8_t  audio_voice;
    Sprite   sprite;
    bool     hittable;
    bool     hit_latched;
} Key;

extern Key keys[NUMBER_INPUT_LANES];

void key_init(Key* key, uint32_t button_mask, uint8_t audio_voice, Sprite sprite);
void key_init_keys(void);

void key_draw(Key* key);
void key_draw_all(void);

void key_set_hittable(Key* key, bool hittable);
bool key_try_hit(Key* key, uint32_t input_edges);

void key_update_sprite(uint8_t key_index,
                       uint8_t new_sprite_id,
                       uint8_t new_sprite_type,
                       uint16_t new_color);

#endif