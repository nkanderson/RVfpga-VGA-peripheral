#ifndef KEY_H
#define KEY_H

#include <stdint.h>

#include "input_controller.h"
#include "audio.h"
#include "vga_sprite.h"

#define NUMBER_INPUT_LANES 4

typedef struct {
    uint32_t button;
    uint8_t  audio_voice;
    Sprite   sprite;
    uint8_t  reg;
    uint8_t  hittable;
    uint8_t  hit_latched;
} Key;

extern Key keys[NUMBER_INPUT_LANES];

void key_init(Key* key, uint32_t button_mask, uint8_t audio_voice, Sprite sprite);
void key_init_keys(void);

void key_draw(Key* key);
void key_draw_all(void);

void key_set_hittable(Key* key, uint8_t hittable);
uint8_t key_try_hit(Key* key, uint32_t input_edges);

void key_update_sprite_color(Key* key, uint16_t new_color);
void key_update_sprite_form(Key* key, uint8_t new_sprite_id, uint8_t new_sprite_type);

#endif