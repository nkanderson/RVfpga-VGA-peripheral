////////////////////////////////////////////////////////////////////////////////
// File Name: main.c
// Project Name: Note Feller
//
// Description:
//   Main gameplay driver for Note Feller.
//   Owns high-level game state, input polling, score/audio integration,
//   and placeholder hooks for unfinished graphics/note/key systems.
////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>

#include "input_controller.h"
#include "audio.h"
#include "score.h"
#include "seven_segment.h"
#include "vga_sprite.h"
#include "note.h"
#include "key.h"

typedef enum {
    GAME_STATE_START = 0,
    GAME_STATE_PLAYING,
    GAME_STATE_END
} GameState;

#define GAME_TICK_DELAY 50000u
#define AUDIO_DEFAULT_VOLUME 8u

static GameState game_state = GAME_STATE_START;

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

    // TODO: Initialize VGA once graphics API is finalized.
    // vga_init();
    // TODO: Initialize finalized note/key systems here.
}

static void start_screen_update(void)
{
    // TODO: Draw start menu/title screen with VGA.
    // TODO: Show "Press Enter" to start.

    uint32_t presses = input_poll_new_presses();

    if (presses & INPUT_LANE_4) {
        score_reset();
        audio_silence();

        // TODO: Reset note chart / song position here.

        game_state = GAME_STATE_PLAYING;
    }
}

static void playing_update(void)
{
    uint32_t presses = input_poll_new_presses();

    // TODO: Replace this placeholder with finalized gameplay engine:
    // - spawn notes according to song/chart timing
    // - update active note positions
    // - compare presses against active notes
    // - call score_register_hit() on hit
    // - call score_register_miss() on miss
    // - call audio_set_voice(...) for hit feedback
    // - update VGA sprites for notes/lanes

    if (presses & INPUT_LANE_0) {
        score_register_hit();
        audio_silence();
        audio_set_voice(AUDIO_VOICE_C4, 1);
    }

    if (presses & INPUT_LANE_1) {
        score_register_hit();
        audio_silence();
        audio_set_voice(AUDIO_VOICE_D4, 1);
    }

    if (presses & INPUT_LANE_2) {
        score_register_hit();
        audio_silence();
        audio_set_voice(AUDIO_VOICE_E4, 1);
    }

    if (presses & INPUT_LANE_3) {
        score_register_hit();
        audio_silence();
        audio_set_voice(AUDIO_VOICE_F4, 1);
    }

    if (presses & INPUT_LANE_4) {
        score_register_hit();
        audio_silence();
        audio_set_voice(AUDIO_VOICE_G4, 1);
    }

    // TODO: Replace this with real end condition:
    // if (song_finished && no_active_notes) {
    //     game_state = GAME_STATE_END;
    // }
}

static void end_screen_update(void)
{
    audio_silence();

    // TODO: Draw final score / restart screen with VGA.
    // TODO: Optionally display final score on seven segment.

    uint32_t presses = input_poll_new_presses();

    if (presses & INPUT_LANE_4) {
        score_reset();

        // TODO: Reset chart/song/note state here.

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

        delay(GAME_TICK_DELAY);
    }

    return 0;
}