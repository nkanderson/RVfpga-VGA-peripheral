//=============================================================
//   Niklas Anderson
//   May 19th 2026
//   Wishbone audio peripheral for ECE 540 final project.
//   Provides a simple PWM audio output and an SD pin for muting.
//   Exposes memory-mapped registers for note index and volume.
// 
//   LLM Acknowledgment: This code was generated with the assistance of a
//   language model, which provided the initial structure and logic. The
//   final implementation was reviewed and edited by the author to ensure
//   correctness and functionality on the target hardware platform.
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
    localparam NUM_NOTES = 8;
    localparam logic [23:0] NOTE_LUT [0:NUM_NOTES-1] = '{
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
    localparam logic [9:0] REG_CTRL   = 10'h0;
    localparam logic [9:0] REG_VOICES = 10'h1;
    localparam logic [9:0] REG_VOL    = 10'h2;

    logic [9:0]  reg_sel;
    logic        wb_access;

    assign reg_sel   = wb_adr_i[11:2];
    assign wb_access = wb_cyc_i && wb_stb_i;

    //---------------------------------------------------------
    // CPU-visible registers
    //---------------------------------------------------------
    logic        ctrl_enable;
    logic [3:0]  ctrl_volume;
    logic [7:0]  voices_on; 

    //---------------------------------------------------------
    // Audio generation logic
    //---------------------------------------------------------
    logic [23:0] phase_accumulator [0:NUM_NOTES-1];
    logic        square [0:NUM_NOTES-1];
    logic [3:0]  voices_vol [0:NUM_NOTES-1]; // per-voice 4-bit volume
    logic signed [7:0] amplitude [0:NUM_NOTES-1];
    logic signed [7:0] sample [0:NUM_NOTES-1];
    logic signed [10:0] sample_sum;
    logic [7:0]  dsm_in;
    logic [8:0]  dsm_acc;

    assign aud_sd = ctrl_enable;
    assign aud_pwm = dsm_acc[8];

    genvar gv;
    generate
        for (gv = 0; gv < NUM_NOTES; gv++) begin : g_amp
            assign amplitude[gv] = $signed({1'b0, voices_vol[gv], 3'b000});
        end
    endgenerate

    // Per-voice combinational sample contribution
    always_comb begin
        sample_sum = 11'sd0;
        for (int v = 0; v < NUM_NOTES; v++) begin
            square[v] = phase_accumulator[v][23];
            sample[v] = voices_on[v] ? (square[v] ? amplitude[v] : -amplitude[v]) : 8'sd0;
            sample_sum = sample_sum + {{3{sample[v][7]}}, sample[v]};  // sign-extend & accumulate
        end
        dsm_in = $unsigned( (sample_sum >>> 3) + 11'sd128 );  // shift, then bias
    end

    // 8 phase accumulators + DSM
    always_ff @(posedge wb_clk_i) begin
        if (wb_rst_i) begin
            for (int v = 0; v < NUM_NOTES; v++) begin
                phase_accumulator[v] <= 24'd0;
            end
            dsm_acc <= 9'd0;
        end else if (ctrl_enable) begin
            for (int v = 0; v < NUM_NOTES; v++) begin
                phase_accumulator[v] <= phase_accumulator[v] + NOTE_LUT[v];
            end
            dsm_acc <= dsm_acc[7:0] + dsm_in;
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
            voices_on      <= 8'b0;
            for (int v = 0; v < NUM_NOTES; v++) begin
                // default: max volume on every voice
                voices_vol[v] <= 4'hF;
            end
        end else begin
            wb_ack_o <= wb_access && !wb_ack_o;

            if (wb_access && wb_we_i && !wb_ack_o) begin
                unique case (reg_sel)
                    REG_CTRL: begin
                        ctrl_enable <= wb_dat_i[0];
                        ctrl_volume <= wb_dat_i[7:4];
                    end

                    REG_VOICES: begin
                        voices_on <= wb_dat_i[7:0];
                    end
                    REG_VOL: begin
                        for (int v = 0; v < NUM_NOTES; v++) begin
                            voices_vol[v] <= wb_dat_i[v*4 +: 4];
                        end
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
            REG_CTRL:   wb_dat_o = {24'd0, ctrl_volume, 3'd0, ctrl_enable};
            REG_VOICES: wb_dat_o = {24'd0, voices_on};
            REG_VOL: wb_dat_o = {voices_vol[7], voices_vol[6], voices_vol[5], voices_vol[4],
                     voices_vol[3], voices_vol[2], voices_vol[1], voices_vol[0]};
            default:    wb_dat_o = 32'h00000000;
        endcase
    end

endmodule
