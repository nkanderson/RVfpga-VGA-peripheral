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

static ScoreState score_state;

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

    score_update_display();
}

void score_register_hit(void)
{
    if (score_state.combo < SCORE_COMBO_MAX) {
        score_state.combo++;
    }

    score_state.value += SCORE_POINTS_PER_HIT *
                         score_state.combo *
                         score_state.multiplier;

    score_update_display();
}

void score_register_miss(void)
{
    score_state.combo = 0;
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

void score_update_display(void)
{
    sevenseg_display_score(score_state.value);
}