////////////////////////////////////////////////////////////////////////////////
// Engineer: Jacob Burtenshaw
// Create Date: 05/25/2026
// File Name: vga_sprite.h
// Project Name: Note Feller
//
// Description:
//   Header file for the Note Feller VGA sprite software driver.
//   This driver provides a clean game-facing API for positioning, coloring, and
//   displaying sprites through the memory-mapped wb_vga peripheral.
//
//   The peripheral supports up to 64 independent hardware sprites rendered
//   in a single scan-line pass each frame. Sprite 0 has the highest display
//   priority; sprite 63 has the lowest.
//
//   Sprites are either 32x32 pixels (VGA_SPRITE_32x32) or 16x16 pixels
//   (VGA_SPRITE_16x16). Colors are 12-bit (4 bits per channel). Use the
//   VGA_COLOR(r, g, b) macro to build a color value from individual channels.
//
// Register Base:
//   0x80003000
//
// AI Contributions:
//   Copilot generated the full contents of this file.
////////////////////////////////////////////////////////////////////////////////

#ifndef VGA_SPRITE_H
#define VGA_SPRITE_H

#include <stdint.h>

// -----------------------------------------------------------------------------
// VGA peripheral base address and register macros
// -----------------------------------------------------------------------------

#define VGA_BASE                   0x80003000UL

#define VGA_REG_MODE              (*(volatile uint32_t *)(VGA_BASE + 0x000))
#define VGA_REG_BG_COLOR          (*(volatile uint32_t *)(VGA_BASE + 0x014))

#define VGA_SPRITE_POS(n)         (*(volatile uint32_t *)(VGA_BASE + 0x080 + (n) * 8))
#define VGA_SPRITE_CFG(n)         (*(volatile uint32_t *)(VGA_BASE + 0x084 + (n) * 8))

// -----------------------------------------------------------------------------
// Mode register bit fields
// -----------------------------------------------------------------------------

#define VGA_MODE_GRAPHICS          0x0
#define VGA_MODE_TEXT              0x1

// -----------------------------------------------------------------------------
// Color helpers
//
// VGA_COLOR packs three 4-bit channel values into the 12-bit format used by
// REG_BG_COLOR and the color field within SPRITE_CFG.
//   r, g, b must be in range [0, 15].
// -----------------------------------------------------------------------------

#define VGA_COLOR(r, g, b)         (((uint32_t)((r) & 0xF) << 8) | \
                                    ((uint32_t)((g) & 0xF) << 4) | \
                                     (uint32_t)((b) & 0xF))

#define VGA_WHITE                  VGA_COLOR(15, 15, 15)
#define VGA_BLACK                  VGA_COLOR(0,   0,  0)
#define VGA_RED                    VGA_COLOR(15,  0,  0)
#define VGA_GREEN                  VGA_COLOR(0,  15,  0)
#define VGA_BLUE                   VGA_COLOR(0,   0, 15)
#define VGA_YELLOW                 VGA_COLOR(15, 15,  0)
#define VGA_CYAN                   VGA_COLOR(0,  15, 15)
#define VGA_MAGENTA                VGA_COLOR(15,  0, 15)

// -----------------------------------------------------------------------------
// Sprite type constants
// -----------------------------------------------------------------------------

#define VGA_SPRITE_32x32           0x0
#define VGA_SPRITE_16x16           0x1

// -----------------------------------------------------------------------------
// Number of available sprite registers
// -----------------------------------------------------------------------------

#define VGA_N_SPRITES              64

// -----------------------------------------------------------------------------
// Guitar sprite ID constants
//
// These IDs index into the unified sprite ROM (guitarSprites.coe) which
// combines the bigfont glyph table (IDs 0-94) and the guitar sprites below.
// -----------------------------------------------------------------------------

#define SPR_CHORD_CIRCLE_SOLID     95
#define SPR_CHORD_CIRCLE_HOLLOW    96
#define SPR_NOTE_CIRCLE_SOLID      97   // 16x16 — use VGA_SPRITE_16x16
#define SPR_NOTE_CIRCLE_HOLLOW     98   // 16x16 — use VGA_SPRITE_16x16
#define SPR_SQUIGGLE_SOLID         99
#define SPR_SQUIGGLE_HOLLOW        100
#define SPR_HORIZ_BAR_SOLID        101
#define SPR_HORIZ_BAR_HOLLOW       102
#define SPR_VERT_SEP_SOLID         103
#define SPR_VERT_SEP_HOLLOW        104
#define SPR_CORNER_TL              105  // 8x8 top-left corner
#define SPR_CORNER_TR              106  // 8x8 top-right corner
#define SPR_CORNER_BL              107  // 8x8 bottom-left corner
#define SPR_CORNER_BR              108  // 8x8 bottom-right corner
#define SPR_HORIZ_BORDER_SOLID     109  // 32x2 horizontal border tile
#define SPR_VERT_BORDER_LEFT       110  // 2x32 left vertical border
#define SPR_VERT_BORDER_RIGHT      111  // 2x32 right vertical border

// -----------------------------------------------------------------------------
// Sprite descriptor struct
//
// Groups all parameters needed to configure one sprite register.
// Pass to vga_set_sprite_s() as an alternative to the individual-argument form.
// -----------------------------------------------------------------------------

typedef struct {
    uint8_t  sprite_id;   // ROM index — use SPR_* constants
    uint8_t  sprite_type; // VGA_SPRITE_32x32 or VGA_SPRITE_16x16
    uint16_t color;       // 12-bit color from VGA_COLOR(r, g, b)
    uint16_t pos_x;       // Left-edge pixel column [0, 639]
    uint16_t pos_y;       // Top-edge pixel row    [0, 479]
} Sprite;

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

void vga_init(void);

void vga_set_sprite(int reg, uint8_t sprite_id, uint8_t sprite_type,
                    uint16_t color, uint16_t pos_x, uint16_t pos_y);

void vga_clear_sprite(int reg);
void vga_clear_all_sprites(void);

void vga_set_bg_color(uint16_t color);

#endif
