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

#include "globals.h"  // For Key structure and game constants

// Initializes the lane data (LEDs, buttons, audio tones, score LEDs).
void keys_init(void);

// Returns the `Key` structure for a given lane (0-3).
Key* get_key(int lane);

#endif // KEYS_H