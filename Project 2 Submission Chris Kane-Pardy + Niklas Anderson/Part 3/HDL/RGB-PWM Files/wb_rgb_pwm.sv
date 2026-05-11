//=============================================================
// Chris Kane-Pardy
// April 21st, 2026
// wb_rgb_pwm.sv
// Description:
//   Wishbone RGB LED peripheral for Project Part 3.
//
//   This module implements a simple Wishbone slave with
//   memory-mapped control registers for two RGB LEDs.
//   Each RGB LED is controlled by one 32-bit register:
//
//      bit 31    : enable
//      bit 29:20 : red duty cycle
//      bit 19:10 : green duty cycle
//      bit  9:0  : blue duty cycle
//
//   The register values are used to drive two rgb_pwm
//   instances. Two additional read-only registers provide
//   debug access to the PWM counters.
//
//   Register offsets:
//      0x00 : RGB0 control
//      0x04 : RGB1 control
//      0x08 : RGB0 counter debug
//      0x0C : RGB1 counter debug
//=============================================================

module wb_rgb_pwm (
    input  logic        wb_clk_i,
    input  logic        wb_rst_i,

    input  logic [5:0]  wb_adr_i,
    input  logic [31:0] wb_dat_i,
    input  logic [3:0]  wb_sel_i,
    input  logic        wb_we_i,
    input  logic        wb_cyc_i,
    input  logic        wb_stb_i,

    output logic [31:0] wb_dat_o,
    output logic        wb_ack_o,

    output logic        rgb0_r,
    output logic        rgb0_g,
    output logic        rgb0_b,

    output logic        rgb1_r,
    output logic        rgb1_g,
    output logic        rgb1_b
);

    //---------------------------------------------------------
    // Local register select values
    //---------------------------------------------------------
    localparam logic [3:0] REG_RGB0_CTRL  = 4'h0;
    localparam logic [3:0] REG_RGB1_CTRL  = 4'h1;
    localparam logic [3:0] REG_RGB0_COUNT = 4'h2;
    localparam logic [3:0] REG_RGB1_COUNT = 4'h3;

    //---------------------------------------------------------
    // Internal control registers
    //---------------------------------------------------------
    logic [31:0] rgb0_ctrl_reg;
    logic [31:0] rgb1_ctrl_reg;

    logic [10:0] rgb0_count_dbg;
    logic [10:0] rgb1_count_dbg;

    logic [3:0] reg_sel;
    logic       wb_access;

    assign reg_sel   = wb_adr_i[5:2];
    assign wb_access = wb_cyc_i && wb_stb_i;

    //---------------------------------------------------------
    // Wishbone acknowledge + register writes
    //---------------------------------------------------------
    always_ff @(posedge wb_clk_i) begin
        if (wb_rst_i) begin
            wb_ack_o      <= 1'b0;
            rgb0_ctrl_reg <= 32'h00000000;
            rgb1_ctrl_reg <= 32'h00000000;
        end else begin
            // one-cycle acknowledge
            wb_ack_o <= wb_access && !wb_ack_o;

            if (wb_access && wb_we_i && !wb_ack_o) begin
                unique case (reg_sel)
                    REG_RGB0_CTRL: rgb0_ctrl_reg <= wb_dat_i;
                    REG_RGB1_CTRL: rgb1_ctrl_reg <= wb_dat_i;
                    default: ;
                endcase
            end
        end
    end

    //---------------------------------------------------------
    // Wishbone read mux
    //---------------------------------------------------------
    always_comb begin
        unique case (reg_sel)
            REG_RGB0_CTRL:  wb_dat_o = rgb0_ctrl_reg;
            REG_RGB1_CTRL:  wb_dat_o = rgb1_ctrl_reg;
            REG_RGB0_COUNT: wb_dat_o = {21'd0, rgb0_count_dbg};
            REG_RGB1_COUNT: wb_dat_o = {21'd0, rgb1_count_dbg};
            default:        wb_dat_o = 32'h00000000;
        endcase
    end

    //---------------------------------------------------------
    // RGB PWM instance 0
    //---------------------------------------------------------
    rgb_pwm #(.COUNTER_WIDTH(11)) u_rgb0 (
        .clk      (wb_clk_i),
        .rst      (wb_rst_i),
        .enable   (rgb0_ctrl_reg[31]),
        .red_duty ({rgb0_ctrl_reg[29:20], 1'b0}),
        .green_duty({rgb0_ctrl_reg[19:10], 1'b0}),
        .blue_duty({rgb0_ctrl_reg[9:0], 1'b0}),
        .rgb_r    (rgb0_r),
        .rgb_g    (rgb0_g),
        .rgb_b    (rgb0_b),
        .count_dbg(rgb0_count_dbg)
    );

    //---------------------------------------------------------
    // RGB PWM instance 1
    //---------------------------------------------------------
    rgb_pwm #(.COUNTER_WIDTH(11)) u_rgb1 (
        .clk      (wb_clk_i),
        .rst      (wb_rst_i),
        .enable   (rgb1_ctrl_reg[31]),
        .red_duty ({rgb1_ctrl_reg[29:20], 1'b0}),
        .green_duty({rgb1_ctrl_reg[19:10], 1'b0}),
        .blue_duty({rgb1_ctrl_reg[9:0], 1'b0}),
        .rgb_r    (rgb1_r),
        .rgb_g    (rgb1_g),
        .rgb_b    (rgb1_b),
        .count_dbg(rgb1_count_dbg)
    );

endmodule