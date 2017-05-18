// Copyright 1986-2016 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2016.3_sdx (win64) Build 1721784 Tue Nov 29 22:12:44 MST 2016
// Date        : Tue May 16 06:18:53 2017
// Host        : SakinderLaptop1 running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub
//               c:/Sim/sdaccel_ku/xcl_design/xcl_design.srcs/sources_1/bd/xcl_design/ip/xcl_design_slice_reset_system_pr_0/xcl_design_slice_reset_system_pr_0_stub.v
// Design      : xcl_design_slice_reset_system_pr_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xcku115-flvb2104-2-e
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* X_CORE_INFO = "xlslice,Vivado 2016.3_sdx" *)
module xcl_design_slice_reset_system_pr_0(Din, Dout)
/* synthesis syn_black_box black_box_pad_pin="Din[1:0],Dout[0:0]" */;
  input [1:0]Din;
  output [0:0]Dout;
endmodule