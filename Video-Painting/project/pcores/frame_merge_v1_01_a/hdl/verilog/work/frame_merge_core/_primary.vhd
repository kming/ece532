library verilog;
use verilog.vl_types.all;
entity frame_merge_core is
    generic(
        HRES            : integer := 640;
        VRES            : integer := 480;
        DWIDTH          : integer := 32;
        AWIDTH          : integer := 32;
        SLV_REG_DWIDTH  : integer := 32
    );
    port(
        clk             : in     vl_logic;
        reset           : in     vl_logic;
        draw_fifo_data_in: in     vl_logic_vector;
        video_fifo_data_in: in     vl_logic_vector;
        comp_fifo_full  : in     vl_logic;
        comp_fifo_empty : in     vl_logic;
        draw_fifo_full  : in     vl_logic;
        draw_fifo_empty : in     vl_logic;
        video_fifo_full : in     vl_logic;
        video_fifo_empty: in     vl_logic;
        draw_fifo_sel   : out    vl_logic;
        draw_fifo_read  : out    vl_logic;
        video_fifo_sel  : out    vl_logic;
        video_fifo_read : out    vl_logic;
        comp_fifo_write : out    vl_logic;
        comp_fifo_data_out: out    vl_logic_vector;
        master_busy     : in     vl_logic;
        rd_req          : out    vl_logic;
        wr_req          : out    vl_logic;
        addr            : out    vl_logic_vector;
        go              : out    vl_logic;
        slave_go        : in     vl_logic_vector;
        slave_draw_addr : in     vl_logic_vector;
        slave_video_addr: in     vl_logic_vector;
        slave_comp_addr : in     vl_logic_vector
    );
    attribute mti_svvh_generic_type : integer;
    attribute mti_svvh_generic_type of HRES : constant is 1;
    attribute mti_svvh_generic_type of VRES : constant is 1;
    attribute mti_svvh_generic_type of DWIDTH : constant is 1;
    attribute mti_svvh_generic_type of AWIDTH : constant is 1;
    attribute mti_svvh_generic_type of SLV_REG_DWIDTH : constant is 1;
end frame_merge_core;
