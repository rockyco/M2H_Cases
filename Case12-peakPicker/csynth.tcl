open_project proj_peakPicker
set_top peakPicker
add_files peakPicker.cpp
add_files -tb peakPicker_tb.cpp
add_files -tb ./locations_3_ref.txt
add_files -tb ./peakLocs_out.txt
add_files -tb ./pssCorrMagSq_3_in.txt
add_files -tb ./threshold_in.txt
open_solution solution1
set_part {xczu28dr-ffvg1517-2-e}
create_clock -period 3.33 -name default
set_clock_uncertainty 12.5%
config_compile -pipeline_loops 1
config_interface -m_axi_addr64=0
config_rtl -reset control
config_dataflow -default_channel fifo -fifo_depth 2
csynth_design
exit
