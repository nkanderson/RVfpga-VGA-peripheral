//=============================================================
// VGA Peripheral Base Address and Registers
//=============================================================
#define D_VGA_BASE_ADDRESS 0x80001500

// VGA Register offsets
#define VGA_COORD_REG 0x04 // [19:10] = row, [9:0] = col
#define VGA_DATA_REG 0x0C  // [7:0] = color (R[2], G[1], B[0])

// Color definitions
#define VGA_RED 0x04
#define VGA_CYAN 0x03
#define VGA_WHITE 0x07

// Screen dimensions
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

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
static void vga_set_color(unsigned char color) {
  vga_write_reg(VGA_DATA_REG, color & 0x07);
}

/**
 * Draw a 32x32 box at the specified coordinates
 */
static void vga_draw_box(int row, int col, unsigned char color) {
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
    unsigned char color = test_cases[i][2];

    vga_draw_box(row, col, color);
    delay(1000000);
  }
}

//=============================================================
// Main Program
//=============================================================

int main(void) {

  while (1) {
    vga_box_demo();
    delay(2000000);
  }

  return 0;
}
