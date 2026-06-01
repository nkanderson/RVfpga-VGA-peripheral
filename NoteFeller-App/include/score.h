#ifndef SCORE_H
#define SCORE_H

#include <stdint.h>

#define SCORE_POINTS_PER_HIT 100
#define SCORE_COMBO_MAX      9999

typedef struct {
    uint32_t value;
    uint32_t combo;
    uint32_t multiplier;
} ScoreState;

void score_init(void);
void score_reset(void);

void score_register_hit(void);
void score_register_miss(void);

uint32_t score_get_value(void);
uint32_t score_get_combo(void);
uint32_t score_get_multiplier(void);

void score_set_multiplier(uint32_t multiplier);

const ScoreState* score_get_state(void);

void score_update_display(void);

#endif