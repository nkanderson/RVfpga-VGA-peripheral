////////////////////////////////////////////////////////////////////////////////
// Engineer:    Sajida Sayyad
// Create Date: 29/05/2026
// File Name:   score.h
// Project Name: Guitar Hero FPGA
// Description:
//   Header file for score tracking. Defines functions to initialize, update,
//   and retrieve the player's score.
////////////////////////////////////////////////////////////////////////////////

#ifndef SCORE_H
#define SCORE_H

// Initializes the score to 0 and updates the display.
void score_init(void);

// Adds points to the player's score and updates the display.
void add_score(int points);

// Returns the current score.
int get_score(void);

#endif // SCORE_H