////////////////////////////////////////////////////////////////////////////////
// Engineer: Chris Kane-Pardy
// Create Date: 05/21/2026
// File Name: input_controller.h
// Project Name: Note Feller
//
// Description:
//   Header file for the Note Feller input controller software driver.
//   This driver provides a clean game-facing API for reading player input from
//   the memory-mapped wb_input_controller peripheral.
//
//   Current input source:
//     - Nexys A7 onboard pushbuttons
//
//   Future input source:
//     - USB keyboard input through the same software-facing interface
//
// Register Base:
//   0x80001500
////////////////////////////////////////////////////////////////////////////////

#ifndef INPUT_CONTROLLER_H
#define INPUT_CONTROLLER_H

#include <stdint.h>

// -----------------------------------------------------------------------------
// Input controller base address and register offsets
// -----------------------------------------------------------------------------

#define INPUT_CONTROLLER_BASE      0x80001500UL

#define INPUT_STATUS_REG          (*(volatile uint32_t *)(INPUT_CONTROLLER_BASE + 0x00))
#define INPUT_EDGE_REG            (*(volatile uint32_t *)(INPUT_CONTROLLER_BASE + 0x04))
#define INPUT_CTRL_REG            (*(volatile uint32_t *)(INPUT_CONTROLLER_BASE + 0x08))
#define INPUT_MODE_REG            (*(volatile uint32_t *)(INPUT_CONTROLLER_BASE + 0x0C))

// -----------------------------------------------------------------------------
// Input bit masks
// -----------------------------------------------------------------------------

#define INPUT_MASK_ALL             0x1F

#define INPUT_LANE_0               (1U << 0)
#define INPUT_LANE_1               (1U << 1)
#define INPUT_LANE_2               (1U << 2)
#define INPUT_LANE_3               (1U << 3)
#define INPUT_LANE_4               (1U << 4)

// -----------------------------------------------------------------------------
// Input source modes
// -----------------------------------------------------------------------------

#define INPUT_MODE_BUTTONS         0x0
#define INPUT_MODE_USB             0x1

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

void input_init(void);

uint32_t input_get_status(void);
uint32_t input_get_edges(void);

void input_clear_edges(uint32_t mask);
void input_clear_all_edges(void);

uint32_t input_poll_new_presses(void);

void input_set_mode(uint32_t mode);
uint32_t input_get_mode(void);

#endif