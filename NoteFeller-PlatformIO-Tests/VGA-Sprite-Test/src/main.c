////////////////////////////////////////////////////////////////////////////////
// Engineer: Jacob Burtenshaw
// Create Date: 05/25/2026
// File Name: main.c
// Project Name: Note Feller — VGA Sprite Test
//
// Description:
//   Guitar Hero notes-falling demo using the vga_sprite driver API.
//   Five lanes of colored rectangular notes fall from top to bottom on a
//   white background. Dotted grey lane separators sit centered on each lane
//   boundary. A 24px tall colored hit zone (three stacked 32x8 bars) sits at
//   the bottom of each lane.
//
//   Sprite allocation (32 total):
//     0–4   : Falling note bars (SPR_HORIZ_BAR_SOLID, one per lane)
//     5–9   : Hit zone row 0 (y = HIT_Y)
//     10–14 : Hit zone row 1 (y = HIT_Y + 8)
//     15–19 : Hit zone row 2 (y = HIT_Y + 16)
//     20–23 : Separator dots row 0 (4 columns, y = DOT_Y0)
//     24–27 : Separator dots row 1 (y = DOT_Y1)
//     28–31 : Separator dots row 2 (y = DOT_Y2)
//
// AI Contributions:
//   Copilot generated the layout constants, sprite setup helpers, and the
//   falling-note animation loop.
////////////////////////////////////////////////////////////////////////////////

#include "vga_sprite.h"

// -----------------------------------------------------------------------------
// Layout constants
// -----------------------------------------------------------------------------

#define N_LANES         5
#define LANE_W          32
#define SCREEN_W        640
#define LANES_TOTAL_W   (N_LANES * LANE_W)
#define LANE_START_X    ((SCREEN_W - LANES_TOTAL_W) / 2)   // 240

// Note x: flush to the lane left edge (bar is 32px wide = one full lane).
#define NOTE_X(lane)    (LANE_START_X + (lane) * LANE_W)

// Separator x: centered on the boundary between lanes, offset -2 so the 4px
// wide separator straddles the boundary.  Four separators for five lanes.
#define SEP_X(gap)      (LANE_START_X + ((gap) + 1) * LANE_W - 2)

// Hit zone: three stacked horizontal bars per lane = 24px tall rectangle.
#define HIT_Y           430
#define HIT_ROW0        (HIT_Y)
#define HIT_ROW1        (HIT_Y + 8)
#define HIT_ROW2        (HIT_Y + 16)

// Notes wrap back to the top when they reach the hit zone.
#define NOTE_WRAP_Y     HIT_Y

// Dotted separators: three SPR_VERT_SEP_SOLID bars per column, evenly spaced.
// Each bar is 32px tall with ~110px spacing between groups.
#define DOT_Y0          16
#define DOT_Y1          (DOT_Y0 + 32 + 110)
#define DOT_Y2          (DOT_Y1 + 32 + 110)

// -----------------------------------------------------------------------------
// Per-lane colors
// -----------------------------------------------------------------------------

static const uint16_t lane_color[N_LANES] = {
    VGA_COLOR(0,  15,  0),   // Green
    VGA_COLOR(15,  0,  0),   // Red
    VGA_COLOR(15, 15,  0),   // Yellow
    VGA_COLOR(0,   0, 15),   // Blue
    VGA_COLOR(15,  8,  0),   // Orange
};

// -----------------------------------------------------------------------------
// Note fall parameters
// -----------------------------------------------------------------------------

static const int note_speed[N_LANES]  = { 3, 4, 5, 3, 4 };
static const int note_init_y[N_LANES] = { 0, -90, -200, -50, -150 };

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

static void delay(volatile unsigned int iters)
{
    while (iters--) {
        __asm__ volatile ("nop");
    }
}

static void setup_static_sprites(void)
{
    int lane, gap, row;
    const int dot_ys[3] = { DOT_Y0, DOT_Y1, DOT_Y2 };

    // Hit zone — three stacked bars per lane.
    for (lane = 0; lane < N_LANES; lane++) {
        vga_set_sprite( 5 + lane, SPR_HORIZ_BAR_SOLID, VGA_SPRITE_32x32,
                        lane_color[lane], NOTE_X(lane), HIT_ROW0);
        vga_set_sprite(10 + lane, SPR_HORIZ_BAR_SOLID, VGA_SPRITE_32x32,
                        lane_color[lane], NOTE_X(lane), HIT_ROW1);
        vga_set_sprite(15 + lane, SPR_HORIZ_BAR_SOLID, VGA_SPRITE_32x32,
                        lane_color[lane], NOTE_X(lane), HIT_ROW2);
    }

    // Dotted separators — four columns × three rows.
    for (row = 0; row < 3; row++) {
        for (gap = 0; gap < 4; gap++) {
            vga_set_sprite(20 + row * 4 + gap, SPR_VERT_SEP_SOLID, VGA_SPRITE_32x32,
                           VGA_COLOR(8, 8, 8), SEP_X(gap), dot_ys[row]);
        }
    }
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

int main(void)
{
    int lane;
    int note_y[N_LANES];

    vga_init();

    setup_static_sprites();

    for (lane = 0; lane < N_LANES; lane++) {
        note_y[lane] = note_init_y[lane];
    }

    while (1) {
        for (lane = 0; lane < N_LANES; lane++) {
            note_y[lane] += note_speed[lane];

            if (note_y[lane] >= NOTE_WRAP_Y) {
                note_y[lane] = -8;
            }

            if (note_y[lane] >= 0) {
                vga_set_sprite(lane, SPR_HORIZ_BAR_SOLID, VGA_SPRITE_32x32,
                               lane_color[lane], NOTE_X(lane), note_y[lane]);
            } else {
                vga_clear_sprite(lane);
            }
        }

        delay(50000);
    }

    return 0;
}
