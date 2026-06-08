////////////////////////////////////////////////////////////////////////////////
// File Name: main.c
// Project Name: Note Feller
//
// Description:
//   Main gameplay driver for Note Feller.
//   Owns high-level game state, input polling, score/audio integration,
//   and coordination between note spawning, note movement, and hit checking.
//
// Controls:
//   W/A/S/D  -> gameplay lanes
//   Enter    -> start game / end game / return to start
////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdbool.h>

#include "globals.h"
#include "input_controller.h"
#include "audio.h"
#include "score.h"
#include "vga_sprite.h"
#include "note.h"
#include "key.h"
#include "menu.h"

typedef enum {
    GAME_STATE_START = 0,
    GAME_STATE_PLAYING,
    GAME_STATE_END
} GameState;

/*
 * Per-lane input state.
 *
 * LANE_IDLE:
 *   The lane is ready to accept a new key press.
 *
 * LANE_HIT_DETECTED:
 *   A press has already been processed for this lane.
 *   The lane remains locked until the key is released.
 */
typedef enum {
    LANE_IDLE = 0,
    LANE_HIT_DETECTED
} LaneState;

#define AUDIO_DEFAULT_VOLUME  8u
#define AUDIO_SUSTAIN_TICKS   5000u

static GameState game_state = GAME_STATE_START;

static const uint32_t lane_masks[NUMBER_INPUT_LANES] = {
    INPUT_LANE_0,
    INPUT_LANE_1,
    INPUT_LANE_2,
    INPUT_LANE_3
};

static const uint8_t lane_voices[NUMBER_INPUT_LANES] = {
    AUDIO_VOICE_C4,
    AUDIO_VOICE_D4,
    AUDIO_VOICE_E4,
    AUDIO_VOICE_F4
};

static LaneState lane_state[NUMBER_INPUT_LANES] = {
    LANE_IDLE, LANE_IDLE, LANE_IDLE, LANE_IDLE
};

static uint32_t lane_sustain[NUMBER_INPUT_LANES] = {
    0, 0, 0, 0
};

static void reset_lane_visual(uint8_t lane)
{
    key_update_sprite(
        lane,
        SPRITE_FORM_CHORD_CIRCLE_HOLLOW,
        VGA_SPRITE_32x32,
        lane_color_palette[lane]
    );
}

static void set_lane_hit_visual(uint8_t lane)
{
    key_update_sprite(
        lane,
        SPRITE_FORM_CHORD_CIRCLE_SOLID,
        VGA_SPRITE_32x32,
        lane_color_palette[lane]
    );
}

static void set_lane_miss_visual(uint8_t lane)
{
    key_update_sprite(
        lane,
        SPRITE_FORM_CHORD_CIRCLE_SOLID,
        VGA_SPRITE_32x32,
        VGA_BLACK
    );
}

static void process_note_hits(uint32_t presses)
{
    uint32_t held = input_get_status();

    for (uint8_t lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        bool new_press = (presses & lane_masks[lane]) != 0;
        bool is_held   = (held    & lane_masks[lane]) != 0;

        /*
         * Once the key is released, return the lane target to its normal
         * color and allow the next press to be processed.
         */
        if (!is_held) {
            reset_lane_visual(lane);
            lane_state[lane] = LANE_IDLE;
            continue;
        }

        /*
         * Holding a key should not repeatedly score or miss.
         * Only the new-press edge should be processed.
         */
        if (!new_press) {
            continue;
        }

        if (lane_state[lane] != LANE_IDLE) {
            continue;
        }

        if (note_process_hit(lane)) {
            set_lane_hit_visual(lane);

            lane_state[lane] = LANE_HIT_DETECTED;

            score_register_hit();

            lane_sustain[lane] = AUDIO_SUSTAIN_TICKS;
            audio_set_voice(lane_voices[lane], 1);
        } else {
            set_lane_miss_visual(lane);

            lane_state[lane] = LANE_HIT_DETECTED;

            score_register_miss();
        }
    }
}

static void system_init(void)
{
    input_init();
    audio_init(AUDIO_DEFAULT_VOLUME);
    score_init();
    vga_init();

    audio_silence();

    key_init_keys();
    key_draw_all();

    note_init_notes();

    game_state = GAME_STATE_START;
    menu_start_screen();
}

static void reset_gameplay(void)
{
    vga_clear_all_sprites();
    score_reset();
    audio_silence();
    input_clear_all_edges();

    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        lane_state[lane] = LANE_IDLE;
        lane_sustain[lane] = 0;
        reset_lane_visual((uint8_t)lane);
    }

    key_init_keys();
    key_draw_all();

    note_init_notes();
}

static void start_screen_update(void)
{
    uint32_t presses = input_poll_new_presses();

    if (presses & INPUT_LANE_4) {
        reset_gameplay();
        game_state = GAME_STATE_PLAYING;
    }
}

static void playing_update(void)
{
    uint32_t presses = input_poll_new_presses();

    note_spawn_routine();

    uint32_t missed_notes = note_movement_routine();

    if (missed_notes != 0) {
        score_register_miss();
    }

    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        if (lane_sustain[lane] > 0) {
            if (--lane_sustain[lane] == 0) {
                audio_set_voice(lane_voices[lane], 0);
            }
        }
    }

    process_note_hits(presses);

    if (presses & INPUT_LANE_4) {
        audio_silence();
        menu_end_screen();
        game_state = GAME_STATE_END;
    }
}

static void end_screen_update(void)
{
    audio_silence();

    uint32_t presses = input_poll_new_presses();

    if (presses & INPUT_LANE_4) {
        reset_gameplay();
        game_state = GAME_STATE_PLAYING;
    }
}

int main(void)
{
    system_init();

    while (1) {
        switch (game_state) {
            case GAME_STATE_START:
                start_screen_update();
                break;

            case GAME_STATE_PLAYING:
                playing_update();
                break;

            case GAME_STATE_END:
                end_screen_update();
                break;

            default:
                game_state = GAME_STATE_START;
                break;
        }
    }

    return 0;
}
