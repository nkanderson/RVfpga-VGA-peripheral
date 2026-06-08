////////////////////////////////////////////////////////////////////////////////
// Engineer: Chris Kane-Pardy
// Create Date: 05/31/2026
// File Name: score.c
// Project Name: Note Feller
//
// Description:
//   Score tracking module for Note Feller. This module owns the player's score
//   state, including score value, combo count, and score multiplier. The module
//   updates the seven-segment display through the seven_segment API.
//
// Responsibilities:
//   - Track current score
//   - Track current combo
//   - Track adjustable score multiplier
//   - Increment score on successful hits
//   - Reset combo on misses
//   - Update seven-segment score display
////////////////////////////////////////////////////////////////////////////////

#include "score.h"
#include "seven_segment.h"
#include "vga_sprite.h"
#include "menu.h"

#define INDEX_X 0
#define INDEX_MULTIPLIER 1

static ScoreState score_state;

static uint32_t score_calculate_multiplier(uint32_t combo)
{
    if (combo >= 30) {
        return 4;
    }

    if (combo >= 20) {
        return 3;
    }

    if (combo >= 10) {
        return 2;
    }

    return 1;
}

void score_init(void)
{
    sevenseg_init();
    score_reset();
}

void score_reset(void)
{
    score_state.value = 0;
    score_state.combo = 0;
    score_state.multiplier = 1;
    for (int i = 0; i < NUMBER_COMBO_SPRITES; i++) {
        score_state.sprite[i].reg   = i + COMBO_SPRITE_OFFSET; // Example sprite register assignment
        score_state.sprite[i].color = VGA_BLACK; // Example color (yellow)
        score_state.sprite[i].pos_y  = 450;
    }
    //X
    score_state.sprite[INDEX_X].sprite_id   = SPRITE_FORM_X; // Example sprite form
    score_state.sprite[INDEX_X].sprite_type = VGA_SPRITE_16x16; // Example sprite type
    score_state.sprite[INDEX_X].pos_x       = 310; // Example position

    //NUMBER
    score_state.sprite[INDEX_MULTIPLIER].sprite_id   = SPRITE_FORM_1; // Example sprite form
    score_state.sprite[INDEX_MULTIPLIER].sprite_type = VGA_SPRITE_16x16; // Example sprite type
    score_state.sprite[INDEX_MULTIPLIER].pos_x       = 320; // Example position
    score_update_display();
}

void score_register_hit(void)
{
    if (score_state.combo < SCORE_COMBO_MAX) {
        score_state.combo++;
    }

    score_state.multiplier = score_calculate_multiplier(score_state.combo);

    score_state.value += SCORE_POINTS_PER_HIT * score_state.multiplier;

    score_update_display();
}

void score_register_miss(void)
{
    score_state.combo = 0;
    score_state.multiplier = 1;
    score_update_display();
}

uint32_t score_get_value(void)
{
    return score_state.value;
}

uint32_t score_get_combo(void)
{
    return score_state.combo;
}

uint32_t score_get_multiplier(void)
{
    return score_state.multiplier;
}

void score_set_multiplier(uint32_t multiplier)
{
    if (multiplier == 0) {
        multiplier = 1;
    }

    score_state.multiplier = multiplier;
}

const ScoreState* score_get_state(void)
{
    return &score_state;
}

void score_sprite_multiplier_decode(void) {
    switch (score_state.multiplier) {
        case 1:
            score_state.sprite[INDEX_MULTIPLIER].sprite_id = SPRITE_FORM_1;
            break;
        case 2:
            score_state.sprite[INDEX_MULTIPLIER].sprite_id = SPRITE_FORM_2;
            break;
        case 3:
            score_state.sprite[INDEX_MULTIPLIER].sprite_id = SPRITE_FORM_3;
            break;
        case 4:
            score_state.sprite[INDEX_MULTIPLIER].sprite_id = SPRITE_FORM_4 ;
            break;
        default:
            score_state.sprite[INDEX_MULTIPLIER].sprite_id = SPRITE_FORM_1;
            break;
    }
}

void score_update_display(void) {
    sevenseg_display_score(score_state.value);
    score_sprite_multiplier_decode();
    vga_set_sprite(&score_state.sprite[INDEX_X]);
    vga_set_sprite(&score_state.sprite[INDEX_MULTIPLIER]);
    menu_draw_score();
}