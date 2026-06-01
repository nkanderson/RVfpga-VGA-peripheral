////////////////////////////////////////////////////////////////////////////////
// Engineer:    Sajida Sayyad
// Create Date: 29/05/2026
// File Name:   notes.h
// Project Name: Guitar Hero FPGA
// Description:
//   Header file for note management. Defines functions to spawn, update,
//   and check hits for notes.
////////////////////////////////////////////////////////////////////////////////

#ifndef NOTES_H
#define NOTES_H

#include "globals.h"  // For Note structure and game constants
// --- Note Structure ---
typedef struct {
    uint8_t active;       // 1 if the note is active, 0 otherwise
    uint8_t y;            // Current position of the note (LED index)
    uint8_t lane;         // Lane the note belongs to (0-3)
} Note;
// Initializes the note system (sets note as inactive).
void notes_init(void);

// Spawns a new note in a random lane if no note is active.
void spawn_note(void);

// Updates the position of the active note and checks for misses.
void update_note(void);

// Checks if a note is hit by the player and handles scoring/audio.
void check_hit(uint32_t input);

#endif // NOTES_H