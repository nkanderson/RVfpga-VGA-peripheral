////////////////////////////////////////////////////////////////////////////////
// Engineer:    Jacob Burtenshaw
// Create Date: 26/05/2026
// File Name:   note.c
// Project Name: Note Feller
//
// Description:
//   Handles note spawning, fixed-speed note movement, hit detection,
//   and missed-note reporting.
////////////////////////////////////////////////////////////////////////////////

#include "note.h"
#include "globals.h"

// -----------------------------------------------------------------------------
// Sprite dimensions
// -----------------------------------------------------------------------------

#define SPRITE_BIG 32

// -----------------------------------------------------------------------------
// Fixed note timing
// -----------------------------------------------------------------------------
// Lower TICK_THRESHOLD = faster movement.
// Higher INCREMENT_Y   = larger movement step per update.

#define TICK_THRESHOLD 500u
#define INCREMENT_Y    3u

// -----------------------------------------------------------------------------
// Fixed spawn timing
// -----------------------------------------------------------------------------
// Each lane independently waits a randomized number of calls in the range:
// [SPAWN_THRESHOLD_BASE, SPAWN_THRESHOLD_BASE + SPAWN_THRESHOLD_RANGE)

#define SPAWN_THRESHOLD_BASE  18000u
#define SPAWN_THRESHOLD_RANGE 10000u

#define MAX_ACTIVE_PER_LANE NOTES_PER_LANE

// -----------------------------------------------------------------------------
// Note storage
// -----------------------------------------------------------------------------

static Note notes[NUMBER_INPUT_LANES][NOTES_PER_LANE];

// -----------------------------------------------------------------------------
// Per-lane spawn state
// -----------------------------------------------------------------------------

static uint32_t spawn_counters[NUMBER_INPUT_LANES];
static uint32_t spawn_thresholds[NUMBER_INPUT_LANES];

// -----------------------------------------------------------------------------
// PRNG state
// -----------------------------------------------------------------------------

static uint32_t lcg_state;

static uint32_t lcg_rand(void)
{
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return lcg_state >> 16;
}

static uint32_t rand_threshold(void)
{
    return SPAWN_THRESHOLD_BASE + (lcg_rand() % SPAWN_THRESHOLD_RANGE);
}

// -----------------------------------------------------------------------------
// Initialization
// -----------------------------------------------------------------------------

void note_init(Note *note, uint8_t reg, uint8_t lane)
{
    Sprite sprite;

    sprite.reg         = reg;
    sprite.sprite_id   = SPRITE_FORM_CHORD_CIRCLE_SOLID;
    sprite.sprite_type = VGA_SPRITE_32x32;
    sprite.color       = lane_color_palette[lane];
    sprite.pos_x       = lane_locations[lane] + (KEY_LANE_W - SPRITE_BIG) / 2;
    sprite.pos_y       = 0;

    note->active   = 0;
    note->hittable = false;
    note->y        = 0;
    note->tick_ctr = 0;
    note->sprite   = sprite;

    vga_clear_sprite(note->sprite.reg);
}

void note_init_notes(void)
{
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        for (int i = 0; i < NOTES_PER_LANE; i++) {
            uint8_t reg = (uint8_t)((lane * NOTES_PER_LANE) + i + NOTE_SPRITE_OFFSET);
            note_init(&notes[lane][i], reg, (uint8_t)lane);
        }
    }

    // Seed the LCG from build timestamp to avoid CSR dependency.
    // __TIME__ expands to "HH:MM:SS".
    const char *t = __TIME__;

    lcg_state = ((uint32_t)(t[0]) * 100u + (uint32_t)(t[1]) * 10u +
                 (uint32_t)(t[3]) * 100u + (uint32_t)(t[4]) * 10u +
                 (uint32_t)(t[6]) * 10u  + (uint32_t)(t[7])) * 2654435761u;

    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        spawn_thresholds[lane] = rand_threshold();
        spawn_counters[lane]   = lcg_rand() % spawn_thresholds[lane];
    }
}

// -----------------------------------------------------------------------------
// Hit/miss checks
// -----------------------------------------------------------------------------

bool note_complete(uint16_t y, uint8_t sprite_height)
{
    return y + sprite_height > SCREEN_H;
}

bool note_hittable_check(uint16_t y, uint8_t sprite_height)
{
    uint16_t note_center = y + (sprite_height / 2);

    uint16_t hit_top    = KEY_Y;
    uint16_t hit_bottom = KEY_Y + KEY_SPRITE_H;

    return (note_center >= hit_top) && (note_center <= hit_bottom);
}

// -----------------------------------------------------------------------------
// Movement
// -----------------------------------------------------------------------------

uint32_t note_movement_routine(void)
{
    uint32_t missed_mask = 0;

    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        for (int i = 0; i < NOTES_PER_LANE; i++) {
            Note *note = &notes[lane][i];

            if (!note->active) {
                continue;
            }

            if (note->tick_ctr < TICK_THRESHOLD) {
                note->tick_ctr++;
                continue;
            }

            note->tick_ctr = 0;
            note->y += INCREMENT_Y;
            note->sprite.pos_y = note->y;

            vga_set_sprite(&note->sprite);

            note->hittable = note_hittable_check(note->y, SPRITE_BIG);

            if (note_complete(note->y, SPRITE_BIG)) {
                note->active   = 0;
                note->hittable = false;

                vga_clear_sprite(note->sprite.reg);

                missed_mask |= (1u << lane);
            }
        }
    }

    return missed_mask;
}

// -----------------------------------------------------------------------------
// Hit processing
// -----------------------------------------------------------------------------

bool note_process_hit(int lane)
{
    for (int i = 0; i < NOTES_PER_LANE; i++) {
        Note *note = &notes[lane][i];

        if (note->active && note->hittable) {
            note->active   = 0;
            note->hittable = false;

            vga_clear_sprite(note->sprite.reg);

            return true;
        }
    }

    return false;
}

// -----------------------------------------------------------------------------
// Spawning
// -----------------------------------------------------------------------------

static uint8_t note_count_active_in_lane(int lane)
{
    uint8_t count = 0;

    for (int i = 0; i < NOTES_PER_LANE; i++) {
        if (notes[lane][i].active) {
            count++;
        }
    }

    return count;
}

void note_spawn_routine(void)
{
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        if (spawn_counters[lane] < spawn_thresholds[lane]) {
            spawn_counters[lane]++;
            continue;
        }

        if (note_count_active_in_lane(lane) >= MAX_ACTIVE_PER_LANE) {
            spawn_counters[lane] = 0;
            spawn_thresholds[lane] = rand_threshold();
            continue;
        }

        int slot = -1;

        for (int i = 0; i < NOTES_PER_LANE; i++) {
            if (!notes[lane][i].active) {
                slot = i;
                break;
            }
        }

        if (slot < 0) {
            spawn_counters[lane] = 0;
            spawn_thresholds[lane] = rand_threshold();
            continue;
        }

        Note *note = &notes[lane][slot];

        note->active       = 1;
        note->hittable     = false;
        note->y            = 0;
        note->tick_ctr     = 0;
        note->sprite.pos_y = 0;

        vga_set_sprite(&note->sprite);

        spawn_counters[lane] = 0;
        spawn_thresholds[lane] = rand_threshold();
    }
}