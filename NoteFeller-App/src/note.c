////////////////////////////////////////////////////////////////////////////////
// Engineer:    Jacob Burtenshaw
// Create Date: 26/05/2026
// File Name:   note.c
// Project Name: Note Feller
// Description:
//   Handles note spawning, movement, and hit detection.
////////////////////////////////////////////////////////////////////////////////

#include "note.h"
#include "keys.h"       

// Define constants for note movement
#define TICK_THRESHOLD 100000 // Number of ticks before moving the note down
#define INCREMENT_Y 2

#define SPRITE_SMALL   16    // Height/width of a VGA_SPRITE_16x16 sprite in pixels
#define SPRITE_BIG     32    // Height/width of a VGA_SPRITE_32x32 sprite in pixels

// Initialize a note
void note_init(Note *note, uint8_t reg, uint8_t lane) {
    Sprite sprite;
    for (int i = 0; i < NUMBER_INPUT_LANES; i++) {
        sprite.reg         = i + NOTE_SPRITE_OFFSET;
        sprite.sprite_id   = SPRITE_FORM_NOTE_CIRCLE_SOLID;
        sprite.sprite_type = VGA_SPRITE_16x16;
        sprite.color       = lane_color_palette[lane];
        sprite.pos_x       = lane_locations[lane] + (KEY_LANE_W - SPRITE_SMALL) / 2; // Center the note in the lane
        sprite.pos_y       = KEY_Y;
    }

    note->active = 0;       // Deactivate the note
    note->hittable = 0;
    note->y = 0;            // Start at the top of screen
    note->sprite = sprite;  // Assign the sprite
    note->tick_ctr = 0;     // Reset the tick counter
}

void note_init_notes(void) {
    for (int lane = 0; lane < NUMBER_INPUT_LANES; lane++) {
        for (int i = 0; i < NOTES_PER_LANE; i++) {
            uint8_t reg = (lane * NOTES_PER_LANE) + i + NOTE_SPRITE_OFFSET; // Assign sprite register index
            note_init(&notes[lane][i], reg, lane);
        }
    }
}


// Returns true when the note has fallen past the bottom of the VGA display.
bool note_complete(uint16_t y, uint8_t sprite_height) {
    return y + sprite_height > SCREEN_H;
}

// The hit box spans [KEY_Y, KEY_Y + KEY_SPRITE_H). Because the note is
// smaller or equal to the hit box, the note is hittable once its center
// row enters the hit box and until its center row exits the bottom.
bool note_hittable_check(uint16_t y, uint8_t sprite_height) {
    uint16_t note_top    = y;
    uint16_t note_bottom = y + sprite_height;
    uint16_t hit_top     = KEY_Y;
    uint16_t hit_bottom  = KEY_Y + KEY_SPRITE_H;

    uint16_t overlap_top    = (note_top    > hit_top)    ? note_top    : hit_top;
    uint16_t overlap_bottom = (note_bottom < hit_bottom) ? note_bottom : hit_bottom;

    if (overlap_bottom <= overlap_top) {return false;}

    uint16_t overlap = overlap_bottom - overlap_top;

    return (overlap > (uint16_t)sprite_height / 2u);
}

// Update note position
void note_movement_routine(Note *note) {
    Sprite sprite = note->sprite;
    
    if(!note->active) {return;} // If the note is not active, do nothing

    if(note->tick_ctr < TICK_THRESHOLD){
        note->tick_ctr++;
        return; // Wait until the tick threshold is reached
    }

    note->tick_ctr = 0;           // Reset the tick counter
    note->y += INCREMENT_Y;       // Move the note down
    note->sprite.pos_y = note->y; // Update the sprite's y position
    vga_set_sprite(&sprite);

    note->hittable = note_hittable_check(note->y, SPRITE_SMALL); // Check if the note is hittable

    if(note_complete(note->y, SPRITE_SMALL)) {
        note->active   = 0; // Deactivate the note if it has fallen off the screen
        note->hittable = 0;
        vga_clear_sprite(sprite.reg);
    }

}


