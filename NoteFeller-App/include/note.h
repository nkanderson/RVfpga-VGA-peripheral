////////////////////////////////////////////////////////////////////////////////
// Engineer:    Jacob Burtenshaw
// Create Date: 29/05/2026
// File Name:   note.h
// Project Name: Note Feller
// Description:
//   Header file for note management. Defines functions to spawn, update,
//   and check hits for notes.
////////////////////////////////////////////////////////////////////////////////

#ifndef NOTES_H
#define NOTES_H

#include <stdint.h>
#include <stdbool.h>

#include "globals.h"    
#include "key.h"        
#include "vga_sprite.h"

#define NOTES_PER_LANE (MAX_NOTES_IN_GAME / NUMBER_INPUT_LANES)

typedef struct {
    uint8_t  active;       // 1 if the note is active, 0 otherwise
    uint16_t y;            // Current y pixel position of the note
    bool     hittable;    // 1 if the note is within the hit zone, 0 otherwise
    uint16_t tick_ctr;    // Counter for timing note movement
    Sprite   sprite;
} Note;

// Initializes the note system (sets note as inactive).
void note_init(Note *note, uint8_t reg, uint8_t lane);

void note_init_notes(void);

// Spawns a new note in a random lane if no note is active.
void note_spawn_routine(void);

// Updates the position of all notes across all lanes.
void note_movement_routine(void);

// Returns true if any note in the given lane is active and within the hit zone.
bool note_lane_hit_check(int lane);

// Deactivates the first hittable note in the lane and clears its sprite.
void note_process_hit(int lane);

// Returns true when the note has fallen past the bottom of the VGA display.
bool note_complete(uint16_t y, uint8_t sprite_height);

// Returns true when more than half of the note sprite overlaps the key hit box.
bool note_hittable_check(uint16_t y, uint8_t sprite_height);

#endif // NOTES_H