////////////////////////////////////////////////////////////////////////////////
// Engineer:    Jacob Burtenshaw
// Create Date: 26/05/2026
// File Name:   note.c
// Project Name: Note Feller
// Description:
//   Handles note spawning, movement, and hit detection.
////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "note.h"
#include "globals.h"       

// Define constants for note movement
#define TICK_THRESHOLD 1000  // Number of ticks before moving the note down
#define INCREMENT_Y 2

#define SPRITE_SMALL   16    // Height/width of a VGA_SPRITE_16x16 sprite in pixels
#define SPRITE_BIG     32    // Height/width of a VGA_SPRITE_32x32 sprite in pixels

#define SPAWN_THRESHOLD 60000
#define MAX_NOTES_PER_WAVE 4

#define HIT_WINDOW_PAD_TOP    0
#define HIT_WINDOW_PAD_BOTTOM 0

#define SPAWN_CHANCE_PERCENT  2u
#define MIN_SPAWN_GAP_TICKS   80u

// Note array — private to this file; use the note_* API from outside.
static Note notes[NUMBER_INPUT_LANES][NOTES_PER_LANE];

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

    // Seed srand from build timestamp to avoid CSR dependency.
    // __TIME__ expands to "HH:MM:SS" at compile time (standard C predefined macro).
    const char *t = __TIME__;
    unsigned int seed = ((unsigned int)(t[0]) * 100u + (unsigned int)(t[1]) * 10u +
                         (unsigned int)(t[3]) * 100u + (unsigned int)(t[4]) * 10u +
                         (unsigned int)(t[6]) * 10u  + (unsigned int)(t[7])) * 2654435761u;
    srand(seed);
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

// Spawns a random number of notes on random eligible lanes.
// A static counter increments each call. Once it exceeds SPAWN_THRESHOLD,
// rand() determines how many notes to spawn (1 to MAX_NOTES_PER_WAVE).
// Each note is assigned a random lane; if that lane is full the note is
// skipped. The counter resets regardless.
void note_spawn_routine(void)
{
    static uint32_t call_counter = 0;

    call_counter++;
    if (call_counter < SPAWN_THRESHOLD) {
        return;
    }
    call_counter = 0;

    int notes_to_spawn = 1 + (rand() % MAX_NOTES_PER_WAVE);

    for (int n = 0; n < notes_to_spawn; n++) {
        int lane = rand() % NUMBER_INPUT_LANES;

        // If the lane is full, skip this note.
        if (note_count_active_in_lane(lane) >= NOTES_PER_LANE) {
            continue;
        }

        // Activate the first free slot in the chosen lane.
        for (int i = 0; i < NOTES_PER_LANE; i++) {
            Note *note = &notes[lane][i];

            if (!note->active) {
                note->active       = 1;
                note->hittable     = 0;
                note->y            = 0;
                note->tick_ctr     = 0;
                note->sprite.pos_y = 0;
                vga_set_sprite(&note->sprite);
                break;
            }
        }
    }
}