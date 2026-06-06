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

typedef enum {
    GAME_STATE_START = 0,
    GAME_STATE_PLAYING,
    GAME_STATE_END
} GameState;

#define GAME_LOOP_DELAY       50000u
#define AUDIO_DEFAULT_VOLUME  8u

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

static bool lane_hit_locked[NUMBER_INPUT_LANES] = {
    false, false, false, false
};

#define AUDIO_SUSTAIN_TICKS 5000u
static uint32_t lane_sustain[NUMBER_INPUT_LANES] = {0, 0, 0, 0};

static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm__ volatile ("nop");
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
}

static void reset_gameplay(void)
{
    score_reset();
    audio_silence();
    input_clear_all_edges();

    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        lane_hit_locked[lane] = false;
    }

    key_init_keys();
    key_draw_all();

    note_init_notes();
}

//static void process_note_hits(uint32_t presses)
//{
//    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
//
//        /*
//         * Unlock the lane only once there are no hittable notes left
//         * in that lane. This prevents rapid repeated key presses from
//         * scoring multiple notes stacked in the same hit window.
//         */
//        if (!note_lane_hit_check(lane)) {
//            lane_hit_locked[lane] = false;
//        }
//
//        if (!(presses & lane_masks[lane])) {
//            continue;
//        }
//
//        if (lane_hit_locked[lane]) {
//            continue;
//        }
//
//        if (note_process_hit(lane)) {
//            lane_hit_locked[lane] = true;
//
//            score_register_hit();
//
//            lane_sustain[lane] = AUDIO_SUSTAIN_TICKS;
//            audio_set_voice(lane_voices[lane], 1);
//        } else {
//            score_register_miss();
//        }
//    }
//}

/* ---------------------------------------------------------------------------
 * Per-lane FSM state
 * ---------------------------------------------------------------------------
 * LANE_IDLE          – no active hit; lane is ready to accept a new press.
 * LANE_HIT_DETECTED  – a hit was registered this press; lane is locked until
 *                      the button is released (press bit clears for this lane).
 * ---------------------------------------------------------------------------
 */
typedef enum {
    LANE_IDLE,
    LANE_HIT_DETECTED,
} LaneState;

static LaneState lane_state[NUMBER_INPUT_LANES];

static void process_note_hits(uint32_t presses) {
    for (uint8_t lane = 0; lane < NUMBER_INPUT_LANES; lane++) {

        bool button_pressed = (presses & lane_masks[lane]) != 0;

        switch (lane_state[lane]) {
        case LANE_IDLE:
            if (!button_pressed) {
                break;
            }
            
            if (note_process_hit(lane)) {
                key_update_sprite(lane, SPRITE_FORM_CHORD_CIRCLE_SOLID, VGA_SPRITE_32x32, VGA_GREEN);
                lane_state[lane] = LANE_HIT_DETECTED;
                score_register_hit();
                lane_sustain[lane] = AUDIO_SUSTAIN_TICKS;
                audio_set_voice(lane_voices[lane], 1);
            } else {
                key_update_sprite(lane, SPRITE_FORM_CHORD_CIRCLE_SOLID, VGA_SPRITE_32x32, VGA_RED);
                score_register_miss();
            }
            break;

        case LANE_HIT_DETECTED:
            if (button_pressed) {
                break;
            }

            if (!button_pressed) {
                key_update_sprite(lane, SPRITE_FORM_CHORD_CIRCLE_HOLLOW, VGA_SPRITE_32x32, lane_color_palette[lane]);
            }
            break;
        default:
            /* Unreachable – reset to safe state. */
            lane_state[lane] = LANE_IDLE;
            break;
        }
    }
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
    note_movement_routine();

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
        game_state = GAME_STATE_END;
    }
}

static void end_screen_update(void)
{
    audio_silence();

    uint32_t presses = input_poll_new_presses();

    if (presses & INPUT_LANE_4) {
        game_state = GAME_STATE_START;
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

        //delay(GAME_LOOP_DELAY);
    }

    return 0;
}
