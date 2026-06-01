////////////////////////////////////////////////////////////////////////////////
// Engineer:    Sajida Sayyad
// Create Date: 29/05/2026
// File Name:   keys.h
// Project Name: Guitar Hero FPGA
// Description:
//   Header file for lane/chord management. Defines the `Key` structure and
//   functions to initialize and retrieve lane data.
////////////////////////////////////////////////////////////////////////////////

#ifndef KEYS_H
#define KEYS_H

#include <stdint.h>

// --- Key Structure ---
typedef struct {
    uint8_t led;          // LED to blink for this key
    uint32_t button;      // Button mask for this key
    uint8_t audio_voice;  // Audio voice to play when key is pressed
    uint8_t score_led;    // LED to blink for score increment
} Key;

// Initializes the lane data (LEDs, buttons, audio tones, score LEDs).
void keys_init(void);

// Returns the `Key` structure for a given lane (0-3).
Key* get_key(int lane);

#endif // KEYS_H