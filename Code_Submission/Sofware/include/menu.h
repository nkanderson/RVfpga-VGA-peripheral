////////////////////////////////////////////////////////////////////////////////
// Engineer:     Niklas Anderson & Sajida Sayyad
// Create Date:  06/06/2026
// File Name:    menu.h
// Project Name: Note Feller
// Description:  Menu management and on-screen score display for the game.
//
// LLM Acknowledgment: Parts of this code were generated with the assistance 
// of a large language model, which provided the initial structure and logic.
// The final implementation was reviewed and edited by the author to ensure
// correctness and functionality on the target hardware platform.
////////////////////////////////////////////////////////////////////////////////

#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include "globals.h"
#include "vga_sprite.h"
#include "score.h"

#define CHAR_W   16   // pixel width of a 16x16 glyph sprite

#define SCORE_STR_MAX_LEN 11   // up to 10 decimal digits (UINT32_MAX = 4294967295) + null terminator

// --- Sprite register allocations ---
// Start and end screens never display simultaneously, so they share register bases.
#define MENU_REG_HEADING        0   // "NOTE FELLER" / "GAME OVER"     up to 11 chars → 0–10
#define MENU_REG_TITLE_CIRCLES 11   // 4 decorative note circles (start only)         → 11–14
#define MENU_REG_FINAL_LABEL    9   // "FINAL SCORE:" (end only)        12 chars       → 9–20
#define MENU_REG_FINAL_VALUE   21   // final score digits (end only)    up to 10       → 21–30
#define MENU_REG_PROMPT        31   // "PRESS ENTER TO ..." (both)      up to 22 chars → 31–52

// Gameplay HUD — only active during GAME_STATE_PLAYING
#define MENU_REG_SCORE_LABEL   40   // "SCORE:"                         6 chars        → 40–45
#define MENU_REG_SCORE_VALUE   46   // gameplay score digits            up to 10       → 46–55

// --- Shared layout y positions (start and end screens use the same rows) ---
#define SCREEN_HEADING_Y       50   // "NOTE FELLER" / "GAME OVER"
#define SCREEN_PROMPT_Y       240   // "PRESS ENTER TO START" / "PRESS ENTER TO RESTART"

// --- Title screen x positions (strings centered on 640px screen) ---
#define TITLE_X               232   // "NOTE FELLER"           (11 chars): (640 - 11 * CHAR_W) / 2
#define START_PROMPT_X        160   // "PRESS ENTER TO START"  (20 chars): (640 - 20 * CHAR_W) / 2

// Decorative note circles beneath the title — one per gameplay lane, drawn
// with the same sprite form, size, and lane colors as in-game notes.
// 4 circles + 3 gaps = 4*32 + 3*16 = 176px, the same width as "NOTE FELLER",
// so starting at TITLE_X centers the row beneath the title.
#define TITLE_CIRCLE_W         32   // VGA_SPRITE_32x32 width/height
#define TITLE_CIRCLE_GAP       16   // gap between circles
#define TITLE_CIRCLES_X        TITLE_X
#define TITLE_CIRCLES_Y       100   // row beneath "NOTE FELLER"

// --- End screen x positions ---
#define GAMEOVER_X            248   // "GAME OVER"              (9 chars): (640 -  9 * CHAR_W) / 2
#define FINAL_SCORE_LABEL_X   224   // "FINAL SCORE:"          (12 chars): (640 - 12 * CHAR_W) / 2
#define FINAL_SCORE_LABEL_LEN  12   // length of "FINAL SCORE:" — used to right-align the value beneath it
#define FINAL_SCORE_LABEL_Y   150
#define FINAL_SCORE_VALUE_Y   175   // separate row to avoid sprite overlap with label
#define END_PROMPT_X          144   // "PRESS ENTER TO RESTART" (22 chars): (640 - 22 * CHAR_W) / 2

// --- Gameplay HUD ---
// Lower-left corner, sharing its row with the combo multiplier indicator
// (see score.c, score_state.sprite[].pos_y). Positioned on the left so the
// full 10-digit uint32_t range can be displayed without crowding the screen edge.
#define SCORE_LABEL_X         16    // "SCORE:" (6 chars), small margin from the left edge
#define SCORE_VALUE_X         (SCORE_LABEL_X + 6 * CHAR_W)   // 112, starts after the ':' glyph
#define SCORE_Y               450   // matches the combo indicator's pos_y

void menu_start_screen(void);
void menu_draw_score(void);
void menu_end_screen(void);

#endif // MENU_H
