/*******************************
* Frame Merge IP
* Wahid Rahman
* Jan. 30, 2014
* ECE532 Winter 2014
* University of Toronto
* *****************************/

module frame_merge_core #(
    parameter HRES              = 640,
    parameter VRES              = 480,
    parameter BYTES_PER_PIXEL   = 4,
    parameter DWIDTH            = 32,
    parameter AWIDTH            = 32,
    parameter SLV_REG_DWIDTH    = 32
)
(
    // Inputs
    input wire                      clk,
    input wire                      reset,
    
    input wire  [DWIDTH-1:0]        draw_fifo_data_in,
    input wire  [DWIDTH-1:0]        video_fifo_data_in,
    
    input wire                      comp_fifo_full,
    input wire                      comp_fifo_empty,
    input wire                      draw_fifo_full,
    input wire                      draw_fifo_empty,
    input wire                      video_fifo_full,
    input wire                      video_fifo_empty,

    // Outputs
    output wire                     draw_fifo_sel,
    output wire                     draw_fifo_read,

    output wire                     video_fifo_sel,
    output wire                     video_fifo_read,

	 output wire							comp_fifo_sel,
    output wire                     comp_fifo_write,
    output wire [DWIDTH-1:0]        comp_fifo_data_out,
    

    // Master Controller signals
    input wire                      master_busy,    // Toggles high one cycle
	 input wire								bus_ack,

    output wire                     rd_req,
    output wire                     wr_req,
    output wire [AWIDTH-1:0]        addr,
    output wire                     go,

    // Slave regs
    input wire  [SLV_REG_DWIDTH-1:0]    slave_go,
    input wire  [SLV_REG_DWIDTH-1:0]    slave_draw_addr,
    input wire  [SLV_REG_DWIDTH-1:0]    slave_video_addr,
    input wire  [SLV_REG_DWIDTH-1:0]    slave_comp_addr
);

    // High level description of procedure:
    // 1) Read Draw Frame pixel
    // 2) Read Video Frame pixel
    // 3) "Merge" the two pixels
    // 4) Output resulting pixel to Comp Frame


    // The following procedures are a first-attempt.
    // It can be optimized, e.g. removing wait states
    // and pipelining instead

    // Read operations (1 and 2)
    // Request a read from DataFrame addr
    // Wait until DrawFifo full
    // (should be same as mst_cmd_sm_set_done)
    // Do the same for VideoFrame addr

    // Merge operation (3)
    // Make sure output fifo empty (wait till it is)
    // Read DrawFifo pixel
    // Read VideoFifo pixel
    // Is DrawFifo pixel non white?
    //      output pixel = DrawFifo pixel
    // else:
    //      output pixel = VideoFifo pixel

    // Write operations (4)
    // Write pixel data (4 bytes) to output fifo until full.
    // Then do a write request (to Comp Frame in memory).
    // Wait until fifo empty (should be same as
    // mst_cmd_sm_set_done). 
    // This means the line has been written to memory.

    localparam STATE_REG_WIDTH      = 6;

    localparam IDLE                 = 6'd0;
    localparam WAIT_MASTER_BUSY     = 6'd1;
    localparam CHECK_COMP_EMPTY     = 6'd2;
    localparam READ_DRAW            = 6'd3;
    localparam READ_DRAW_DLY        = 6'd4;
    localparam READ_DRAW_DLY2       = 6'd5;
    localparam WAIT_DRAW_RD         = 6'd6;
    localparam READ_VIDEO           = 6'd7;
    localparam READ_VIDEO_DLY       = 6'd8;
    localparam READ_VIDEO_DLY2      = 6'd9;
    localparam WAIT_VIDEO_RD        = 6'd10;
    localparam READ_INPUT_FIFOS     = 6'd11;
    localparam STORE_INPUT_DATA    	= 6'd12;
    localparam MERGE_PIXELS         = 6'd13;
    localparam WRITE_OUTPUT_FIFOS   = 6'd14;
    localparam WRITE_COMP           = 6'd15;
    localparam WRITE_COMP_DLY       = 6'd16;
    localparam WRITE_COMP_DLY2      = 6'd17;
    localparam WAIT_COMP_WR         = 6'd18;
    localparam CHECK_LINE_CNT       = 6'd19;
    localparam DONE                 = 6'd20;

    // Test params
    localparam DDR_BASE_ADDR        = 32'hA8000000;
    localparam FRAME_SIZE_BYTES     = 32'h400000;   // at most 1280x720 pixels x 4 bytes/pixel + padding
    localparam DRAW_BASE_ADDR       = DDR_BASE_ADDR;
    localparam VIDEO_BASE_ADDR      = DRAW_BASE_ADDR + FRAME_SIZE_BYTES;
    localparam COMP_BASE_ADDR       = VIDEO_BASE_ADDR + FRAME_SIZE_BYTES;


    // Output registers for assignment
    reg                             r_draw_fifo_sel;
    reg                             r_draw_fifo_read;

    reg                             r_video_fifo_sel;
    reg                             r_video_fifo_read;

    reg                             r_comp_fifo_write;
	 reg										r_comp_fifo_sel;
    reg     [DWIDTH-1:0]            r_comp_fifo_data_out;

    reg                             r_rd_req;
    reg                             r_wr_req;
    reg     [AWIDTH-1:0]            r_addr;
    reg                             r_go;

    assign draw_fifo_sel = r_draw_fifo_sel;
    assign draw_fifo_read = r_draw_fifo_read;
    
    assign video_fifo_sel = r_video_fifo_sel;
    assign video_fifo_read = r_video_fifo_read;

	assign comp_fifo_sel = r_comp_fifo_sel;
    assign comp_fifo_write = r_comp_fifo_write;
    assign comp_fifo_data_out = r_comp_fifo_data_out;

    assign rd_req = r_rd_req;
    assign wr_req = r_wr_req;

    assign addr = r_addr;
    assign go = r_go;

    // Internal registers
    reg     [DWIDTH-1:0]            r_draw_fifo_data;
    reg     [DWIDTH-1:0]            r_video_fifo_data;
    reg     [STATE_REG_WIDTH-1:0]   cs;
    reg     [STATE_REG_WIDTH-1:0]   ns;
    reg     [31:0]                   line_cnt;       // lines per frame
    reg     [31:0]                   pixel_cnt;      // pixels per line
    reg                             set_done_flag;
    reg                             done;
    reg     [AWIDTH-1:0]            line_offset;

    wire                            start_merge;
    assign                          start_merge = slave_go[0];

    // Current State
    always @(posedge clk)
    begin
        if (reset)
        begin
            cs <= IDLE;
        end
        else
        begin
            cs <= ns;
        end
    end

    // Next State Logic
    always @(*)
    begin
        if (reset)
        begin
            ns <= IDLE;
        end
        else
        begin
            case (cs)
                IDLE:
                begin
                    if (start_merge)
                    begin
                        ns <= WAIT_MASTER_BUSY;
                    end
                    else
                    begin
                        ns <= IDLE;
                    end
                end
                WAIT_MASTER_BUSY:
                begin
                    if (master_busy)
                    begin
                        ns <= WAIT_MASTER_BUSY;
                    end
                    else
                    begin
                        //ns <= CHECK_COMP_EMPTY;   -- skip for now
                        ns <= READ_DRAW;
                    end
                end
                CHECK_COMP_EMPTY:       ;   // Skip for now, what should we do?
                READ_DRAW:
                begin
							if(bus_ack)
							begin
								ns <= WAIT_DRAW_RD;	// move on if acknowledged
							end
							else
							begin
								ns <= READ_DRAW;		// otherwise wait
							end
                end
                READ_DRAW_DLY:	// skip
                begin
                    ns <= READ_DRAW_DLY2;
                end
                READ_DRAW_DLY2:	// skip
                begin
                    ns <= WAIT_DRAW_RD;
                end
                WAIT_DRAW_RD:
                begin
                    if (master_busy)
                    begin
                        ns <= WAIT_DRAW_RD;
                    end
                    else
                    begin
                        ns <= READ_VIDEO;
                    end
                end
                READ_VIDEO:
                begin
							if (bus_ack)
							begin
								ns <= WAIT_VIDEO_RD;
							end
							else
							begin
								ns <= READ_VIDEO;
							end
                end
                READ_VIDEO_DLY:	//skip
                begin
                    ns <= READ_VIDEO_DLY2;
                end
                READ_VIDEO_DLY2:	//skip
                begin
                    ns <= WAIT_VIDEO_RD;
                end
                WAIT_VIDEO_RD:
                begin
                    if (master_busy)
                    begin
                        ns <= WAIT_VIDEO_RD;
                    end
                    else
                    begin
                        ns <= READ_INPUT_FIFOS;
                    end
                end
                READ_INPUT_FIFOS:
                begin
                    ns <= STORE_INPUT_DATA;
                end
                STORE_INPUT_DATA:
                begin
                    ns <= MERGE_PIXELS;
                end
                MERGE_PIXELS:
                begin
                    ns <= WRITE_OUTPUT_FIFOS;
                end
                WRITE_OUTPUT_FIFOS:
                begin
                    if (pixel_cnt >= HRES-2)     // Processed entire line, given one cycle delay from ns --> cs
                    begin
                        ns <= WRITE_COMP;
                    end
                    else
                    begin
                        ns <= READ_INPUT_FIFOS;     // Loop till we've processed entire line
                    end
                end
                WRITE_COMP:
                begin
							if (bus_ack)
							begin
								ns <= WAIT_COMP_WR;
							end
							else
							begin
								ns <= WRITE_COMP;
							end
                end
                WRITE_COMP_DLY:	//skip
                begin
                    ns <= WRITE_COMP_DLY2;
                end
                WRITE_COMP_DLY2:	//skip
                begin
                    ns <= WAIT_COMP_WR;
                end
                WAIT_COMP_WR:
                begin
                    if (master_busy)
                    begin
                        ns <= WAIT_COMP_WR;
                    end
                    else
                    begin
                        ns <= CHECK_LINE_CNT;
                    end
                end
                CHECK_LINE_CNT:
                begin
                    if (line_cnt >= VRES)    // Processed entire frame, done
                    begin
                        ns <= DONE;
                    end
                    else
                    begin
                        ns <= READ_DRAW;
                    end
                end
                DONE:
                begin
                    ns <= IDLE;
                end
            endcase
        end
    end

    // Control signals
    always @(posedge clk)
    begin
        if (reset)
        begin
            // Reset
            line_cnt                <= 0;
            pixel_cnt               <= 0;
            set_done_flag           <= 0;
            r_draw_fifo_sel         <= 0;
            r_draw_fifo_read        <= 0;
            r_video_fifo_sel        <= 0;
            r_video_fifo_read       <= 0;
            r_comp_fifo_write       <= 0;
				r_comp_fifo_sel			<= 0;
            r_rd_req                <= 0;
            r_wr_req                <= 0;
            r_addr                  <= 0;
            r_go                    <= 0;
        end
        else
        begin
            // Default
            line_cnt                <= line_cnt;
            pixel_cnt               <= pixel_cnt;
            set_done_flag           <= 0;
            r_draw_fifo_sel         <= 0;
            r_draw_fifo_read        <= 0;
            r_video_fifo_sel        <= 0;
            r_video_fifo_read       <= 0;
            r_comp_fifo_write       <= 0;
				r_comp_fifo_sel			<= 0;
            r_rd_req                <= 0;
            r_wr_req                <= 0;
            r_addr                  <= 0;
            r_go                    <= 0;

            // State conditions
            case (cs)
                IDLE:
                begin
                    line_cnt        <= 0;
                    pixel_cnt       <= 0;
                end
                WAIT_MASTER_BUSY:
                begin
                    line_cnt            <= line_cnt + 1;    // Do it here for the first time to prepare line_offset in advance
                end
                CHECK_COMP_EMPTY:       ;
                READ_DRAW:
                begin
                    pixel_cnt           <= 0;
                    r_draw_fifo_sel     <= 1'b1;
                    r_rd_req            <= 1'b1;
                    r_addr              <= slave_draw_addr + line_offset;        // --wr CHANGE! READ FROM A SLAVE!
                    r_go                <= 1'b1;
                end
                READ_DRAW_DLY:
                begin
                    r_draw_fifo_sel     <= 1'b1;
                    r_rd_req            <= 1'b1;
                    r_addr              <= slave_draw_addr + line_offset;        // --wr CHANGE! READ FROM A SLAVE!
                    r_go                <= 1'b1;
                end
                READ_DRAW_DLY2:
                begin
                    r_draw_fifo_sel     <= 1'b1;
                    r_rd_req            <= 1'b1;
                    r_addr              <= slave_draw_addr + line_offset;        // --wr CHANGE! READ FROM A SLAVE!
                    r_go                <= 1'b1;
                end
                WAIT_DRAW_RD:
                begin
                    r_draw_fifo_sel     <= 1'b1;
                    r_addr              <= slave_draw_addr + line_offset;        // --wr CHANGE! READ FROM A SLAVE!
                end
                READ_VIDEO:
                begin
                    r_video_fifo_sel    <= 1'b1;
                    r_rd_req            <= 1'b1;
                    r_addr              <= slave_video_addr + line_offset;        // --wr CHANGE! READ FROM A SLAVE!
                    r_go                <= 1'b1;
                end
                READ_VIDEO_DLY:
                begin
                    r_video_fifo_sel    <= 1'b1;
                    r_rd_req            <= 1'b1;
                    r_addr              <= slave_video_addr + line_offset;        // --wr CHANGE! READ FROM A SLAVE!
                end
                READ_VIDEO_DLY2:
                begin
                    r_video_fifo_sel    <= 1'b1;
                    r_rd_req            <= 1'b1;
                    r_addr              <= slave_video_addr + line_offset;        // --wr CHANGE! READ FROM A SLAVE!
                end
                WAIT_VIDEO_RD:
                begin
                    r_video_fifo_sel    <= 1'b1;
                    r_addr              <= slave_video_addr + line_offset;        // --wr CHANGE! READ FROM A SLAVE!
                end
                READ_INPUT_FIFOS:
                begin
                    r_draw_fifo_read    <= 1'b1;
                    r_video_fifo_read   <= 1'b1;
                end
                STORE_INPUT_DATA:       ;
                MERGE_PIXELS:
                begin
                    pixel_cnt           <= pixel_cnt + 1;
                end
                WRITE_OUTPUT_FIFOS:
                begin
                    r_comp_fifo_write   <= 1'b1;
                end
                WRITE_COMP:
                begin
                    r_wr_req            <= 1'b1;
						  r_comp_fifo_sel		<= 1'b1;
                    r_addr              <= slave_comp_addr + line_offset;
                    r_go                <= 1'b1;
                end
                WRITE_COMP_DLY:
                begin
                    r_wr_req            <= 1'b1;
						  r_comp_fifo_sel		<= 1'b1;
                    r_addr              <= slave_comp_addr + line_offset;
                end
                WRITE_COMP_DLY2:
                begin
                    r_wr_req            <= 1'b1;
						  r_comp_fifo_sel		<= 1'b1;
                    r_addr              <= slave_comp_addr + line_offset;
                end
                WAIT_COMP_WR:
                begin
							r_comp_fifo_sel		<= 1'b1;
                    r_addr              <= slave_comp_addr + line_offset;
                end
                CHECK_LINE_CNT:         
					 begin
							line_cnt 			<= line_cnt + 32'd1;		// For every iteration of the loop
					 end
                DONE:
                begin
                    set_done_flag       <= 1'b1;
                end
            endcase
        end

    end

    // Data Path

    // Pixel is R_G_B_XX, assume for white XX = FF
    assign is_draw_white    = (r_draw_fifo_data == 32'hFFFFFFFF) ?
                                1'b1 : 1'b0;
    // Pixel registers
    always @(posedge clk)
    begin
        if (reset)
        begin
            r_draw_fifo_data        <= 32'h0;
            r_video_fifo_data       <= 32'h0;
            r_comp_fifo_data_out    <= 32'h0;
        end
        else
        begin
            r_draw_fifo_data        <= (cs == STORE_INPUT_DATA) ? 
                                        draw_fifo_data_in : 
                                        r_draw_fifo_data;

            r_video_fifo_data       <= (cs == STORE_INPUT_DATA) ? 
                                        video_fifo_data_in : 
                                        r_video_fifo_data;

            r_comp_fifo_data_out    <= (cs == MERGE_PIXELS) ?
                                            (is_draw_white) ?
                                            r_video_fifo_data :
                                            r_draw_fifo_data
                                        : r_comp_fifo_data_out;
        end
    end

    // Done flag
    always @(posedge clk)
    begin
        if (reset)
        begin
            done    <= 1'b0;
        end
        else
        begin
            if (cs == WAIT_MASTER_BUSY)     // Starting state
            begin
                done    <= 1'b0;       // Clear the done flag
            end
            else
            begin
                done    <= set_done_flag | done;
            end
        end
    end

    // Line address offset
    always @(posedge clk)
    begin
        if (reset)
        begin
            line_offset <= 32'h0;       // AWIDTH = 32
        end
        else
        begin
            line_offset <= (line_cnt-1)*32'd1280*32'd4;
        end
    end

endmodule

