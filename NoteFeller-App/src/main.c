////////////////////////////////////////////////////////////////////////////////
// Engineer:    Sajida Sayyad
// Create Date: 26/05/2026
// File Name:   main.c
// Project Name: Single-Lane Rhythm Game
// Description:
//   Main game logic with continuous note spawning.
//   - Game starts when BTNC is pressed.
//   - Notes spawn continuously and move slowly across LEDs (0 to 10).
//   - Player must press BTNR (4th LED), BTNU (6th LED), BTNL (8th LED), or BTND (10th LED).
//   - Successful hits trigger unique audio tones, LED feedback, and score increments.
//   - If the note is missed, it disappears, and no score is awarded.
////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include "audio.h"            // For audio control
#include "input_controller.h" // For input button masks
#include "gpio.h"             // For GPIO control
#include "globals.h"          // For game state and constants

// --- Game State (Definitions) ---
int game_started = 0;          // 1 if the game is running, 0 otherwise
int note_position = 0;         // Current LED position of the note (0-10)
int score = 0;                 // Player's score
int note_active = 0;           // 1 if a note is active, 0 otherwise

// Simple delay function for timing.
static void delay(volatile uint32_t count) {
    while (count--) { __asm__ volatile ("nop"); }
}

// Initializes the game (input, audio, GPIO).
void game_init(void) {
    input_init();              // Initialize input controller
    audio_init(8);             // Initialize audio with default volume 8
    gpio_init();               // Initialize GPIO
    game_started = 0;          // Game starts in "not started" state
    note_position = 0;         // Note starts at LED 0
    score = 0;                 // Reset score
    note_active = 0;           // No note is active at startup
    GPIO_OUTPUT = 0;           // Turn off all LEDs
}

// Blinks an LED for a short duration.
void blink_led(int led) {
    GPIO_OUTPUT |= (1U << led);  // Turn on the LED
    delay(HIT_LED_DURATION);     // Longer delay for visibility
    GPIO_OUTPUT &= ~(1U << led); // Turn off the LED
}

// Spawns a new note at LED 0.
void spawn_note(void) {
    note_active = 1;            // Activate the note
    note_position = 0;          // Start note at LED 0
    GPIO_OUTPUT = (1U << note_position);  // Light up LED 0
}

// Main game loop.
int main(void) {
    game_init();  // Initialize the game

    while (1) {
        // Start game on BTNC press (INPUT_LANE_4)
        if (!game_started && (input_poll_new_presses() & INPUT_LANE_4)) {
            game_started = 1;    // Set game as started
            spawn_note();        // Spawn the first note
        }

        if (game_started) {
            // Check if no note is active and spawn a new one
            if (!note_active) {
                spawn_note();
            }

            // Move the note to the next LED if it is active
            if (note_active) {
                note_position++;
                GPIO_OUTPUT = (1U << note_position);  // Light up the current LED

                // Check for hits at specific LED positions
                uint32_t input = input_poll_new_presses();

                // --- Hit at 4th LED (BTNR) ---
                if (note_position == 4 && (input & INPUT_LANE_1)) {
                    // Play audio tone for hit
                    audio_silence();
                    audio_set_voice(AUDIO_VOICE_C4, 1);

                    // Blink LED 11 (hit indication) and LED 15 (score increment)
                    blink_led(11);
                    blink_led(15);

                    // Update score
                    score += SCORE_PER_HIT;

                    // Deactivate the note
                    note_active = 0;
                }
                // --- Hit at 6th LED (BTNU) ---
                else if (note_position == 6 && (input & INPUT_LANE_2)) {
                    // Play audio tone for hit
                    audio_silence();
                    audio_set_voice(AUDIO_VOICE_D4, 1);

                    // Blink LED 12 (hit indication) and LED 15 (score increment)
                    blink_led(12);
                    blink_led(15);

                    // Update score
                    score += SCORE_PER_HIT;

                    // Deactivate the note
                    note_active = 0;
                }
                // --- Hit at 8th LED (BTNL) ---
                else if (note_position == 8 && (input & INPUT_LANE_0)) {
                    // Play audio tone for hit
                    audio_silence();
                    audio_set_voice(AUDIO_VOICE_E4, 1);

                    // Blink LED 13 (hit indication) and LED 15 (score increment)
                    blink_led(13);
                    blink_led(15);

                    // Update score
                    score += SCORE_PER_HIT;

                    // Deactivate the note
                    note_active = 0;
                }
                // --- Hit at 10th LED (BTND) ---
                else if (note_position == 10 && (input & INPUT_LANE_3)) {
                    // Play audio tone for hit
                    audio_silence();
                    audio_set_voice(AUDIO_VOICE_F4, 1);

                    // Blink LED 14 (hit indication) and LED 15 (score increment)
                    blink_led(14);
                    blink_led(15);

                    // Update score
                    score += SCORE_PER_HIT;

                    // Deactivate the note (will respawn in the next loop)
                    note_active = 0;
                }
                // --- Missed Note ---
                else if (note_position > MAX_LED_POSITION) {
                    note_active = 0;  // Deactivate the note
                    GPIO_OUTPUT = 0;  // Turn off all LEDs
                }
            }

            delay(NOTE_SPEED);  // Control note speed (slower)
        }
    }
    return 0;
}