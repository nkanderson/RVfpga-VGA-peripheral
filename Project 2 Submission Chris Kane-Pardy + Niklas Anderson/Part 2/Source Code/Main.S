# ------------------------------------------------------------
# Project 1 - Part 2
# Chris Kane-Pardy
# April 19th, 2026
# GPIO1: LEDs
# GPIO2: Pushbuttons
#
# Function:
#   - Display a 16-bit counter on the LEDs
#   - BTNL (BTN0) slows the count
#   - BTNR (BTN1) resets the count to 1
# ------------------------------------------------------------

# -----------------------------
# Memory-mapped I/O addresses
# -----------------------------
.eqv GPIO1_IN,     0x80001400
.eqv GPIO1_OUT,    0x80001404
.eqv GPIO1_OE,     0x80001408

.eqv GPIO2_IN,     0x80001800
.eqv GPIO2_OUT,    0x80001804
.eqv GPIO2_OE,     0x80001808

# -----------------------------
# Button bit masks
# -----------------------------
.eqv BTNL_MASK,    0x01
.eqv BTNR_MASK,    0x02

# -----------------------------
# Delay constants
# -----------------------------
.eqv DELAY_FAST,   0x0008FFFF
.eqv DELAY_SLOW,   0x004FFFFF

.text
.globl main

main:
    # "Cache" frequently used peripheral addresses
    li      s1, GPIO1_OUT
    li      s2, GPIO1_OE
    li      s3, GPIO2_IN

    # Configure GPIO1 lower 16 bits as outputs for LEDs
    li      t0, 0x0000FFFF
    sw      t0, 0(s2)

    # Initialize counter to 1
    li      s0, 1

main_loop:
    # Show current counter value on LEDs
    sw      s0, 0(s1)

    # Read pushbuttons from GPIO2 input register
    lw      t0, 0(s3)

    # If BTNR (BTN1) is pressed, reset counter to 1
    andi    t1, t0, BTNR_MASK
    bnez    t1, reset_counter

    # Select delay based on BTNL (BTN0)
    andi    t1, t0, BTNL_MASK
    bnez    t1, use_slow_delay

use_fast_delay:
    li      t2, DELAY_FAST
    j       delay_loop

use_slow_delay:
    li      t2, DELAY_SLOW

delay_loop:
    addi    t2, t2, -1
    bnez    t2, delay_loop

    # Increment counter
    addi    s0, s0, 1

    # Keep counter to 16 bits
    li      t3, 0xFFFF
    and     s0, s0, t3

    # Counter must never remain at 0
    bnez    s0, main_loop
    li      s0, 1
    j       main_loop

reset_counter:
    li      s0, 1
    j       main_loop