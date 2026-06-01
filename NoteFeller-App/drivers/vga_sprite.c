////////////////////////////////////////////////////////////////////////////////
// Engineer: Jacob Burtenshaw
// Create Date: 05/25/2026
// File Name: vga_sprite.c
// Project Name: Note Feller
//
// Description:
//   Software driver for the Note Feller wb_vga peripheral.
//   This file abstracts the memory-mapped sprite registers behind a small API
//   that game code can call without depending on raw hardware addresses or
//   register-packing details.
//
//   Up to 32 independent sprites are supported simultaneously. Sprite 0 has
//   the highest display priority; sprite 31 has the lowest.
//
// AI Contributions:
//   Copilot generated the full contents of this file.
//
// Register Map:
//   BASE + 0x000 : REG_MODE
//                  bit[0] = display mode. 0 = graphics, 1 = text overlay.
//
//   BASE + 0x014 : REG_BG_COLOR
//                  bits[11:0] = background fill color (4-bit R, G, B channels).
//
//   BASE + 0x080 + n*8 : SPRITE_POS[n]
//                  bits[9:0]  = x position (pixels from left edge).
//                  bits[19:10] = y position (pixels from top edge).
//
//   BASE + 0x084 + n*8 : SPRITE_CFG[n]
//                  bit[0]     = visible.
//                  bit[1]     = sprite_type (0 = 32x32, 1 = 16x16).
//                  bits[15:4] = color packed as {R[15:12], G[11:8], B[7:4]}.
//                  bits[31:25] = sprite_id (7-bit index into sprite ROM).
////////////////////////////////////////////////////////////////////////////////

#include "vga_sprite.h"

// -----------------------------------------------------------------------------
// vga_init
//
// Puts the VGA peripheral into graphics mode with a white background and
// hides all 32 sprite registers. Call this once at startup before drawing
// any sprites.
// -----------------------------------------------------------------------------

void vga_init(void)
{
    VGA_REG_MODE = VGA_MODE_GRAPHICS;
    vga_set_bg_color(VGA_WHITE);
    vga_clear_all_sprites();
}

// -----------------------------------------------------------------------------
// vga_set_sprite
//
// Configures one sprite register and makes it visible.
//
//   sprite->reg         : sprite slot index [0, 63]. 0 has the highest draw priority.
//   sprite->sprite_id   : index into the sprite ROM (use SPRITE_FORM_* constants).
//   sprite->sprite_type : VGA_SPRITE_32x32 or VGA_SPRITE_16x16.
//   sprite->color       : 12-bit packed color from VGA_COLOR(r, g, b).
//   sprite->pos_x       : left-edge pixel column [0, 639].
//   sprite->pos_y       : top-edge pixel row    [0, 479].
// -----------------------------------------------------------------------------

void vga_set_sprite(const Sprite *sprite)
{
    int reg = sprite->reg;
    if (reg < 0 || reg >= VGA_N_SPRITES) {
        return;
    }

    VGA_SPRITE_POS(reg) = ((uint32_t)(sprite->pos_y & 0x3FF) << 10) | (uint32_t)(sprite->pos_x & 0x3FF);

    VGA_SPRITE_CFG(reg) = ((uint32_t)(sprite->sprite_id   & 0x7F)  << 25)
                        | ((uint32_t)(sprite->color        & 0xFFF) << 4)
                        | ((uint32_t)(sprite->sprite_type  & 0x1)   << 1)
                        | 0x1U;
}

// -----------------------------------------------------------------------------
// vga_clear_sprite
//
// Hides a single sprite register without changing its other fields.
//
//   reg : sprite slot index [0, 31].
// -----------------------------------------------------------------------------

void vga_clear_sprite(int reg)
{
    if (reg < 0 || reg >= VGA_N_SPRITES) {
        return;
    }

    VGA_SPRITE_CFG(reg) &= ~0x1U;
}

// -----------------------------------------------------------------------------
// vga_clear_all_sprites
//
// Hides all 32 sprite registers. Zeroing the entire CFG word is safe because
// a zero CFG has visible=0.
// -----------------------------------------------------------------------------

void vga_clear_all_sprites(void)
{
    int i;
    for (i = 0; i < VGA_N_SPRITES; i++) {
        VGA_SPRITE_CFG(i) = 0;
    }
}

// -----------------------------------------------------------------------------
// vga_set_bg_color
//
// Updates the background fill color shown wherever no visible sprite covers
// the pixel.
//
//   color : 12-bit packed color from VGA_COLOR(r, g, b).
// -----------------------------------------------------------------------------

void vga_set_bg_color(uint16_t color)
{
    VGA_REG_BG_COLOR = (uint32_t)(color & 0xFFF);
}
