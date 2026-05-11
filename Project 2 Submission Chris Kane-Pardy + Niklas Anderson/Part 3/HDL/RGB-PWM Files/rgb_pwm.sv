//=============================================================
// Chris Kane-Pardy
// April 20th, 2026
// rgb_pwm.sv
// Description:
//   Three-channel RGB PWM controller.
//   This module instantiates three pwm_channel modules to
//   generate independent PWM outputs for the red, green,
//   and blue components of a single RGB LED.
//
//   - One shared enable input controls all three channels
//   - Each color has its own duty-cycle input
//   - The red channel's counter is exposed as a debug output
//
//   This module will be used inside the Wishbone RGB
//   peripheral for Project Part 3.
//=============================================================

module rgb_pwm #(
    parameter integer COUNTER_WIDTH = 11
) (
    input  logic clk,
    input  logic rst,
    input  logic enable,

    input  logic [COUNTER_WIDTH-1:0] red_duty,
    input  logic [COUNTER_WIDTH-1:0] green_duty,
    input  logic [COUNTER_WIDTH-1:0] blue_duty,

    output logic rgb_r,
    output logic rgb_g,
    output logic rgb_b,

    output logic [COUNTER_WIDTH-1:0] count_dbg
);

    logic [COUNTER_WIDTH-1:0] red_count_dbg;

    pwm_channel #(.COUNTER_WIDTH(COUNTER_WIDTH)) u_pwm_red (
        .clk      (clk),
        .rst      (rst),
        .enable   (enable),
        .duty     (red_duty),
        .pwm_out  (rgb_r),
        .count_dbg(red_count_dbg)
    );

    pwm_channel #(.COUNTER_WIDTH(COUNTER_WIDTH)) u_pwm_green (
        .clk      (clk),
        .rst      (rst),
        .enable   (enable),
        .duty     (green_duty),
        .pwm_out  (rgb_g),
        .count_dbg()
    );

    pwm_channel #(.COUNTER_WIDTH(COUNTER_WIDTH)) u_pwm_blue (
        .clk      (clk),
        .rst      (rst),
        .enable   (enable),
        .duty     (blue_duty),
        .pwm_out  (rgb_b),
        .count_dbg()
    );

    assign count_dbg = red_count_dbg;

endmodule