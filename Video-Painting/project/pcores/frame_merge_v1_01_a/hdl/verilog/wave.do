vlib work
vlog -novopt -work work frame_merge_core.v
vlog -novopt -work work ../vhdl

force -freeze sim:/user_logic/Bus2IP_Clk 1 0, 0 {5000 ps} -r {10 ns}
force -freeze sim:/user_logic/Bus2IP_Resetn 0 0
force -freeze sim:/user_logic/Bus2IP_Data 0 0
force -freeze sim:/user_logic/Bus2IP_BE 0 0
force -freeze sim:/user_logic/Bus2IP_RdCE 0 0
force -freeze sim:/user_logic/Bus2IP_WrCE 0 0
force -freeze sim:/user_logic/bus2ip_mst_cmdack 0 0
force -freeze sim:/user_logic/bus2ip_mst_cmplt 0 0
force -freeze sim:/user_logic/bus2ip_mst_error 0 0
force -freeze sim:/user_logic/bus2ip_mst_rearbitrate 0 0
force -freeze sim:/user_logic/bus2ip_mst_cmd_timeout 0 0
force -freeze sim:/user_logic/bus2ip_mstrd_d 0 0
force -freeze sim:/user_logic/bus2ip_mstrd_rem 0 0
force -freeze sim:/user_logic/bus2ip_mstrd_sof_n 1 0
force -freeze sim:/user_logic/bus2ip_mstrd_eof_n 1 0
force -freeze sim:/user_logic/bus2ip_mstrd_src_rdy_n 1 0
force -freeze sim:/user_logic/bus2ip_mstrd_src_dsc_n 1 0
force -freeze sim:/user_logic/ip2bus_mstrd_dst_rdy_n 1 0
force -freeze sim:/user_logic/ip2bus_mstrd_dst_dsc_n 1 0
force -freeze sim:/user_logic/bus2ip_mstwr_dst_rdy_n 1 0
force -freeze sim:/user_logic/bus2ip_mstwr_dst_dsc_n 1 0
