////////////////////////////////////////////////////////////////////////////////
// Engineer: Chris Kane-Pardy
// Create Date: 05/21/2026
// File Name: input_controller.c
// Project Name: Note Feller
//
// Description:
//   Software driver for the Note Feller wb_input_controller peripheral.
//   This file abstracts the memory-mapped input registers behind a small API
//   that the game engine can call without depending on raw hardware addresses.
//
//   The current baseline implementation reads the Nexys A7 onboard pushbuttons.
//   The same API is intended to support USB keyboard input later by changing the
//   input mode in hardware while preserving the software interface.
//
// Register Map:
//   BASE + 0x00 : INPUT_STATUS
//                 Current active input bits.
//
//   BASE + 0x04 : INPUT_EDGE
//                 Latched newly pressed input bits.
//                 Write 1s to clear corresponding edge bits.
//
//   BASE + 0x08 : INPUT_CTRL
//                 Control register.
//                 bit[0] = clear all edge bits.
//
//   BASE + 0x0C : INPUT_MODE
//                 Input source select.
//                 0 = onboard pushbuttons
//                 1 = USB keyboard input
////////////////////////////////////////////////////////////////////////////////

#include "input_controller.h"

// -----------------------------------------------------------------------------
// input_init
//
// Initializes the input controller for baseline gameplay.
// The default mode is onboard pushbutton input.
// Any stale edge events are cleared before gameplay starts.
// -----------------------------------------------------------------------------

void input_init(void)
{
    input_set_mode(INPUT_MODE_BUTTONS);
    input_clear_all_edges();
}

// -----------------------------------------------------------------------------
// input_get_status
//
// Returns the current held input state.
// Each bit corresponds to one gameplay lane.
// -----------------------------------------------------------------------------

uint32_t input_get_status(void)
{
    return INPUT_STATUS_REG & INPUT_MASK_ALL;
}

// -----------------------------------------------------------------------------
// input_get_edges
//
// Returns latched newly pressed inputs.
// These bits remain set until cleared by software.
// -----------------------------------------------------------------------------

uint32_t input_get_edges(void)
{
    return INPUT_EDGE_REG & INPUT_MASK_ALL;
}

// -----------------------------------------------------------------------------
// input_clear_edges
//
// Clears selected edge bits.
// The hardware uses write-1-to-clear behavior.
// -----------------------------------------------------------------------------

void input_clear_edges(uint32_t mask)
{
    INPUT_EDGE_REG = mask & INPUT_MASK_ALL;
}

// -----------------------------------------------------------------------------
// input_clear_all_edges
//
// Clears all latched edge bits.
// -----------------------------------------------------------------------------

void input_clear_all_edges(void)
{
    INPUT_CTRL_REG = 0x1;
}

// -----------------------------------------------------------------------------
// input_poll_new_presses
//
// Convenience function for game logic.
// Reads newly pressed inputs, clears those edge flags, and returns the result.
// This is the preferred function for rhythm-game hit detection.
// -----------------------------------------------------------------------------

uint32_t input_poll_new_presses(void)
{
    uint32_t edges = input_get_edges();

    if (edges != 0) {
        input_clear_edges(edges);
    }

    return edges;
}

// -----------------------------------------------------------------------------
// input_set_mode
//
// Selects the hardware input source.
// Currently supported:
//   INPUT_MODE_BUTTONS = onboard pushbuttons
// Future:
//   INPUT_MODE_USB     = USB keyboard
// -----------------------------------------------------------------------------

void input_set_mode(uint32_t mode)
{
    INPUT_MODE_REG = mode;
}

// -----------------------------------------------------------------------------
// input_get_mode
//
// Returns the current input mode.
// -----------------------------------------------------------------------------

uint32_t input_get_mode(void)
{
    return INPUT_MODE_REG;
}