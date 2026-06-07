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
#define TICK_THRESHOLD 1000  // Number of ticks before moving the note down
#define INCREMENT_Y 2

#define SPRITE_SMALL   16    // Height/width of a VGA_SPRITE_16x16 sprite in pixels
#define SPRITE_BIG     32    // Height/width of a VGA_SPRITE_32x32 sprite in pixels

// Spawn timing: each lane independently waits a random number of routine calls
// (in the range [BASE, BASE+RANGE)) before it becomes eligible to emit a note.
#define SPAWN_THRESHOLD_BASE  60000u
#define SPAWN_THRESHOLD_RANGE 20000u

#define HIT_WINDOW_PAD_TOP    0
#define HIT_WINDOW_PAD_BOTTOM 0

#define SPAWN_CHANCE_PERCENT  2u
#define MIN_SPAWN_GAP_TICKS   80u
#define MAX_ACTIVE_PER_LANE   3u

// Note array — private to this file; use the note_* API from outside.
static Note notes[NUMBER_INPUT_LANES][NOTES_PER_LANE];

// Per-lane spawn state
static uint32_t spawn_counters[NUMBER_INPUT_LANES];   // calls since last spawn for each lane
static uint32_t spawn_thresholds[NUMBER_INPUT_LANES]; // randomized target calls before next spawn

// LCG PRNG (Knuth multiplicative coefficients)
static uint32_t lcg_state;

static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return lcg_state >> 16;
}

static uint32_t rand_threshold(void) {
    return (uint32_t)(SPAWN_THRESHOLD_BASE + (lcg_rand() % SPAWN_THRESHOLD_RANGE));
}

// Initialize a note
void note_init(Note *note, uint8_t reg, uint8_t lane) {
    Sprite sprite;
    sprite.reg         = reg;
    sprite.sprite_id   = SPRITE_FORM_CHORD_CIRCLE_SOLID;
    sprite.sprite_type = VGA_SPRITE_32x32;
    sprite.color       = lane_color_palette[lane];
    sprite.pos_x       = lane_locations[lane] + (KEY_LANE_W - SPRITE_BIG) / 2;
    sprite.pos_y       = 0;

    note->active = 0;       // Deactivate the note
    note->hittable = 0;
    note->y = 0;            // Start at the top of screen
    note->sprite = sprite;  // Assign the sprite
    note->tick_ctr = 0;     // Reset the tick counter
    vga_clear_sprite(note->sprite.reg); //Clear the sprite from the screen
}

void note_init_notes(void) {
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        for (int i = 0; i < NOTES_PER_LANE; i++) {
            uint8_t reg = (lane * NOTES_PER_LANE) + i + NOTE_SPRITE_OFFSET; // Assign sprite register index
            note_init(&notes[lane][i], reg, lane);
        }
    }

    // Seed the LCG from build timestamp to avoid CSR dependency.
    // __TIME__ expands to "HH:MM:SS" at compile time (standard C predefined macro).
    const char *t = __TIME__;
    lcg_state = ((uint32_t)(t[0]) * 100u + (uint32_t)(t[1]) * 10u +
                 (uint32_t)(t[3]) * 100u + (uint32_t)(t[4]) * 10u +
                 (uint32_t)(t[6]) * 10u  + (uint32_t)(t[7])) * 2654435761u;

    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
      spawn_thresholds[lane] = rand_threshold();
      spawn_counters[lane]   = (uint32_t)(lcg_rand() % spawn_thresholds[lane]);
    }
}


// Returns true when the note has fallen past the bottom of the VGA display.
bool note_complete(uint16_t y, uint8_t sprite_height) {
    return y + sprite_height > SCREEN_H;
}

// The hit box spans [KEY_Y, KEY_Y + KEY_SPRITE_H). Because the note is
// smaller or equal to the hit box, the note is hittable once its center
// row enters the hit box and until its center row exits the bottom.
bool note_hittable_check(uint16_t y, uint8_t sprite_height)
{
    uint16_t note_center = y + (sprite_height / 2);

    uint16_t hit_top    = KEY_Y;
    uint16_t hit_bottom = KEY_Y + KEY_SPRITE_H;

    return (note_center >= hit_top) && (note_center <= hit_bottom);
}

// Updates every note across all lanes.
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
                note->hittable = 0;
                vga_clear_sprite(note->sprite.reg);

                missed_mask |= (1u << lane);
            }
        }
    }

    return missed_mask;
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
bool note_process_hit(int lane)
{
    for (int i = 0; i < NOTES_PER_LANE; i++) {
        Note *note = &notes[lane][i];

        if (note->active && note->hittable) {
            note->active   = 0;
            note->hittable = 0;
            vga_clear_sprite(note->sprite.reg);
            return true;
        }
    }

    return false;
}

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

// Co-authored by Copilot
// Spawns a new note in a random eligible lane.
// A lane is eligible once its counter reaches its randomized threshold AND it
// has a free note slot. One eligible lane is chosen at random and one note
// is activated at y=0.
void note_spawn_routine(void)
{
    bool eligible_lanes[NUMBER_INPUT_LANES] = {false, false, false, false};
    int eligible_count = 0;
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
      if (spawn_counters[lane] < spawn_thresholds[lane]) {
        spawn_counters[lane]++;
      } else {
        if (note_count_active_in_lane(lane) < MAX_ACTIVE_PER_LANE) {
          eligible_lanes[lane] = true;
          eligible_count++;
        } else {
          // Enforce a gap for full lanes, to avoid lanes having reached
          // their threshold and then waiting and immediately re-spawning.
          // Helps prevent clumping of notes across eligible lanes.
          spawn_counters[lane] = 0;
          spawn_thresholds[lane] = rand_threshold();
        }
      }
    }

    if (!eligible_count) {
      return;
    }

    int chosen_lane = (int)(lcg_rand() % (uint32_t)eligible_count);
    int eligible_idx = 0;
    int spawned_lane = -1;

    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
      if (eligible_lanes[lane] && eligible_idx++ == chosen_lane) {
        int slot = -1;

        for (int i = 0; i < NOTES_PER_LANE; i++) {
            if (!notes[lane][i].active) {
                slot = i;
                break;
            }
        }

        if (slot < 0) {
            continue;
        }

        Note *note = &notes[lane][slot];

        note->active       = 1;
        note->hittable     = 0;
        note->y            = 0;
        note->tick_ctr     = 0;
        note->sprite.pos_y = 0;

        vga_set_sprite(&note->sprite);

        spawn_counters[lane] = 0;
        spawn_thresholds[lane] = rand_threshold();
        spawned_lane = lane;
      }
    }
    // Re-roll eligible lanes that weren't chosen to prevent note clumping.
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
      if (eligible_lanes[lane] && lane != spawned_lane) {
          spawn_counters[lane] = 0;
          spawn_thresholds[lane] = rand_threshold();
      }
    }
}
