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

static GameState game_state = GAME_STATE_START;

static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm__ volatile ("nop");
    }
}

static void system_init(void)
{
    game_state = GAME_STATE_START;
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

    key_init_keys();
    key_draw_all();

    note_init_notes();

    // TODO: Reset real song/chart state here.
}

static void sync_key_hittable_flags(void)
{
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        bool lane_hittable = note_lane_hit_check(lane);
        key_set_hittable(&keys[lane], lane_hittable);
    }
}

static void process_note_hits(uint32_t presses)
{
    // For each gameplay lane, check if the button was pressed this tick.
    // key_try_hit returns true only when the lane is hittable and not latched,
    // so a press without a hittable note in the hit box registers as a miss.
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        if (!(presses & keys[lane].button)) { continue; }

        if (key_try_hit(&keys[lane], presses)) {
            note_process_hit(lane);
            score_register_hit();
            audio_silence();
            audio_set_voice(keys[lane].audio_voice, 1);
        } else {
            score_register_miss();
        }
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

static void playing_update(void) {
    uint32_t presses = input_poll_new_presses(); // Latch new button edges for this tick

    // Spawn a note if possible
    note_spawn_routine();
    // Move all active notes down; deactivate any that exit the screen
    note_movement_routine(); 
    // Set each key's hittable flag based on current note positions
    sync_key_hittable_flags(); 
    // Register hits or misses for any lane buttons pressed this tick
    process_note_hits(presses); 
    // Re-sync after hits so consumed notes immediately clear their key flag
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