////////////////////////////////////////////////////////////////////////////////
// Engineer: Jacob Burtenshaw
// Create Date: 05/25/2026
// File Name: main.c
// Project Name: Note Feller — VGA Sprite Test
//
// Description:
//   Guitar Hero notes-falling demo using the vga_sprite driver API. Utilizes
//   all 64 hardware sprite registers to render a full keyboard-instrument scene:
//   three waves of falling colored notes per lane, a 24px hit zone, dotted
//   lane separators, a top/bottom border, side border segments, and corners.
//
//   Sprite allocation (64 total):
//     0–4   : Note wave A — fast falling notes, one per lane
//     5–9   : Note wave B — medium speed, staggered start Y
//     10–14 : Note wave C — slow speed, near top at start
//     15–19 : Hit zone row 0 (y = HIT_Y)
//     20–24 : Hit zone row 1 (y = HIT_Y + 8)
//     25–29 : Hit zone row 2 (y = HIT_Y + 16)
//     30–33 : Separator dot row 0 (4 columns, y = DOT_Y0)
//     34–37 : Separator dot row 1 (y = DOT_Y1)
//     38–41 : Separator dot row 2 (y = DOT_Y2)
//     42–46 : Top border (5 × SPR_HORIZ_BORDER_SOLID across 160px)
//     47–51 : Bottom border (5 tiles, below hit zone)
//     52–55 : Left border  (4 × SPR_VERT_BORDER_LEFT,  128px coverage)
//     56–59 : Right border (4 × SPR_VERT_BORDER_RIGHT, 128px coverage)
//     60–63 : Corners — TL, TR, BL, BR
//
// AI Contributions:
//   Copilot generated the layout constants, sprite setup helpers, and the
//   multi-wave falling-note animation loop.
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
#define LANE_END_X      (LANE_START_X + LANES_TOTAL_W)     // 400

// Note x: flush to the lane left edge (bar is 32px wide = one full lane).
#define NOTE_X(lane)    (LANE_START_X + (lane) * LANE_W)

// Separator x: centered on boundary, -2 so the 4px bar straddles the boundary.
#define SEP_X(gap)      (LANE_START_X + ((gap) + 1) * LANE_W - 2)

// Frame border constants — the play area runs from FRAME_TOP to FRAME_BOTTOM.
#define FRAME_TOP       10
#define FRAME_BOTTOM    460
#define BORDER_LEFT_X   (LANE_START_X - 2)    // 2px wide border flush to lanes
#define BORDER_RIGHT_X  (LANE_END_X)           // immediately after last lane

// Hit zone: three stacked horizontal bars per lane = 24px tall.
#define HIT_Y           428
#define HIT_ROW0        (HIT_Y)
#define HIT_ROW1        (HIT_Y + 8)
#define HIT_ROW2        (HIT_Y + 16)

// Notes wrap back off the top when they pass the hit zone.
#define NOTE_WRAP_Y     HIT_Y

// Dotted separators: 3 rows evenly spaced through the play area.
#define DOT_Y0          48
#define DOT_Y1          (DOT_Y0 + 32 + 120)
#define DOT_Y2          (DOT_Y1 + 32 + 120)

// Three note waves with different speeds per lane.
#define N_WAVES         3

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
// Note fall parameters — three waves per lane (fast, medium, slow)
// -----------------------------------------------------------------------------

// Speed (pixels per tick) for each wave.
static const int wave_speed[N_WAVES] = { 5, 3, 2 };

// Initial Y offsets so the three waves are visually spread out.
static const int wave_init_y[N_LANES][N_WAVES] = {
    {   0, -140, -300 },
    { -55, -200,  -80 },
    {-110,  -30, -210 },
    { -80, -250,  -50 },
    { -20, -170, -340 },
};

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

static void delay(volatile unsigned int iters)
{
    while (iters--) {
        __asm__ volatile ("nop");
    }
}

// Co-authored by Copilot
static void setup_static_sprites(void)
{
    int lane, gap, row;
    Sprite s;

    // Hit zone — three stacked 32x8 bars per lane.
    s.sprite_id   = SPR_HORIZ_BAR_SOLID;
    s.sprite_type = VGA_SPRITE_32x32;
    for (lane = 0; lane < N_LANES; lane++) {
        s.color = lane_color[lane];
        s.pos_x = NOTE_X(lane);

        s.pos_y = HIT_ROW0;
        vga_set_sprite(15 + lane, s.sprite_id, s.sprite_type, s.color, s.pos_x, s.pos_y);
        s.pos_y = HIT_ROW1;
        vga_set_sprite(20 + lane, s.sprite_id, s.sprite_type, s.color, s.pos_x, s.pos_y);
        s.pos_y = HIT_ROW2;
        vga_set_sprite(25 + lane, s.sprite_id, s.sprite_type, s.color, s.pos_x, s.pos_y);
    }

    // Dotted lane separators — 4 gaps × 3 rows.
    s.sprite_id   = SPR_VERT_SEP_SOLID;
    s.sprite_type = VGA_SPRITE_32x32;
    s.color       = VGA_COLOR(8, 8, 8);
    {
        int dot_ys[3] = { DOT_Y0, DOT_Y1, DOT_Y2 };
        for (row = 0; row < 3; row++) {
            s.pos_y = dot_ys[row];
            for (gap = 0; gap < 4; gap++) {
                s.pos_x = SEP_X(gap);
                vga_set_sprite(30 + row * 4 + gap,
                               s.sprite_id, s.sprite_type, s.color, s.pos_x, s.pos_y);
            }
        }
    }

    // Top border — 5 tiles of 32x2 spanning the full 160px play area width.
    s.sprite_id   = SPR_HORIZ_BORDER_SOLID;
    s.sprite_type = VGA_SPRITE_32x32;
    s.color       = VGA_WHITE;
    s.pos_y       = FRAME_TOP;
    for (int t = 0; t < N_LANES; t++) {
        s.pos_x = LANE_START_X + t * LANE_W;
        vga_set_sprite(42 + t, s.sprite_id, s.sprite_type, s.color, s.pos_x, s.pos_y);
    }

    // Bottom border — same span, just below hit zone.
    s.pos_y = FRAME_BOTTOM;
    for (int t = 0; t < N_LANES; t++) {
        s.pos_x = LANE_START_X + t * LANE_W;
        vga_set_sprite(47 + t, s.sprite_id, s.sprite_type, s.color, s.pos_x, s.pos_y);
    }

    // Left border — 4 × 2x32 segments.
    s.sprite_id = SPR_VERT_BORDER_LEFT;
    s.pos_x     = BORDER_LEFT_X;
    for (int seg = 0; seg < 4; seg++) {
        s.pos_y = FRAME_TOP + 2 + seg * 32;
        vga_set_sprite(52 + seg, s.sprite_id, s.sprite_type, s.color, s.pos_x, s.pos_y);
    }

    // Right border — 4 × 2x32 segments.
    s.sprite_id = SPR_VERT_BORDER_RIGHT;
    s.pos_x     = BORDER_RIGHT_X;
    for (int seg = 0; seg < 4; seg++) {
        s.pos_y = FRAME_TOP + 2 + seg * 32;
        vga_set_sprite(56 + seg, s.sprite_id, s.sprite_type, s.color, s.pos_x, s.pos_y);
    }

    // Corners — TL, TR, BL, BR.
    s.sprite_id = SPR_CORNER_TL;
    s.pos_x = BORDER_LEFT_X;  s.pos_y = FRAME_TOP;
    vga_set_sprite(60, s.sprite_id, s.sprite_type, s.color, s.pos_x, s.pos_y);

    s.sprite_id = SPR_CORNER_TR;
    s.pos_x = BORDER_RIGHT_X; s.pos_y = FRAME_TOP;
    vga_set_sprite(61, s.sprite_id, s.sprite_type, s.color, s.pos_x, s.pos_y);

    s.sprite_id = SPR_CORNER_BL;
    s.pos_x = BORDER_LEFT_X;  s.pos_y = FRAME_BOTTOM;
    vga_set_sprite(62, s.sprite_id, s.sprite_type, s.color, s.pos_x, s.pos_y);

    s.sprite_id = SPR_CORNER_BR;
    s.pos_x = BORDER_RIGHT_X; s.pos_y = FRAME_BOTTOM;
    vga_set_sprite(63, s.sprite_id, s.sprite_type, s.color, s.pos_x, s.pos_y);
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

int main(void)
{
    int lane, wave;
    int note_y[N_LANES][N_WAVES];

    vga_init();
    setup_static_sprites();

    for (lane = 0; lane < N_LANES; lane++) {
        for (wave = 0; wave < N_WAVES; wave++) {
            note_y[lane][wave] = wave_init_y[lane][wave];
        }
    }

    // Co-authored by Copilot
    while (1) {
        Sprite note;
        note.sprite_id   = SPR_HORIZ_BAR_SOLID;
        note.sprite_type = VGA_SPRITE_32x32;

        for (wave = 0; wave < N_WAVES; wave++) {
            for (lane = 0; lane < N_LANES; lane++) {
                int reg = wave * N_LANES + lane;

                note_y[lane][wave] += wave_speed[wave];

                if (note_y[lane][wave] >= NOTE_WRAP_Y) {
                    note_y[lane][wave] = -8;
                }

                if (note_y[lane][wave] >= 0) {
                    note.color = lane_color[lane];
                    note.pos_x = NOTE_X(lane);
                    note.pos_y = (uint16_t)note_y[lane][wave];
                    vga_set_sprite(reg, note.sprite_id, note.sprite_type,
                                   note.color, note.pos_x, note.pos_y);
                } else {
                    vga_clear_sprite(reg);
                }
            }
        }

        delay(40000);
    }

    return 0;
}
