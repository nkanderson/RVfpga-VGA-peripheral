#include <stdint.h>

// ============================================================
// Note Feller - Input Controller Test Program
// Chris Kane-Pardy
//
// Tests the wb_input_controller peripheral located at:
// Base Address: 0x80001500
//
// This program:
// 1. Reads current button states
// 2. Detects newly pressed buttons
// 3. Clears edge flags after handling
// 4. Displays debug output through LEDs
// ============================================================

// Existing GPIO LED output register
#define GPIO_BASE          0x80001400UL
#define GPIO_OUTPUT        (*(volatile uint32_t *)(GPIO_BASE + 0x04))
#define GPIO_OUTPUT_EN     (*(volatile uint32_t *)(GPIO_BASE + 0x08))

// Input controller registers
#define INPUT_BASE         0x80001500UL

#define INPUT_STATUS       (*(volatile uint32_t *)(INPUT_BASE + 0x00))
#define INPUT_EDGE         (*(volatile uint32_t *)(INPUT_BASE + 0x04))
#define INPUT_CTRL         (*(volatile uint32_t *)(INPUT_BASE + 0x08))
#define INPUT_MODE         (*(volatile uint32_t *)(INPUT_BASE + 0x0C))

// ============================================================
// Input API
// ============================================================

uint32_t input_get_status(void)
{
    return INPUT_STATUS & 0x1F;
}

uint32_t input_get_edges(void)
{
    return INPUT_EDGE & 0x1F;
}

void input_clear_edges(uint32_t mask)
{
    INPUT_EDGE = mask & 0x1F;
}

// ============================================================
// Main Test Program
// ============================================================

int main(void)
{
    // Enable LEDs as outputs
    GPIO_OUTPUT_EN = 0xFFFF;

    // Select onboard button mode
    // 0 = buttons
    // 1 = USB keyboard (future)
    INPUT_MODE = 0x0;

    // Clear stale edge events
    INPUT_CTRL = 0x1;

    while (1)
    {
        // Read current button states
        uint32_t buttons = input_get_status();

        // Read newly pressed buttons
        uint32_t edges = input_get_edges();

        // ----------------------------------------------------
        // Display current held buttons on LEDs [4:0]
        // ----------------------------------------------------
        GPIO_OUTPUT = buttons;

        // ----------------------------------------------------
        // Example edge handling:
        // If a button was newly pressed,
        // briefly light upper LEDs
        // ----------------------------------------------------
        if (edges)
        {
            GPIO_OUTPUT |= (edges << 8);

            // Clear handled edge bits
            input_clear_edges(edges);
        }
    }

    return 0;
}