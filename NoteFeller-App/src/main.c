////////////////////////////////////////////////////////////////////////////////
// File Name: main.c
// Project Name: Note Feller
//
// Description:
//   Main gameplay driver for Note Feller.
//   Owns high-level game state, input polling, score/audio integration,
//   and coordination between notes, keys, and VGA.
////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>

#include "globals.h"
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

#define GAME_LOOP_DELAY       50000u
#define AUDIO_DEFAULT_VOLUME  8u

// Temporary demo spawn timing.
// TODO: Replace with real chart/song timing.
#define DEMO_SPAWN_THRESHOLD  150u

static GameState game_state = GAME_STATE_START;
static uint32_t demo_spawn_counter = 0;

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

    key_init_keys();
    key_draw_all();

    note_init_notes();
}

static void reset_gameplay(void)
{
    score_reset();
    audio_silence();
    input_clear_all_edges();

    demo_spawn_counter = 0;

    key_init_keys();
    key_draw_all();

    note_init_notes();

    // TODO: Reset real song/chart state here.
}

static void update_all_notes(void)
{
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        for (int i = 0; i < NOTES_PER_LANE; i++) {
            note_movement_routine(&notes[lane][i]);
        }
    }
}

static void sync_key_hittable_flags(void)
{
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        uint8_t lane_hittable = 0;

        for (int i = 0; i < NOTES_PER_LANE; i++) {
            if (notes[lane][i].active && notes[lane][i].hittable) {
                lane_hittable = 1;
                break;
            }
        }

        key_set_hittable(&keys[lane], lane_hittable);
    }
}

static void process_note_hits(uint32_t presses)
{
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        for (int i = 0; i < NOTES_PER_LANE; i++) {
            Note* note = &notes[lane][i];

            if (!note->active || !note->hittable) {
                continue;
            }

            if (key_try_hit(&keys[lane], presses)) {
                note->active = 0;
                note->hittable = 0;

                vga_clear_sprite(note->sprite.reg);

                score_register_hit();

                audio_silence();
                audio_set_voice(keys[lane].audio_voice, 1);

                break;
            }
        }
    }
}

static void process_note_misses(void)
{
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        for (int i = 0; i < NOTES_PER_LANE; i++) {
            Note* note = &notes[lane][i];

            if (!note->active) {
                continue;
            }

            if (note->y > (KEY_Y + KEY_SPRITE_H)) {
                note->active = 0;
                note->hittable = 0;

                vga_clear_sprite(note->sprite.reg);

                score_register_miss();
            }
        }
    }
}

static void demo_spawn_update(void)
{
    // TODO: Replace with real chart/song note spawning.
    demo_spawn_counter++;

    if (demo_spawn_counter >= DEMO_SPAWN_THRESHOLD) {
        demo_spawn_counter = 0;
        note_spawn_note();
    }
}

static void start_screen_update(void)
{
    // TODO: Draw start menu/title screen with VGA.
    // TODO: Show "Press Enter" to start.

    uint32_t presses = input_poll_new_presses();

    if (presses & INPUT_LANE_4) {
        reset_gameplay();
        game_state = GAME_STATE_PLAYING;
    }
}

static void playing_update(void)
{
    uint32_t presses = input_poll_new_presses();

    demo_spawn_update();

    update_all_notes();

    sync_key_hittable_flags();

    process_note_hits(presses);

    process_note_misses();

    sync_key_hittable_flags();

    // Temporary way to end game.
    // TODO: Replace with real end condition.
    if (presses & INPUT_LANE_4) {
        game_state = GAME_STATE_END;
    }
}

static void end_screen_update(void)
{
    audio_silence();

    // TODO: Draw final score / restart screen with VGA.
    // TODO: Display final score visually.

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