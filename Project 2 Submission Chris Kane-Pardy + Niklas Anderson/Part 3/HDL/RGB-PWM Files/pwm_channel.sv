//=============================================================
// Chris Kane-Pardy
// April 20th, 2026
// pwm_channel.sv
// Description: 
//   Single-channel PWM generator used for RGB LED control.
//   This module generates a pulse-width modulated signal
//   based on a free-running counter and a programmable
//   duty cycle input.
//
//   - If enable = 0 → output is always OFF
//   - If enable = 1 → output is HIGH when count < duty
//   - Counter continuously increments and wraps naturally
//
//   This module will be used as the building block for
//   the RGB PWM peripheral in Project Part 3.
//=============================================================

module pwm_channel #(
    parameter integer COUNTER_WIDTH = 11
) (
    input  logic clk,                         // system clock
    input  logic rst,                         // active-high reset
    input  logic enable,                      // enable PWM output
    input  logic [COUNTER_WIDTH-1:0] duty,    // duty cycle threshold
    output logic pwm_out,                     // PWM output signal
    output logic [COUNTER_WIDTH-1:0] count_dbg // debug counter output
);

    // Internal counter
    logic [COUNTER_WIDTH-1:0] count_q;

    //---------------------------------------------------------
    // Counter logic
    //---------------------------------------------------------
    always_ff @(posedge clk) begin
        if (rst) begin
            count_q <= '0;
        end else begin
            count_q <= count_q + 1'b1;
        end
    end

    //---------------------------------------------------------
    // PWM comparison logic
    //---------------------------------------------------------
    always_comb begin
        count_dbg = count_q;

        if (!enable)
            pwm_out = 1'b0;
        else if (count_q < duty)
            pwm_out = 1'b1;
        else
            pwm_out = 1'b0;
    end

endmodule