////////////////////////////////////////////////////////////////////////////////
// Engineer:    Sajida Sayyad
// Create Date: 29/05/2026
// File Name:   score.c
// Project Name: Guitar Hero FPGA
// Description:
//   Tracks the player's score and updates the display. Each successful hit
//   adds 100 points to the score.
////////////////////////////////////////////////////////////////////////////////

#include "score.h"
#include "gpio.h"  // For gpio_display_number()

static int score = 0;  // Player's current score

// Initializes the score to 0 and updates the display.
void score_init(void) {
    score = 0;                     // Reset score to 0
    gpio_display_number(score);    // Update the display
}

// Adds points to the player's score and updates the display.
void add_score(int points) {
    score += points;               // Add points to the score
    gpio_display_number(score);    // Update the display
}

// Returns the current score.
int get_score(void) {
    return score;  // Return the current score
}