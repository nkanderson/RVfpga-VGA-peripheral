//=============================================================
// Chris Kane-Pardy
// April 21st, 2026
// main.c
// Description:
//   Final RGB LED control program for Project 1 Part 3.
//   - Mirrors switches to LEDs
//   - sw[14] enables/disables PWM
//   - sw[15] selects which RGB LED is modified by buttons
//   - BTNR increments red
//   - BTNU increments green
//   - BTNL increments blue
//   - BTND resets selected LED RGB duty cycles to 1
//   - 7-segment display shows selected LED RGB duty cycles
//     in decimal (0..99) as:
//         Dig[7:6] = Red
//         Dig[5]   = blank
//         Dig[4:3] = Green
//         Dig[2]   = blank
//         Dig[1:0] = Blue
//=============================================================

#include <stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))

// GPIO1 registers
#define GPIO1_IN   REG32(0x80001400u)
#define GPIO1_OUT  REG32(0x80001404u)
#define GPIO1_OE   REG32(0x80001408u)

// GPIO2 registers
#define GPIO2_IN   REG32(0x80001800u)

// RGB peripheral registers
#define RGB0_CTRL  REG32(0x80001C00u)
#define RGB1_CTRL  REG32(0x80001C04u)

// 7-segment controller registers
#define SEVSEG_EN  REG32(0x80001038u)
#define SEVSEG_DIG REG32(0x8000103Cu)

// Button bit masks
#define BTN_L   (1u << 0)
#define BTN_R   (1u << 1)
#define BTN_U   (1u << 2)
#define BTN_D   (1u << 3)

#define STEP 64u

static uint32_t make_rgb_ctrl(uint32_t en, uint32_t r, uint32_t g, uint32_t b)
{
    return ((en & 0x1u) << 31) |
           ((r  & 0x3FFu) << 20) |
           ((g  & 0x3FFu) << 10) |
           ((b  & 0x3FFu) <<  0);
}

// Convert 10-bit-ish duty value to decimal percent 0..99
static uint32_t duty_to_percent(uint32_t duty)
{
    uint32_t pct = (duty * 99u) / 1023u;
    if (pct > 99u) pct = 99u;
    return pct;
}

// Pack one decimal 0..99 into two 7-seg digit nibbles
// tens goes in the higher nibble, ones in the lower nibble
static uint32_t pack_two_digits(uint32_t value)
{
    uint32_t tens = value / 10u;
    uint32_t ones = value % 10u;
    return (tens << 4) | ones;
}

int main(void)
{
    uint32_t gpio_in;
    uint32_t switches;
    uint32_t btn, btn_prev = 0;
    uint32_t rising;
    uint32_t enable;
    uint32_t selected_rgb1;

    // Separate stored colors for the two RGB LEDs
    // RGB0 = LD17, RGB1 = LD16
    uint32_t r0 = 100u, g0 =  50u, b0 =   1u;
    uint32_t r1 = 100u, g1 =  50u, b1 =   1u;

    uint32_t r_disp, g_disp, b_disp;
    uint32_t r_pct, g_pct, b_pct;

    // GPIO1:
    // lower 16 bits = outputs (LEDs)
    // upper 16 bits = inputs  (switches)
    GPIO1_OE = 0x0000FFFFu;

    // Enable digits 7,6,4,3,1,0 and blank digits 5 and 2
    // Enables_Reg is active low: 0 = ON, 1 = OFF
    SEVSEG_EN = 0x24u;

    while (1)
    {
        gpio_in = GPIO1_IN;
        switches = (gpio_in >> 16) & 0xFFFFu;
        btn = GPIO2_IN & 0x1Fu;

        // Mirror switches to LEDs
        GPIO1_OUT = switches;

        // Edge detect buttons
        rising = btn & ~btn_prev;
        btn_prev = btn;

        // sw[14] controls PWM enable
        enable = (gpio_in >> 30) & 0x1u;

        // sw[15] selects which RGB LED to modify / display
        selected_rgb1 = (gpio_in >> 31) & 0x1u;

        if (selected_rgb1 == 0u)
        {
            // Modify RGB0 / LD17
            if (rising & BTN_R) {
                r0 += STEP;
                if (r0 > 1023u) r0 = 50u;
            }

            if (rising & BTN_U) {
                g0 += STEP;
                if (g0 > 1023u) g0 = 50u;
            }

            if (rising & BTN_L) {
                b0 += STEP;
                if (b0 > 1023u) b0 = 50u;
            }

            if (rising & BTN_D) {
                r0 = 1u;
                g0 = 1u;
                b0 = 1u;
            }

            r_disp = r0;
            g_disp = g0;
            b_disp = b0;
        }
        else
        {
            // Modify RGB1 / LD16
            if (rising & BTN_R) {
                r1 += STEP;
                if (r1 > 1023u) r1 = 50u;
            }

            if (rising & BTN_U) {
                g1 += STEP;
                if (g1 > 1023u) g1 = 50u;
            }

            if (rising & BTN_L) {
                b1 += STEP;
                if (b1 > 1023u) b1 = 50u;
            }

            if (rising & BTN_D) {
                r1 = 1u;
                g1 = 1u;
                b1 = 1u;
            }

            r_disp = r1;
            g_disp = g1;
            b_disp = b1;
        }

        // Always write both RGB LEDs from their own stored state
        RGB0_CTRL = make_rgb_ctrl(enable, r0, g0, b0);
        RGB1_CTRL = make_rgb_ctrl(enable, r1, g1, b1);

        // Convert selected LED duty cycles to decimal 0..99
        r_pct = duty_to_percent(r_disp);
        g_pct = duty_to_percent(g_disp);
        b_pct = duty_to_percent(b_disp);

        // Digits_Reg layout:
        // [31:28] Dig7 = R tens
        // [27:24] Dig6 = R ones
        // [23:20] Dig5 = blank (disabled by Enables_Reg)
        // [19:16] Dig4 = G tens
        // [15:12] Dig3 = G ones
        // [11: 8] Dig2 = blank (disabled by Enables_Reg)
        // [ 7: 4] Dig1 = B tens
        // [ 3: 0] Dig0 = B ones
        SEVSEG_DIG =
            (pack_two_digits(r_pct) << 24) |
            (0x0u                   << 20) |
            (pack_two_digits(g_pct) << 12) |
            (0x0u                   <<  8) |
            (pack_two_digits(b_pct) <<  0);
    }

    return 0;
}