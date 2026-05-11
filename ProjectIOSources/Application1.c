//=============================================================
// Authors: Chris Kane-Pardy & Niklas Anderson
// Date Created: May 10 2026
// File: main.c
// Project: ECE 540 Project 2 - Application 1
//
// Google Gemini was also used as a debugging tool during bring up,
// as well as for help commenting the code and making sure all design
// intent was clearly communicated.
//
//   Application behavior:
//     - Iterates through decimal digits 0 through 9
//     - Displays even values as 0
//     - Displays odd values normally
//     - Uses a red background
//     - Uses a visible white foreground character
//     - Updates approximately once per second
//
//   VGA Peripheral Register Map:
//     BASE + 0x00 : MODE register
//                   bit 0 = 1 for text mode, 0 for graphics mode
//
//     BASE + 0x04 : COORD register
//                   bits [19:10] = row
//                   bits [9:0]   = column
//
//     BASE + 0x0C : DATA/color register
//                   background RGB:
//                     bits [31:28] = background red
//                     bits [27:24] = background green
//                     bits [23:20] = background blue
//
//                   foreground RGB:
//                     bits [15:12] = foreground red
//                     bits [11:8]  = foreground green
//                     bits [7:4]   = foreground blue
//
//     BASE + 0x10 : CHAR register
//                   bits [7:0] = ASCII character to display
//=============================================================

#include <stdint.h>

//-------------------------------------------------------------
// VGA register definitions
//-------------------------------------------------------------
#define VGA_BASE   0x80001500u

#define VGA_MODE   (*(volatile uint32_t *)(VGA_BASE + 0x00))
#define VGA_COORD  (*(volatile uint32_t *)(VGA_BASE + 0x04))
#define VGA_DATA   (*(volatile uint32_t *)(VGA_BASE + 0x0C))
#define VGA_CHAR   (*(volatile uint32_t *)(VGA_BASE + 0x10))

//-------------------------------------------------------------
// VGA mode values
//-------------------------------------------------------------
#define VGA_GRAPHICS_MODE  0u
#define VGA_TEXT_MODE      1u

//-------------------------------------------------------------
// Display position
//-------------------------------------------------------------
#define TEXT_ROW  100u
#define TEXT_COL  100u

//-------------------------------------------------------------
// Helper macros
//-------------------------------------------------------------
#define VGA_COORD_PACK(row, col) \
    ((((uint32_t)(row) & 0x3FFu) << 10) | ((uint32_t)(col) & 0x3FFu))

#define VGA_COLOR(bg_r, bg_g, bg_b, fg_r, fg_g, fg_b) \
    ((((uint32_t)(bg_r) & 0xFu) << 28) | \
     (((uint32_t)(bg_g) & 0xFu) << 24) | \
     (((uint32_t)(bg_b) & 0xFu) << 20) | \
     (((uint32_t)(fg_r) & 0xFu) << 12) | \
     (((uint32_t)(fg_g) & 0xFu) << 8)  | \
     (((uint32_t)(fg_b) & 0xFu) << 4))

//-------------------------------------------------------------
// delay
//
// Simple software delay loop. The exact delay depends on the
// CPU clock and compiler optimization level, but this value is
// intended to be approximately human-visible, near one second.
//-------------------------------------------------------------
static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm__ volatile ("nop");
    }
}

//-------------------------------------------------------------
// main
//-------------------------------------------------------------
int main(void)
{
    uint32_t number = 0;

    // Configure VGA peripheral for text mode.
    VGA_MODE = VGA_TEXT_MODE;

    // Place the character near the upper-left region of the screen.
    VGA_COORD = VGA_COORD_PACK(TEXT_ROW, TEXT_COL);

    // Red background with white text.
    VGA_DATA = VGA_COLOR(0xF, 0x0, 0x0, 0xF, 0xF, 0xF);

    while (1) {
        uint32_t display_value;

        // Project requirement:
        // even numbers are displayed as 0.
        if ((number % 2u) == 0u) {
            display_value = 0u;
        } else {
            display_value = number;
        }

        // Current hardware displays one character at a time.
        // This application iterates through one decimal digit.
        VGA_CHAR = (uint32_t)('0' + (display_value % 10u));

        delay(2500000u);

        number = (number + 1u) % 10u;
    }

    return 0;
}