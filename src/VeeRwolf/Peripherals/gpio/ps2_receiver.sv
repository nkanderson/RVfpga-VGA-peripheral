`default_nettype none

//////////////////////////////////////////////////////////////////////////////////
// Engineer: Chris Kane-Pardy
// Create Date: 05/21/2026
// Module Name: ps2_receiver
// Project Name: Note Feller
//
// Description:
//   Simple PS/2 keyboard receiver for the Nexys A7 USB HID interface.
//   Receives PS/2 scan-code bytes and produces a one-cycle valid pulse.
//
//   Expected frame format:
//     1 start bit
//     8 data bits (LSB first)
//     1 parity bit
//     1 stop bit
//
// Outputs:
//   scan_code  - received PS/2 byte
//   scan_valid - one-cycle pulse when a byte is received
//
//////////////////////////////////////////////////////////////////////////////////

module ps2_receiver (
    input  wire       clk,
    input  wire       rst,

    input  wire       ps2_clk,
    input  wire       ps2_data,

    output reg [7:0]  scan_code,
    output reg        scan_valid
);

    reg [2:0] ps2_clk_sync;
    reg [3:0] bit_count;
    reg [10:0] shift_reg;

    wire ps2_falling_edge;

    // Synchronize PS/2 clock into FPGA clock domain
    always @(posedge clk) begin
        ps2_clk_sync <= {ps2_clk_sync[1:0], ps2_clk};
    end

    assign ps2_falling_edge =
        (ps2_clk_sync[2:1] == 2'b10);

    always @(posedge clk) begin
        if (rst) begin
            bit_count  <= 4'd0;
            shift_reg  <= 11'd0;

            scan_code  <= 8'd0;
            scan_valid <= 1'b0;
        end else begin
            scan_valid <= 1'b0;

            if (ps2_falling_edge) begin

                // Shift incoming PS/2 bit
                shift_reg <= {ps2_data, shift_reg[10:1]};

                // Count received bits
                if (bit_count == 4'd10) begin

                    // Full PS/2 frame received
                    bit_count <= 4'd0;

                    // Extract data bits [8:1]
                    scan_code <= shift_reg[8:1];

                    // Pulse valid flag
                    scan_valid <= 1'b1;

                end else begin
                    bit_count <= bit_count + 1'b1;
                end
            end
        end
    end

endmodule

`default_nettype wire