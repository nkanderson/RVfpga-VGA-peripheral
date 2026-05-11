//=============================================================
// Author: Chris Kane-Pardy & Niklas Anderson
// Date Created: May 10 2026
// File: main.c
// Project: ECE 540 Project 2 - Application 2
//
// Google Gemini was also used as a debugging tool during bring up,
// as well as for help commenting the code and making sure all design
// intent was clearly communicated.
//
// Description:
//   This application implements a polished VGA screensaver using
//   the custom Wishbone VGA peripheral developed for Project 2.
//
//   The application operates in graphics mode and animates a
//   bouncing square across the VGA display. The square changes
//   color whenever it collides with a screen boundary.
//
//   Additional visual effects:
//     - Independent horizontal and vertical velocities
//     - Dynamic color cycling on collisions
//     - Speed variation over time
//     - Smooth diagonal motion
//
//   This application demonstrates:
//     - Software-controlled VGA animation
//     - Dynamic coordinate updates
//     - Real-time color manipulation
//     - Wishbone register communication
//     - Stable VGA graphics rendering
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
// Screen geometry
//-------------------------------------------------------------
#define SCREEN_WIDTH   640
#define SCREEN_HEIGHT  480
#define BOX_SIZE       32

//-------------------------------------------------------------
// Helper macros
//-------------------------------------------------------------
#define VGA_COORD_PACK(row, col) \
    ((((uint32_t)(row) & 0x3FFu) << 10) | ((uint32_t)(col) & 0x3FFu))

#define VGA_FG_COLOR(r, g, b) \
    ((((uint32_t)(r) & 0xFu) << 12) | \
     (((uint32_t)(g) & 0xFu) << 8)  | \
     (((uint32_t)(b) & 0xFu) << 4))

//-------------------------------------------------------------
// delay
//-------------------------------------------------------------
static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm__ volatile ("nop");
    }
}

//-------------------------------------------------------------
// set_box_color
//-------------------------------------------------------------
static void set_box_color(uint32_t r, uint32_t g, uint32_t b)
{
    VGA_DATA = VGA_FG_COLOR(r, g, b);
}

//-------------------------------------------------------------
// main
//-------------------------------------------------------------
int main(void)
{
    //---------------------------------------------------------
    // Box position
    //---------------------------------------------------------
    int32_t row = 80;
    int32_t col = 120;

    //---------------------------------------------------------
    // Initial motion vector
    //---------------------------------------------------------
    int32_t row_step = 3;
    int32_t col_step = 5;

    //---------------------------------------------------------
    // Color palette
    //---------------------------------------------------------
    const uint32_t colors[][3] = {
        {0xF, 0x0, 0x0}, // red
        {0x0, 0xF, 0x0}, // green
        {0x0, 0x0, 0xF}, // blue
        {0xF, 0xF, 0x0}, // yellow
        {0xF, 0x0, 0xF}, // magenta
        {0x0, 0xF, 0xF}, // cyan
        {0xF, 0xF, 0xF}, // white
        {0xF, 0x8, 0x0}  // orange
    };

    const uint32_t num_colors = sizeof(colors) / sizeof(colors[0]);

    uint32_t color_index = 0;
    uint32_t bounce_count = 0;

    //---------------------------------------------------------
    // Configure graphics mode
    //---------------------------------------------------------
    VGA_MODE = VGA_GRAPHICS_MODE;

    set_box_color(colors[color_index][0],
                  colors[color_index][1],
                  colors[color_index][2]);

    //---------------------------------------------------------
    // Main animation loop
    //---------------------------------------------------------
    while (1) {

        //-----------------------------------------------------
        // Update VGA hardware registers
        //-----------------------------------------------------
        VGA_COORD = VGA_COORD_PACK((uint32_t)row,
                                   (uint32_t)col);

        //-----------------------------------------------------
        // Horizontal collision detection
        //-----------------------------------------------------
        if ((col + col_step) <= 0) {
            col = 0;
            col_step = -col_step;
            bounce_count++;
            color_index = (color_index + 1) % num_colors;
        }
        else if ((col + col_step) >= (SCREEN_WIDTH - BOX_SIZE)) {
            col = SCREEN_WIDTH - BOX_SIZE;
            col_step = -col_step;
            bounce_count++;
            color_index = (color_index + 1) % num_colors;
        }
        else {
            col += col_step;
        }

        //-----------------------------------------------------
        // Vertical collision detection
        //-----------------------------------------------------
        if ((row + row_step) <= 0) {
            row = 0;
            row_step = -row_step;
            bounce_count++;
            color_index = (color_index + 1) % num_colors;
        }
        else if ((row + row_step) >= (SCREEN_HEIGHT - BOX_SIZE)) {
            row = SCREEN_HEIGHT - BOX_SIZE;
            row_step = -row_step;
            bounce_count++;
            color_index = (color_index + 1) % num_colors;
        }
        else {
            row += row_step;
        }

        //-----------------------------------------------------
        // Update color after collisions
        //-----------------------------------------------------
        set_box_color(colors[color_index][0],
                      colors[color_index][1],
                      colors[color_index][2]);

        //-----------------------------------------------------
        // Occasionally vary motion speed slightly to make
        // the animation feel less repetitive.
        //-----------------------------------------------------
        if ((bounce_count % 6) == 0 && bounce_count != 0) {

            if (col_step > 0)
                col_step = (col_step % 7) + 2;
            else
                col_step = -(((-col_step) % 7) + 2);

            if (row_step > 0)
                row_step = (row_step % 5) + 2;
            else
                row_step = -(((-row_step) % 5) + 2);

            bounce_count++;
        }

        //-----------------------------------------------------
        // Delay controls animation speed
        //-----------------------------------------------------
        delay(18000);
    }

    return 0;
}