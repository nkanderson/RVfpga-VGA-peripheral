////////////////////////////////////////////////////////////////////////////////
// Author: Jacob Burtenshaw
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
//   Copilot generated the contents of this file.
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
// See the SPRITE_FORM_* section below for the full descriptive macro set.
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// SPRITE_FORM_* — descriptive macros for every entry in guitarSprites.coe.
//
// IDs 0–94 map to bigfont glyphs (ASCII 32–126); sprite_id = ASCII_code - 32.
// Use VGA_SPRITE_16x16 type for all glyph characters.
//
// IDs 95–111 are the guitar game sprites.
// Use VGA_SPRITE_16x16 for NOTE_CIRCLE_* and SQUIGGLE_* (16px wide bitmaps).
// All others use VGA_SPRITE_32x32.
// -----------------------------------------------------------------------------

// --- IDs 0–94: bigfont glyph characters (ASCII 32–126) ---
#define SPRITE_FORM_SPACE                        0   // ' '
#define SPRITE_FORM_EXCLAMATION                  1   // '!'
#define SPRITE_FORM_QUOTE                        2   // '"'
#define SPRITE_FORM_HASH                         3   // '#'
#define SPRITE_FORM_DOLLAR                       4   // '$'
#define SPRITE_FORM_PERCENT                      5   // '%'
#define SPRITE_FORM_AMPERSAND                    6   // '&'
#define SPRITE_FORM_APOSTROPHE                   7   // '\''
#define SPRITE_FORM_PAREN_LEFT                   8   // '('
#define SPRITE_FORM_PAREN_RIGHT                  9   // ')'
#define SPRITE_FORM_ASTERISK                     10  // '*'
#define SPRITE_FORM_PLUS                         11  // '+'
#define SPRITE_FORM_COMMA                        12  // ','
#define SPRITE_FORM_MINUS                        13  // '-'
#define SPRITE_FORM_PERIOD                       14  // '.'
#define SPRITE_FORM_SLASH                        15  // '/'
#define SPRITE_FORM_0                            16  // '0'
#define SPRITE_FORM_1                            17  // '1'
#define SPRITE_FORM_2                            18  // '2'
#define SPRITE_FORM_3                            19  // '3'
#define SPRITE_FORM_4                            20  // '4'
#define SPRITE_FORM_5                            21  // '5'
#define SPRITE_FORM_6                            22  // '6'
#define SPRITE_FORM_7                            23  // '7'
#define SPRITE_FORM_8                            24  // '8'
#define SPRITE_FORM_9                            25  // '9'
#define SPRITE_FORM_COLON                        26  // ':'
#define SPRITE_FORM_SEMICOLON                    27  // ';'
#define SPRITE_FORM_LESS_THAN                    28  // '<'
#define SPRITE_FORM_EQUALS                       29  // '='
#define SPRITE_FORM_GREATER_THAN                 30  // '>'
#define SPRITE_FORM_QUESTION                     31  // '?'
#define SPRITE_FORM_AT                           32  // '@'
#define SPRITE_FORM_A                            33  // 'A'
#define SPRITE_FORM_B                            34  // 'B'
#define SPRITE_FORM_C                            35  // 'C'
#define SPRITE_FORM_D                            36  // 'D'
#define SPRITE_FORM_E                            37  // 'E'
#define SPRITE_FORM_F                            38  // 'F'
#define SPRITE_FORM_G                            39  // 'G'
#define SPRITE_FORM_H                            40  // 'H'
#define SPRITE_FORM_I                            41  // 'I'
#define SPRITE_FORM_J                            42  // 'J'
#define SPRITE_FORM_K                            43  // 'K'
#define SPRITE_FORM_L                            44  // 'L'
#define SPRITE_FORM_M                            45  // 'M'
#define SPRITE_FORM_N                            46  // 'N'
#define SPRITE_FORM_O                            47  // 'O'
#define SPRITE_FORM_P                            48  // 'P'
#define SPRITE_FORM_Q                            49  // 'Q'
#define SPRITE_FORM_R                            50  // 'R'
#define SPRITE_FORM_S                            51  // 'S'
#define SPRITE_FORM_T                            52  // 'T'
#define SPRITE_FORM_U                            53  // 'U'
#define SPRITE_FORM_V                            54  // 'V'
#define SPRITE_FORM_W                            55  // 'W'
#define SPRITE_FORM_X                            56  // 'X'
#define SPRITE_FORM_Y                            57  // 'Y'
#define SPRITE_FORM_Z                            58  // 'Z'
#define SPRITE_FORM_BRACKET_LEFT                 59  // '['
#define SPRITE_FORM_BACKSLASH                    60  // '\\'
#define SPRITE_FORM_BRACKET_RIGHT                61  // ']'
#define SPRITE_FORM_CARET                        62  // '^'
#define SPRITE_FORM_UNDERSCORE                   63  // '_'
#define SPRITE_FORM_GRAVE                        64  // '`'
#define SPRITE_FORM_a                            65  // 'a'
#define SPRITE_FORM_b                            66  // 'b'
#define SPRITE_FORM_c                            67  // 'c'
#define SPRITE_FORM_d                            68  // 'd'
#define SPRITE_FORM_e                            69  // 'e'
#define SPRITE_FORM_f                            70  // 'f'
#define SPRITE_FORM_g                            71  // 'g'
#define SPRITE_FORM_h                            72  // 'h'
#define SPRITE_FORM_i                            73  // 'i'
#define SPRITE_FORM_j                            74  // 'j'
#define SPRITE_FORM_k                            75  // 'k'
#define SPRITE_FORM_l                            76  // 'l'
#define SPRITE_FORM_m                            77  // 'm'
#define SPRITE_FORM_n                            78  // 'n'
#define SPRITE_FORM_o                            79  // 'o'
#define SPRITE_FORM_p                            80  // 'p'
#define SPRITE_FORM_q                            81  // 'q'
#define SPRITE_FORM_r                            82  // 'r'
#define SPRITE_FORM_s                            83  // 's'
#define SPRITE_FORM_t                            84  // 't'
#define SPRITE_FORM_u                            85  // 'u'
#define SPRITE_FORM_v                            86  // 'v'
#define SPRITE_FORM_w                            87  // 'w'
#define SPRITE_FORM_x                            88  // 'x'
#define SPRITE_FORM_y                            89  // 'y'
#define SPRITE_FORM_z                            90  // 'z'
#define SPRITE_FORM_BRACE_LEFT                   91  // '{'
#define SPRITE_FORM_PIPE                         92  // '|'
#define SPRITE_FORM_BRACE_RIGHT                  93  // '}'
#define SPRITE_FORM_TILDE                        94  // '~'

// --- IDs 95–111: guitar game sprites ---
#define SPRITE_FORM_CHORD_CIRCLE_SOLID           95  // 32x32 large circle, filled
#define SPRITE_FORM_CHORD_CIRCLE_HOLLOW          96  // 32x32 large circle, outline
#define SPRITE_FORM_NOTE_CIRCLE_SOLID            97  // 16x16 small circle, filled  — use VGA_SPRITE_16x16
#define SPRITE_FORM_NOTE_CIRCLE_HOLLOW           98  // 16x16 small circle, outline — use VGA_SPRITE_16x16
#define SPRITE_FORM_SQUIGGLE_SOLID               99  // 16x32 squiggly trail, filled
#define SPRITE_FORM_SQUIGGLE_HOLLOW              100 // 16x32 squiggly trail, outline
#define SPRITE_FORM_RECTANGLE_SOLID              101 // 32x8  horizontal bar, filled
#define SPRITE_FORM_RECTANGLE_HOLLOW             102 // 32x8  horizontal bar, outline
#define SPRITE_FORM_SEPARATOR_SOLID              103 // 4x32  vertical separator, filled
#define SPRITE_FORM_SEPARATOR_HOLLOW             104 // 4x32  vertical separator, outline
#define SPRITE_FORM_CORNER_TOP_LEFT              105 // 8x8   top-left corner
#define SPRITE_FORM_CORNER_TOP_RIGHT             106 // 8x8   top-right corner
#define SPRITE_FORM_CORNER_BOTTOM_LEFT           107 // 8x8   bottom-left corner
#define SPRITE_FORM_CORNER_BOTTOM_RIGHT          108 // 8x8   bottom-right corner
#define SPRITE_FORM_BORDER_HORIZONTAL            109 // 32x2  horizontal border tile
#define SPRITE_FORM_BORDER_VERTICAL_LEFT         110 // 2x32  left vertical border
#define SPRITE_FORM_BORDER_VERTICAL_RIGHT        111 // 2x32  right vertical border

// -----------------------------------------------------------------------------
// Sprite descriptor struct
//
// Groups all parameters needed to configure one sprite register.
// Pass to vga_set_sprite_s() as an alternative to the individual-argument form.
// -----------------------------------------------------------------------------

typedef struct {
    uint8_t  sprite_id;   // ROM index — use SPRITE_FORM_* constants
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
