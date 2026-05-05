//=============================================================
//   Chris Kane-Pardy
//   May 4th 2026
//   Wishbone VGA peripheral for Project 2.
//   Provides 640x480 VGA timing using dtg.sv and exposes
//   simple memory-mapped registers for row/column and data.
//=============================================================

module wb_vga (
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

    output logic        vga_hsync,
    output logic        vga_vsync,
    output logic [3:0]  vga_red,
    output logic [3:0]  vga_green,
    output logic [3:0]  vga_blue
);

    //---------------------------------------------------------
    // Register select
    //---------------------------------------------------------
    localparam logic [3:0] REG_COORD = 4'h1;
    localparam logic [3:0] REG_DATA  = 4'h3;

    logic [3:0] reg_sel;
    logic       wb_access;

    assign reg_sel   = wb_adr_i[5:2];
    assign wb_access = wb_cyc_i && wb_stb_i;

    //---------------------------------------------------------
    // VGA timing signals
    //---------------------------------------------------------
    logic        video_on;
    logic [11:0] pixel_row;
    logic [11:0] pixel_col;

    dtg u_dtg (
        .clock        (wb_clk_i),
        .rst          (wb_rst_i),
        .horiz_sync   (vga_hsync),
        .vert_sync    (vga_vsync),
        .video_on     (video_on),
        .pixel_row    (pixel_row),
        .pixel_column (pixel_col)
    );

    //---------------------------------------------------------
    // CPU-visible registers
    //---------------------------------------------------------
    logic [9:0]  target_row;
    logic [9:0]  target_col;
    logic [7:0]  data_buffer;

    //---------------------------------------------------------
    // Wishbone acknowledge and writes
    //---------------------------------------------------------
    always_ff @(posedge wb_clk_i) begin
        if (wb_rst_i) begin
            wb_ack_o    <= 1'b0;
            target_row  <= 10'd100;
            target_col  <= 10'd100;
            data_buffer <= 8'h0F;
        end else begin
            wb_ack_o <= wb_access && !wb_ack_o;

            if (wb_access && wb_we_i && !wb_ack_o) begin
                unique case (reg_sel)
                    REG_COORD: begin
                        target_row <= wb_dat_i[19:10];
                        target_col <= wb_dat_i[9:0];
                    end

                    REG_DATA: begin
                        data_buffer <= wb_dat_i[7:0];
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
            REG_COORD: wb_dat_o = {12'd0, target_row, target_col};
            REG_DATA:  wb_dat_o = {24'd0, data_buffer};
            default:   wb_dat_o = 32'h00000000;
        endcase
    end

    //---------------------------------------------------------
    // Simple visible test pattern
    //
    // Draws a 32x32 square at target_row/target_col.
    // Color comes from data_buffer bits.
    //---------------------------------------------------------
    logic box_on;

    always_comb begin
        box_on =
            video_on &&
            (pixel_row >= target_row) &&
            (pixel_row <  target_row + 12'd32) &&
            (pixel_col >= target_col) &&
            (pixel_col <  target_col + 12'd32);

        if (!video_on) begin
            vga_red   = 4'h0;
            vga_green = 4'h0;
            vga_blue  = 4'h0;
        end else if (box_on) begin
            vga_red   = {4{data_buffer[2]}};
            vga_green = {4{data_buffer[1]}};
            vga_blue  = {4{data_buffer[0]}};
        end else begin
            vga_red   = 4'h0;
            vga_green = 4'h0;
            vga_blue  = 4'h2;
        end
    end

endmodule