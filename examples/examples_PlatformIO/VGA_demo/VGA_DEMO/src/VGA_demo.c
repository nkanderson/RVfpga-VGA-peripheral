// ============================================================================
// File: VGA_demo.c
// Author: Jacob Burtenshaw
// Date Created: 2026-05-25
// Description: Guitar Hero notes-falling demo using the Phase 2 VGA sprite
//              engine. Five lanes of colored rectangular notes fall from top
//              to bottom on a white background. Block-dotted lane separators
//              (4px wide bars) sit centered on each lane boundary. A tall
//              colored rectangle hit zone (3 stacked 32x8 bars = 24px) sits
//              at the bottom of each lane.
//
//              Sprite allocation (32 total):
//                0–4   : Falling note bars (Horizontal Bar solid, ID 101)
//                5–9   : Hit zone row 0 (y=HIT_Y+0, Horizontal Bar solid)
//                10–14 : Hit zone row 1 (y=HIT_Y+8)
//                15–19 : Hit zone row 2 (y=HIT_Y+16)
//                20–23 : Separator dots row 0 (4 columns, y=DOT_Y0)
//                24–27 : Separator dots row 1 (y=DOT_Y1)
//                28–31 : Separator dots row 2 (y=DOT_Y2)
//
//              Sprite ID reference (unified ROM, IDs 95-111):
//                101 = Horizontal Bar solid  — 32px wide, 8px tall solid rect
//                103 = Vertical Separator solid — 4px wide, 32px tall bar
//
// AI Contributions: Copilot generated sprite register helpers, layout
//                   calculations, and the falling-note animation loop.
// ============================================================================


// =============================================================
// VGA Peripheral Base Address and Registers
// =============================================================
#define VGA_BASE        0x80003000u

#define REG_MODE        0x000   // bit[0]: 0=graphics, 1=text
#define REG_BG_COLOR    0x014   // [11:8]=R [7:4]=G [3:0]=B (12-bit)

// Sprite registers:
//   SPRITE_POS[n] at offset 0x080 + n*8 : [19:10]=y_pos  [9:0]=x_pos
//   SPRITE_CFG[n] at offset 0x084 + n*8 : [31:25]=sprite_id(7b)
//                                          [15:12]=R [11:8]=G [7:4]=B
//                                          [1]=sprite_type (0=32x32, 1=16x16)
//                                          [0]=visible
#define SPRITE_POS(n)   (0x080u + (n) * 8u)
#define SPRITE_CFG(n)   (0x084u + (n) * 8u)

// Sprite IDs (unified ROM)
#define SPR_HBAR        101u  // Horizontal Bar solid — 32x8px full solid rect
#define SPR_VSEP        103u  // Vertical Separator solid — 4px wide, 32px tall

// =============================================================
// Layout
// =============================================================
#define N_LANES         5
#define LANE_W          32        // px per lane
#define SCREEN_W        640
#define LANES_TOTAL_W   (N_LANES * LANE_W)  // 160px
#define LANE_START_X    ((SCREEN_W - LANES_TOTAL_W) / 2)  // 240

// Note x: flush to lane left edge (Horizontal Bar is 32px wide = full lane)
#define NOTE_X(lane)    (LANE_START_X + (lane) * LANE_W)

// Separator x: centered on each lane boundary, offset left by 2 so 4px bar
// straddles the boundary.  4 separators between 5 lanes (gap = 0..3).
#define SEP_X(gap)      (LANE_START_X + ((gap) + 1) * LANE_W - 2)

// Hit zone: 3 stacked Horizontal Bars = 24px tall solid rectangle per lane
#define HIT_Y           430
#define HIT_ROW0        (HIT_Y)
#define HIT_ROW1        (HIT_Y + 8)
#define HIT_ROW2        (HIT_Y + 16)

// Notes wrap back above screen when they reach the hit zone
#define NOTE_WRAP_Y     HIT_Y

// Dotted separator: 3 Vertical Separator bars per column, evenly spaced
// Each bar is 32px tall. Gap between dot-groups: ~110px.
#define DOT_Y0          16
#define DOT_Y1          (DOT_Y0 + 32 + 110)   // 158
#define DOT_Y2          (DOT_Y1 + 32 + 110)   // 300

// =============================================================
// Colors — SPRITE_CFG color field [15:4]: {R[3:0],G[3:0],B[3:0]} << 4
//          REG_BG_COLOR [11:0]: {R[3:0],G[3:0],B[3:0]}
// =============================================================
#define BG_WHITE        0xFFFu

static const unsigned int lane_color[N_LANES] = {
    0x0F00u,   // Green
    0xF000u,   // Red
    0xFF00u,   // Yellow
    0x00F0u,   // Blue
    0xF800u,   // Orange
};

// Light grey separators stand out against white without clashing with lane colors
#define SEP_COLOR       0x8880u  // R=8, G=8, B=8 — mid-grey

// Note fall speeds (px per frame)
static const int note_speed[N_LANES]  = { 3, 4, 5, 3, 4 };

// Staggered start positions
static const int note_init_y[N_LANES] = { 0, -90, -200, -50, -150 };

// =============================================================
// Helpers
// =============================================================

static inline void vga_wr(unsigned int offset, unsigned int val) {
    volatile unsigned int *r = (volatile unsigned int *)(VGA_BASE + offset);
    *r = val;
}

static inline void sprite_set_pos(int n, int y, int x) {
    unsigned int pos = ((unsigned int)(y & 0x3FFu) << 10) | (unsigned int)(x & 0x3FFu);
    vga_wr(SPRITE_POS((unsigned)n), pos);
}

static inline void sprite_set_cfg(int n, unsigned int sprite_id,
                                   unsigned int cfg_color,
                                   int sprite_type, int visible) {
    unsigned int cfg = (sprite_id << 25) | cfg_color
                       | (unsigned int)(sprite_type << 1)
                       | (unsigned int)(visible & 1);
    vga_wr(SPRITE_CFG((unsigned)n), cfg);
}

static void delay(unsigned int iters) {
    volatile unsigned int i;
    for (i = 0; i < iters; i++) asm volatile("nop");
}

// =============================================================
// Setup static sprites (hit zones + dotted separators)
// =============================================================
static void setup_static_sprites(void) {
    // Hit zone: 3 stacked Horizontal Bars per lane → 24px tall rectangle
    for (int lane = 0; lane < N_LANES; lane++) {
        int x = NOTE_X(lane);
        sprite_set_pos( 5 + lane, HIT_ROW0, x);
        sprite_set_cfg( 5 + lane, SPR_HBAR, lane_color[lane], 0, 1);
        sprite_set_pos(10 + lane, HIT_ROW1, x);
        sprite_set_cfg(10 + lane, SPR_HBAR, lane_color[lane], 0, 1);
        sprite_set_pos(15 + lane, HIT_ROW2, x);
        sprite_set_cfg(15 + lane, SPR_HBAR, lane_color[lane], 0, 1);
    }

    // Dotted separators: 4 columns × 3 rows = 12 sprites (20–31)
    // Vertical Separator sprite (ID 103) = 4px wide, 32px tall.
    // x is centered on the lane boundary (±2px).
    int dot_ys[3] = { DOT_Y0, DOT_Y1, DOT_Y2 };
    for (int row = 0; row < 3; row++) {
        for (int gap = 0; gap < 4; gap++) {
            int spr = 20 + row * 4 + gap;
            sprite_set_pos(spr, dot_ys[row], SEP_X(gap));
            sprite_set_cfg(spr, SPR_VSEP, SEP_COLOR, 0, 1);
        }
    }
}

// =============================================================
// Main
// =============================================================

int main(void) {
    vga_wr(REG_MODE, 0x00);
    vga_wr(REG_BG_COLOR, BG_WHITE);

    setup_static_sprites();

    int note_y[N_LANES];
    for (int lane = 0; lane < N_LANES; lane++)
        note_y[lane] = note_init_y[lane];

    while (1) {
        for (int lane = 0; lane < N_LANES; lane++) {
            note_y[lane] += note_speed[lane];

            if (note_y[lane] >= NOTE_WRAP_Y)
                note_y[lane] = -8;

            int visible = (note_y[lane] >= 0);
            int draw_y  = visible ? note_y[lane] : 0;

            sprite_set_cfg(lane, SPR_HBAR, lane_color[lane], 0, visible);
            sprite_set_pos(lane, draw_y, NOTE_X(lane));
        }

        delay(50000);
    }

    return 0;
}
