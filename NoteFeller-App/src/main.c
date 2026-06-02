////////////////////////////////////////////////////////////////////////////////
// File Name: main.c
// Project Name: Note Feller
//
// Description:
//   Main gameplay driver for Note Feller.
//   Owns high-level game state, input polling, score/audio integration,
//   and coordination between note spawning, note movement, and hit checking.
////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdbool.h>

#include "globals.h"
#include "input_controller.h"
#include "audio.h"
#include "score.h"
#include "vga_sprite.h"
#include "note.h"

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
    note_init_notes();
}

static void reset_gameplay(void)
{
    score_reset();
    audio_silence();
    input_clear_all_edges();
    note_init_notes();
}

static void process_note_hits(uint32_t presses)
{
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        if (!(presses & lane_masks[lane])) {
            continue;
        }

        if (note_lane_hit_check(lane)) {
            note_process_hit(lane);
            score_register_hit();

            audio_silence();
            audio_set_voice(lane_voices[lane], 1);
        } else {
            score_register_miss();
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
    process_note_hits(presses);

    if (presses & INPUT_LANE_4) {
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

        delay(GAME_LOOP_DELAY);
    }

    return 0;
}