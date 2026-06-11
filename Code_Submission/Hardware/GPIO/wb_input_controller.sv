`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: Portland State University
// Engineer: Chris Kane-Pardy
//
// Create Date: 05/20/2026 07:51:19 PM
// Design Name: Note Feller Input Subsystem
// Module Name: wb_input_controller
// Project Name: Note Feller
// Target Devices: Digilent Nexys A7 / Xilinx Artix-7
// Tool Versions: Vivado 2024.x
// Description:
//   Wishbone memory-mapped input controller for the Note Feller SoC.
//   This module exposes player input state to software through registers.
//   Baseline functionality reads the Nexys A7 onboard pushbuttons.
//   The interface also reserves support for future USB keyboard input.
//
// Dependencies:
//   Connected to existing VeeRwolf/RVfpgaEL2 Wishbone interconnect.
//   Uses the gpio2 address slot at base address 0x80001500.
//
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
//   Register map:
//     Base + 0x00 : INPUT_STATUS
//     Base + 0x04 : INPUT_EDGE
//     Base + 0x08 : INPUT_CTRL
//     Base + 0x0C : INPUT_MODE
//
//////////////////////////////////////////////////////////////////////////////////

`default_nettype none

module wb_input_controller (
    input  wire        wb_clk_i,
    input  wire        wb_rst_i,

    input  wire [31:0] wb_adr_i,
    input  wire [31:0] wb_dat_i,
    input  wire  [3:0] wb_sel_i,
    input  wire        wb_we_i,
    input  wire        wb_cyc_i,
    input  wire        wb_stb_i,

    output reg  [31:0] wb_dat_o,
    output reg         wb_ack_o,
    output wire        wb_err_o,
    output wire        wb_rty_o,

    input  wire [4:0]  i_btn,

    input  wire [7:0]  i_usb_keycode,
    input  wire        i_usb_valid
);

    // Register offsets from base address 0x80001500:
    // 0x00 = INPUT_STATUS: current active input bits
    // 0x04 = INPUT_EDGE:   newly pressed input bits, write 1s to clear
    // 0x08 = INPUT_CTRL:   bit[0] clears edge register
    // 0x0C = INPUT_MODE:   0 = onboard buttons, 1 = USB keyboard

    localparam ADDR_STATUS = 4'h0;
    localparam ADDR_EDGE   = 4'h1;
    localparam ADDR_CTRL   = 4'h2;
    localparam ADDR_MODE   = 4'h3;

    // async directives tell Vivado to treat these FFs as a 2DFF synchronizer
    (* ASYNC_REG = "TRUE" *) reg [4:0] btn_sync_0;
    (* ASYNC_REG = "TRUE" *) reg [4:0] btn_sync_1;
    reg [4:0] input_prev;

    reg [4:0] input_status;
    reg [4:0] input_edge;
    reg [1:0] input_mode;

    wire bus_access = wb_cyc_i & wb_stb_i;
    wire bus_write  = bus_access & wb_we_i;
    wire bus_read   = bus_access & ~wb_we_i;

    wire [3:0] reg_addr = wb_adr_i[5:2];

    assign wb_err_o = 1'b0;
    assign wb_rty_o = 1'b0;

    // -------------------------------------------------------------------------
    // USB / PS2 keyboard input decoding
    //
    // PS/2 Set-2 scan codes:
    //
    // A     = 0x1C
    // S     = 0x1B
    // D     = 0x23
    // F     = 0x2B
    // Enter = 0x5A
    //
    // Break codes:
    // F0 <scan_code>
    //
    // The USB HID host on the Nexys A7 forwards keyboard data as PS/2 scan codes.
    // -------------------------------------------------------------------------
    
    reg        break_code;
    reg [4:0]  usb_input_state;
    
    always @(posedge wb_clk_i) begin
        if (wb_rst_i) begin
            break_code     <= 1'b0;
            usb_input_state <= 5'd0;
    
        end else if (i_usb_valid) begin
    
            // Break code prefix
            if (i_usb_keycode == 8'hF0) begin
                break_code <= 1'b1;
    
            end else begin
    
                // -------------------------------------------------------------
                // Key press events
                // -------------------------------------------------------------
                if (!break_code) begin
                    case (i_usb_keycode)
                    
                        8'h1C: usb_input_state[0] <= 1'b1; // A
                        8'h1B: usb_input_state[1] <= 1'b1; // S
                        8'h23: usb_input_state[2] <= 1'b1; // D
                        8'h2B: usb_input_state[3] <= 1'b1; // F
                        8'h5A: usb_input_state[4] <= 1'b1; // Enter
                    
                        default: begin
                        end
                    endcase
    
                end else begin
    
                    // ---------------------------------------------------------
                    // Key release events
                    // ---------------------------------------------------------
                    case (i_usb_keycode)
                    
                        8'h1C: usb_input_state[0] <= 1'b0; // A
                        8'h1B: usb_input_state[1] <= 1'b0; // S
                        8'h23: usb_input_state[2] <= 1'b0; // D
                        8'h2B: usb_input_state[3] <= 1'b0; // F
                        8'h5A: usb_input_state[4] <= 1'b0; // Enter
                    
                        default: begin
                        end
                    endcase
    
                    break_code <= 1'b0;
                end
            end
        end
    end

    wire [4:0] selected_input =
        (input_mode == 2'd1) ? usb_input_state : btn_sync_1;

    always @(posedge wb_clk_i) begin
        if (wb_rst_i) begin
            wb_ack_o      <= 1'b0;
            wb_dat_o      <= 32'd0;

            btn_sync_0    <= 5'd0;
            btn_sync_1    <= 5'd0;

            input_prev    <= 5'd0;
            input_status  <= 5'd0;
            input_edge    <= 5'd0;
            input_mode    <= 2'd0;
        end else begin
            // One-cycle Wishbone acknowledge.
            wb_ack_o <= bus_access & ~wb_ack_o;

            // Synchronize physical buttons into Wishbone clock domain.
            btn_sync_0 <= i_btn;
            btn_sync_1 <= btn_sync_0;

            // Current selected input source.
            input_status <= selected_input;

            // Rising-edge / new-press detection.
            input_edge <= input_edge | (selected_input & ~input_prev);
            input_prev <= selected_input;

            // Register writes.
            if (bus_write && ~wb_ack_o) begin
                case (reg_addr)
                    ADDR_EDGE: begin
                        // Write 1s to clear corresponding edge bits.
                        input_edge <= input_edge & ~wb_dat_i[4:0];
                    end

                    ADDR_CTRL: begin
                        // bit[0] clears all edge bits.
                        if (wb_dat_i[0])
                            input_edge <= 5'd0;
                    end

                    ADDR_MODE: begin
                        // 0 = buttons, 1 = USB
                        input_mode <= wb_dat_i[1:0];
                    end

                    default: begin
                    end
                endcase
            end

            // Register reads.
            if (bus_read && ~wb_ack_o) begin
                case (reg_addr)
                    ADDR_STATUS: wb_dat_o <= {27'd0, input_status};
                    ADDR_EDGE:   wb_dat_o <= {27'd0, input_edge};
                    ADDR_CTRL:   wb_dat_o <= 32'd0;
                    ADDR_MODE:   wb_dat_o <= {30'd0, input_mode};
                    default:     wb_dat_o <= 32'd0;
                endcase
            end
        end
    end

endmodule

`default_nettype wire
