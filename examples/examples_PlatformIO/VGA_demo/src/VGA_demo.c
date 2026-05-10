//=============================================================
// VGA Peripheral Base Address and Registers
//=============================================================
#define D_VGA_BASE_ADDRESS 0x80001500

// VGA Register offsets
#define VGA_MODE_REG 0x00  // [0] = mode (1 = text, 0 = graphics)
#define VGA_COORD_REG 0x04 // [19:10] = row, [9:0] = col
// For the VGA_DATA_REG, the color bits are defined as follows:
// Foreground: [15:12]=R, [11:8]=G, [7:4]=B
// Background: [31:28]=R, [27:24]=G, [23:20]=B
#define VGA_DATA_REG 0x0C
#define VGA_CHAR_REG 0x10 // [7:0] = character code

// Color definitions (4-bit foreground colors)
#define VGA_RED 0xF000   // R=1111, G=0000, B=0000
#define VGA_CYAN 0x0FF0  // R=0000, G=1111, B=1111
#define VGA_WHITE 0xFFF0 // R=1111, G=1111, B=1111

// Screen dimensions
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define TEXT_DEMO 1

//=============================================================
// Helper Functions
//=============================================================

/**
 * Write to a VGA register
 */
static inline void vga_write_reg(unsigned int offset, unsigned int value) {
  volatile unsigned int *vga_reg =
      (volatile unsigned int *)(D_VGA_BASE_ADDRESS + offset);
  *vga_reg = value;
}

/**
 * Set the color for subsequent drawing
 */
static void vga_set_color(unsigned int color) {
  vga_write_reg(VGA_DATA_REG, color);
}

/**
 * Draw a 32x32 box at the specified coordinates
 */
static void vga_draw_box(int row, int col, unsigned int color) {
  // Set color
  vga_set_color(color);

  // Set coordinates - hardware draws 32x32 box
  unsigned int coord = ((row & 0x3FF) << 10) | (col & 0x3FF);
  vga_write_reg(VGA_COORD_REG, coord);
}

/**
 * Delay function
 */
static void delay(unsigned int iterations) {
  volatile unsigned int i;
  for (i = 0; i < iterations; i++) {
    asm volatile("nop");
  }
}

/**
 * Simple demo - draw one box at a time
 */
static void vga_box_demo(void) {
  int test_cases[][3] = {
      // {row, col, color}
      {0, 0, VGA_WHITE},     // Top-left corner
      {0, 608, VGA_WHITE},   // Top-right area (640 - 32)
      {448, 0, VGA_WHITE},   // Bottom-left area (480 - 32)
      {448, 608, VGA_WHITE}, // Bottom-right area
      {240, 320, VGA_CYAN},  // Center
      {100, 100, VGA_RED},   // Arbitrary position
  };

  int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

  for (int i = 0; i < num_tests; i++) {
    int row = test_cases[i][0];
    int col = test_cases[i][1];
    unsigned int color = test_cases[i][2];

    vga_draw_box(row, col, color);
    delay(800000);
  }
}

/**
 * Text demo - draw one character at a time, 0-9
 */
static void vga_text_demo(void) {
  // Set text mode
  vga_write_reg(VGA_MODE_REG, 0x01);

  for (int i = 0; i < 10; i++) {
    // Set coordinates (row 5, col i*16)
    unsigned int coord = ((5 & 0x3FF) << 10) | ((i * 16) & 0x3FF);
    vga_write_reg(VGA_COORD_REG, coord);

    // For even numbers, set the background color to red.
    // Background color is in bits [31:28]=R, [27:24]=G, [23:20]=B
    // Foreground is white [15:12]=R, [11:8]=G, [7:4]=B
    if (i % 2 == 0) {
      unsigned int color = 0xF000FFF0; // Foreground white, background red
      vga_write_reg(VGA_DATA_REG, color);
    } else {
      unsigned int color = 0x0000FFF0; // Foreground white, background black
      vga_write_reg(VGA_DATA_REG, color);
    }

    // Write character code to character register
    vga_write_reg(VGA_CHAR_REG, '0' + i);

    delay(1000000);
  }
}

//=============================================================
// Main Program
//=============================================================

int main(void) {
  vga_write_reg(VGA_MODE_REG, 0x00); // Start in graphics mode

  if (TEXT_DEMO) {
    while (1) {
      vga_text_demo();
    }
  } else {
    while (1) {
      vga_box_demo();
      delay(2000000);
    }
  }

  return 0;
}
