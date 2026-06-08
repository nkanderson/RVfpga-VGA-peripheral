////////////////////////////////////////////////////////////////////////////////
// Engineer:     Niklas Anderson & Sajida Sayyad
// Create Date:  06/06/2026
// File Name:    menu.c
// Project Name: Note Feller
// Description:  Menu management and on-screen score display for the game.
//
// LLM Acknowledgment: Parts of this code were generated with the assistance 
// of a large language model, which provided the initial structure and logic.
// The final implementation was reviewed and edited by the author to ensure
// correctness and functionality on the target hardware platform.
////////////////////////////////////////////////////////////////////////////////

#include "menu.h"

static void draw_text(const char* text, uint8_t reg_start, uint16_t x, uint16_t y, uint16_t color) {
    for (int i = 0; text[i] != '\0'; i++) {
        Sprite text_sprite = {
            .reg = reg_start + i,
            .sprite_id = text[i] - ' ' + SPRITE_FORM_SPACE,
            .sprite_type = VGA_SPRITE_16x16,
            .color = color,
            .pos_x = x + (i * CHAR_W),
            .pos_y = y
        };
        vga_set_sprite(&text_sprite);
    }
}

// Draws a row of decorative note circles beneath the title, one per gameplay
// lane, using the same sprite form, size, and lane colors as in-game notes.
static void draw_title_circles(void) {
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        Sprite circle_sprite = {
            .reg = MENU_REG_TITLE_CIRCLES + lane,
            .sprite_id = SPRITE_FORM_CHORD_CIRCLE_SOLID,
            .sprite_type = VGA_SPRITE_32x32,
            .color = lane_color_palette[lane],
            .pos_x = TITLE_CIRCLES_X + lane * (TITLE_CIRCLE_W + TITLE_CIRCLE_GAP),
            .pos_y = TITLE_CIRCLES_Y
        };
        vga_set_sprite(&circle_sprite);
    }
}

// Writes the decimal digits of score into out as a null-terminated string
// and returns the digit count (excluding the null terminator).
// out must be at least SCORE_STR_MAX_LEN bytes.
static int score_to_string(uint32_t score, char* out) {
    int len = 0;

    if (score == 0) {
        out[len++] = '0';
    } else {
        char temp[SCORE_STR_MAX_LEN];
        int temp_len = 0;
        while (score > 0) {
            temp[temp_len++] = '0' + (score % 10);
            score /= 10;
        }
        for (int i = temp_len - 1; i >= 0; i--) {
            out[len++] = temp[i];
        }
    }
    out[len] = '\0';

    return len;
}

void menu_start_screen(void) {
    vga_clear_all_sprites();
    draw_text("NOTE FELLER", MENU_REG_HEADING, TITLE_X, SCREEN_HEADING_Y, VGA_BLACK);
    draw_title_circles();
    draw_text("PRESS ENTER TO START", MENU_REG_PROMPT, START_PROMPT_X, SCREEN_PROMPT_Y, VGA_GREEN);
}

void menu_draw_score(void) {
    char score_str[SCORE_STR_MAX_LEN];
    score_to_string(score_get_value(), score_str);

    draw_text("SCORE:", MENU_REG_SCORE_LABEL, SCORE_LABEL_X, SCORE_Y, VGA_BLACK);
    draw_text(score_str, MENU_REG_SCORE_VALUE, SCORE_VALUE_X, SCORE_Y, VGA_BLACK);
}

void menu_end_screen(void) {
    vga_clear_all_sprites();

    draw_text("GAME OVER", MENU_REG_HEADING, GAMEOVER_X, SCREEN_HEADING_Y, VGA_BLACK);
    draw_text("FINAL SCORE:", MENU_REG_FINAL_LABEL, FINAL_SCORE_LABEL_X, FINAL_SCORE_LABEL_Y, VGA_BLACK);

    char score_str[SCORE_STR_MAX_LEN];
    int score_len = score_to_string(score_get_value(), score_str);

    // Right-align the value's right edge with the label's right edge above it.
    uint16_t final_score_value_x = FINAL_SCORE_LABEL_X + (FINAL_SCORE_LABEL_LEN - score_len) * CHAR_W;
    draw_text(score_str, MENU_REG_FINAL_VALUE, final_score_value_x, FINAL_SCORE_VALUE_Y, VGA_BLACK);

    draw_text("PRESS ENTER TO RESTART", MENU_REG_PROMPT, END_PROMPT_X, SCREEN_PROMPT_Y, VGA_GREEN);
}
