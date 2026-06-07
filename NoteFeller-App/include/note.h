////////////////////////////////////////////////////////////////////////////////
// Engineer:    Jacob Burtenshaw & Chris Kane-Pardy
// Create Date: 29/05/2026
// File Name:   note.h
// Project Name: Note Feller
//
// Description:
//   Header file for note management. Defines functions to spawn, update,
//   check hits, and control note difficulty.
////////////////////////////////////////////////////////////////////////////////

#ifndef NOTES_H
#define NOTES_H

#include <stdint.h>
#include <stdbool.h>

#include "globals.h"
#include "vga_sprite.h"

#define NOTES_PER_LANE (MAX_NOTES_IN_GAME / NUMBER_INPUT_LANES)

typedef struct {
    uint8_t  active;
    uint16_t y;
    bool     hittable;
    uint16_t tick_ctr;
    Sprite   sprite;
} Note;

// Initializes one note object.
void note_init(Note *note, uint8_t reg, uint8_t lane);

// Initializes all notes and resets spawn/difficulty state.
void note_init_notes(void);

// Spawns new notes when lanes become eligible.
void note_spawn_routine(void);

// Updates all active notes.
// Returns a lane bitmask indicating which lanes had notes missed.
uint32_t note_movement_routine(void);

// Returns true if any note in the given lane is active and hittable.
bool note_lane_hit_check(int lane);

// Deactivates the first hittable note in the lane.
bool note_process_hit(int lane);

// Utility checks.
bool note_complete(uint16_t y, uint8_t sprite_height);
bool note_hittable_check(uint16_t y, uint8_t sprite_height);

#endif // NOTES_H