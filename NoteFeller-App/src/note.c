////////////////////////////////////////////////////////////////////////////////
// Engineer:    Jacob Burtenshaw
// Create Date: 26/05/2026
// File Name:   note.c
// Project Name: Note Feller
// Description:
//   Handles note spawning, movement, and hit detection.
////////////////////////////////////////////////////////////////////////////////

#include "note.h"
#include "globals.h"       

// Define constants for note movement
#define TICK_THRESHOLD 100000 // Number of ticks before moving the note down
#define INCREMENT_Y 2

#define SPRITE_SMALL   16    // Height/width of a VGA_SPRITE_16x16 sprite in pixels
#define SPRITE_BIG     32    // Height/width of a VGA_SPRITE_32x32 sprite in pixels

// Spawn timing: each lane independently waits a random number of routine calls
// (in the range [BASE, BASE+RANGE)) before it becomes eligible to emit a note.
#define SPAWN_THRESHOLD_BASE  200u
#define SPAWN_THRESHOLD_RANGE 150u

// Note array — private to this file; use the note_* API from outside.
static Note notes[NUMBER_INPUT_LANES][NOTES_PER_LANE];

// Per-lane spawn state
static uint16_t spawn_counters[NUMBER_INPUT_LANES];   // calls since last spawn for each lane
static uint16_t spawn_thresholds[NUMBER_INPUT_LANES]; // randomized target calls before next spawn

// LCG PRNG (Knuth multiplicative coefficients)
static uint32_t lcg_state;

static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return lcg_state >> 16;
}

static uint16_t rand_threshold(void) {
    return (uint16_t)(SPAWN_THRESHOLD_BASE + (lcg_rand() % SPAWN_THRESHOLD_RANGE));
}

// Initialize a note
void note_init(Note *note, uint8_t reg, uint8_t lane) {
    Sprite sprite;
    for (int i = 0; i < NUMBER_INPUT_LANES; i++) {
        sprite.reg         = i + NOTE_SPRITE_OFFSET;
        sprite.sprite_id   = SPRITE_FORM_NOTE_CIRCLE_SOLID;
        sprite.sprite_type = VGA_SPRITE_16x16;
        sprite.color       = lane_color_palette[lane];
        sprite.pos_x       = lane_locations[lane] + (KEY_LANE_W - SPRITE_SMALL) / 2; // Center the note in the lane
        sprite.pos_y       = KEY_Y;
    }

    note->active = 0;       // Deactivate the note
    note->hittable = 0;
    note->y = 0;            // Start at the top of screen
    note->sprite = sprite;  // Assign the sprite
    note->tick_ctr = 0;     // Reset the tick counter
}

void note_init_notes(void) {
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        for (int i = 0; i < NOTES_PER_LANE; i++) {
            uint8_t reg = (lane * NOTES_PER_LANE) + i + NOTE_SPRITE_OFFSET; // Assign sprite register index
            note_init(&notes[lane][i], reg, lane);
        }
    }

    // Co-authored by Copilot
    // Seed the LCG from the RISC-V cycle CSR for non-deterministic spawning.
    __asm__ volatile ("rdcycle %0" : "=r"(lcg_state));

    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        spawn_counters[lane]   = 0;
        spawn_thresholds[lane] = rand_threshold();
    }
}


// Returns true when the note has fallen past the bottom of the VGA display.
bool note_complete(uint16_t y, uint8_t sprite_height) {
    return y + sprite_height > SCREEN_H;
}

// The hit box spans [KEY_Y, KEY_Y + KEY_SPRITE_H). Because the note is
// smaller or equal to the hit box, the note is hittable once its center
// row enters the hit box and until its center row exits the bottom.
bool note_hittable_check(uint16_t y, uint8_t sprite_height) {
    uint16_t note_top    = y;
    uint16_t note_bottom = y + sprite_height;
    uint16_t hit_top     = KEY_Y;
    uint16_t hit_bottom  = KEY_Y + KEY_SPRITE_H;

    uint16_t overlap_top    = (note_top    > hit_top)    ? note_top    : hit_top;
    uint16_t overlap_bottom = (note_bottom < hit_bottom) ? note_bottom : hit_bottom;

    if (overlap_bottom <= overlap_top) {return false;}

    uint16_t overlap = overlap_bottom - overlap_top;

    return (overlap > (uint16_t)sprite_height / 2u);
}

// Updates every note across all lanes.
void note_movement_routine(void) {
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        for (int i = 0; i < NOTES_PER_LANE; i++) {
            Note *note = &notes[lane][i];
            Sprite sprite = note->sprite;

            if (!note->active) { continue; }

            if (note->tick_ctr < TICK_THRESHOLD) {
                note->tick_ctr++;
                continue;
            }

            note->tick_ctr = 0;
            note->y += INCREMENT_Y;
            note->sprite.pos_y = note->y;
            vga_set_sprite(&sprite);

            note->hittable = note_hittable_check(note->y, SPRITE_SMALL);

            if (note_complete(note->y, SPRITE_SMALL)) {
                note->active   = 0;
                note->hittable = 0;
                vga_clear_sprite(sprite.reg);
            }
        }
    }
}

// Returns true if any note in the given lane is active and within the hit zone.
bool note_lane_hit_check(int lane) {
    for (int i = 0; i < NOTES_PER_LANE; i++) {
        if (notes[lane][i].active && notes[lane][i].hittable) {
            return true;
        }
    }
    return false;
}

// Deactivates the first hittable note in the lane and clears its sprite.
void note_process_hit(int lane) {
    for (int i = 0; i < NOTES_PER_LANE; i++) {
        Note *note = &notes[lane][i];
        if (note->active && note->hittable) {
            note->active   = 0;
            note->hittable = 0;
            vga_clear_sprite(note->sprite.reg);
            return;
        }
    }
}

// Co-authored by Copilot
// Spawns a new note in a random eligible lane.
// A lane is eligible once its counter reaches its randomized threshold AND it
// has a free note slot. One eligible lane is chosen at random and one note
// is activated at y=0.
void note_spawn_routine(void) {
    // Increment counters and collect eligible lanes in one pass.
    uint8_t eligible[NUMBER_INPUT_LANES];
    uint8_t eligible_count = 0;

    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        spawn_counters[lane]++;

        if (spawn_counters[lane] < spawn_thresholds[lane]) { continue; }

        for (int i = 0; i < NOTES_PER_LANE; i++) {
            if (!notes[lane][i].active) {
                eligible[eligible_count++] = (uint8_t)lane;
                break;
            }
        }
    }

    if (eligible_count == 0) { return; }

    // Pick a random eligible lane and find its first free slot.
    int chosen_lane = eligible[lcg_rand() % eligible_count];
    int slot = -1;
    for (int i = 0; i < NOTES_PER_LANE; i++) {
        if (!notes[chosen_lane][i].active) {
            slot = i;
            break;
        }
    }

    if (slot < 0) { return; } // Defensive: should not happen

    // Activate the note at the top of the screen.
    Note *note = &notes[chosen_lane][slot];
    note->active       = 1;
    note->hittable     = 0;
    note->y            = 0;
    note->tick_ctr     = 0;
    note->sprite.pos_y = 0;
    vga_set_sprite(&note->sprite);

    // Reset counter and re-randomize threshold for this lane.
    spawn_counters[chosen_lane]   = 0;
    spawn_thresholds[chosen_lane] = rand_threshold();
}

