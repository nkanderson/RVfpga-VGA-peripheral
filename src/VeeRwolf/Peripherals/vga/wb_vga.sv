//=============================================================
//   Chris Kane-Pardy & Niklas Anderson
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
    localparam logic [3:0] REG_MODE  = 4'h0;
    localparam logic [3:0] REG_COORD = 4'h1;
    localparam logic [3:0] REG_DATA  = 4'h3;
    localparam logic [3:0] REG_CHAR  = 4'h4;

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
    // Font ROM signals
    //---------------------------------------------------------
    logic [6:0]  font_char_index;
    logic [3:0]  font_x;
    logic [3:0]  font_y;
    logic [10:0] font_addr;
    logic [15:0] font_bits;

    logic        char_area;
    logic        glyph_pixel;
    
    font_rom u_font_rom (
        .clka  (wb_clk_i),
        .addra (font_addr),
        .douta (font_bits)
    );

    //---------------------------------------------------------
    // CPU-visible registers
    //---------------------------------------------------------
    logic        mode; // 1 = text mode, 0 = graphics mode
    logic [9:0]  target_row;
    logic [9:0]  target_col;
    // Foreground: [15:12]=R, [11:8]=G, [7:4]=B; Background: [31:28]=R, [27:24]=G, [23:20]=B
    // Graphics mode uses only the foreground color, text mode uses both.
    logic [31:0] data_buffer;
    logic [7:0]  char_code_reg;

    //---------------------------------------------------------
    // Wishbone acknowledge and writes
    //---------------------------------------------------------
    always_ff @(posedge wb_clk_i) begin
        if (wb_rst_i) begin
            wb_ack_o    <= 1'b0;
            mode        <= 1'b0;
            target_row  <= 10'd100;
            target_col  <= 10'd100;
            data_buffer <= 32'h0F0F0F0F; // Foreground white, background white
            char_code_reg   <= 8'h01; // Default to a printable character
        end else begin
            wb_ack_o <= wb_access && !wb_ack_o;

            if (wb_access && wb_we_i && !wb_ack_o) begin
                unique case (reg_sel)
                    REG_MODE: begin
                        mode <= wb_dat_i[0];
                    end

                    REG_COORD: begin
                        target_row <= wb_dat_i[19:10];
                        target_col <= wb_dat_i[9:0];
                    end

                    REG_DATA: begin
                        data_buffer <= wb_dat_i;
                    end

                    REG_CHAR: begin
                        // The font ROM skips the first 32 ASCII characters (which are non-printable),
                        // so we need to adjust the character code accordingly.
                        char_code_reg <= wb_dat_i[7:0] < 8'd32 ? 8'd0 : wb_dat_i[7:0] - 8'd32;
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
            REG_MODE:  wb_dat_o = {31'd0, mode};
            REG_COORD: wb_dat_o = {12'd0, target_row, target_col};
            REG_DATA:  wb_dat_o = data_buffer;
            REG_CHAR:  wb_dat_o = {24'd0, (char_code_reg + 8'd32)}; // Add back the offset for ASCII
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
        // Pre-case default assignments to avoid latch inference
        font_char_index = 7'h00;
        font_x = 4'h0;
        font_y = 4'h0;
        font_addr = 11'h000;
        char_area = 1'b0;
        glyph_pixel = 1'b0;
        box_on = 1'b0;
        if (mode) begin
            // Text mode: determine char code and glyph pixel
            // TODO: font_bits is valid one cycle after font_addr is driven, so we should
            // pipeline the pixel coordinates by one cycle.

            char_area =
                video_on &&
                (pixel_row >= target_row) &&
                (pixel_row <  target_row + 12'd16) &&
                (pixel_col >= target_col) &&
                (pixel_col <  target_col + 12'd16);

            font_y = pixel_row[3:0] - target_row[3:0];
            font_x = pixel_col[3:0] - target_col[3:0];

            font_addr = {char_code_reg[6:0], font_y};

            glyph_pixel = char_area && font_bits[15 - font_x];

            if (!video_on) begin
                vga_red   = 4'h0;
                vga_green = 4'h0;
                vga_blue  = 4'h0;
            end else if (glyph_pixel) begin
                // Foreground color: bits [15:12]=R, [11:8]=G, [7:4]=B
                vga_red   = data_buffer[15:12];
                vga_green = data_buffer[11:8];
                vga_blue  = data_buffer[7:4];
            end else if (char_area) begin
                // Background color: bits [31:28]=R, [27:24]=G, [23:20]=B
                vga_red   = data_buffer[31:28];
                vga_green = data_buffer[27:24];
                vga_blue  = data_buffer[23:20];
            end else begin
                vga_red   = 4'h2;
                vga_green = 4'h0;
                vga_blue  = 4'h0;
            end
        end else begin
            // Graphics mode: determine if we're in the box area
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
                // Foreground color: bits [15:12]=R, [11:8]=G, [7:4]=B
                vga_red   = data_buffer[15:12];
                vga_green = data_buffer[11:8];
                vga_blue  = data_buffer[7:4];
            end else begin
                vga_red   = 4'h0;
                vga_green = 4'h0;
                vga_blue  = 4'h2;
            end
        end
        
    end

endmodule
