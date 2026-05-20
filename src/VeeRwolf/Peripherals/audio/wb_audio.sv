//=============================================================
//   Niklas Anderson
//   May 19th 2026
//   Wishbone audio peripheral for ECE 540 final project.
//   Provides a simple PWM audio output and an SD pin for muting.
//   Exposes memory-mapped registers for note index and volume.
//=============================================================

module wb_audio (
    input  logic        wb_clk_i,
    input  logic        wb_rst_i,

    input  logic [11:0] wb_adr_i,
    input  logic [31:0] wb_dat_i,
    input  logic [3:0]  wb_sel_i,
    input  logic        wb_we_i,
    input  logic        wb_cyc_i,
    input  logic        wb_stb_i,

    output logic [31:0] wb_dat_o,
    output logic        wb_ack_o,

    output logic        aud_pwm,
    output logic        aud_sd
);

    //---------------------------------------------------------
    // Note values - hardcoded for now.
    // Uses the formula note = (note_freq * 2^24) / f_clk, where f_clk is 25 MHz.
    // For example, A4 = 440 Hz -> (440 * 2^24) / 25,000,000 = 0x000127.
    //---------------------------------------------------------
    localparam logic [23:0] NOTE_LUT [0:7] = '{
        24'h0000B0,  // 0: C4
        24'h0000C5,  // 1: D4
        24'h0000DD,  // 2: E4
        24'h0000EA,  // 3: F4
        24'h000107,  // 4: G4
        24'h000127,  // 5: A4
        24'h00014C,  // 6: B4
        24'h00015F   // 7: C5
    };

    //---------------------------------------------------------
    // Register select
    //---------------------------------------------------------
    localparam logic [9:0] REG_CTRL = 10'h0;
    localparam logic [9:0] REG_NOTE = 10'h1;

    logic [9:0]  reg_sel;
    logic        wb_access;

    assign reg_sel   = wb_adr_i[11:2];
    assign wb_access = wb_cyc_i && wb_stb_i;

    //---------------------------------------------------------
    // CPU-visible registers
    //---------------------------------------------------------
    logic        ctrl_enable;
    logic [3:0]  ctrl_volume;
    logic [3:0]  note_idx;
    logic        note_on;

    //---------------------------------------------------------
    // Audio generation logic
    //---------------------------------------------------------
    logic [23:0] phase_accumulator;
    logic [23:0] phase_increment;
    logic        square;
    logic [6:0]  amplitude;
    logic [7:0]  sample;
    logic [8:0]  dsm_acc;

    assign amplitude = {ctrl_volume[3:0], 3'b000};
    assign phase_increment = NOTE_LUT[note_idx[2:0]];
    assign square = phase_accumulator[23];
    assign sample = note_on ?
                    (square ? (8'd128 + amplitude) : (8'd128 - amplitude))
                    : 8'd128;

    assign aud_sd = ctrl_enable;
    assign aud_pwm = dsm_acc[8];

    always_ff @(posedge wb_clk_i) begin
        if (wb_rst_i) begin
            phase_accumulator <= 24'd0;
            dsm_acc <= 9'd0;
        end else if (ctrl_enable) begin
            phase_accumulator <= phase_accumulator + phase_increment;
            dsm_acc <= dsm_acc[7:0] + sample;
        end
    end

    //---------------------------------------------------------
    // Wishbone acknowledge and writes
    //---------------------------------------------------------
    always_ff @(posedge wb_clk_i) begin
        if (wb_rst_i) begin
            wb_ack_o       <= 1'b0;
            ctrl_enable    <= 1'b0;
            ctrl_volume    <= 4'd0;
            note_idx       <= 4'd0;
            note_on        <= 1'b0;
        end else begin
            wb_ack_o <= wb_access && !wb_ack_o;

            if (wb_access && wb_we_i && !wb_ack_o) begin
                unique case (reg_sel)
                    REG_CTRL: begin
                        ctrl_enable <= wb_dat_i[0];
                        ctrl_volume <= wb_dat_i[7:4];
                    end

                    REG_NOTE: begin
                        note_idx <= wb_dat_i[3:0];
                        note_on  <= wb_dat_i[8];
                    end
                    default: ;
                endcase
            end
        end
    end

    //---------------------------------------------------------
    // Wishbone reads
    //---------------------------------------------------------
    always_comb begin
        unique case (reg_sel)
            REG_CTRL: wb_dat_o = {24'd0, ctrl_volume, 3'd0, ctrl_enable};
            REG_NOTE: wb_dat_o = {23'd0, note_on, 4'd0, note_idx};
            default:  wb_dat_o = 32'h00000000;
        endcase
    end

endmodule
