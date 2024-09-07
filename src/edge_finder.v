/* Copyright 2024 Grug Huhler.  License SPDX BSD-2-Clause.

   edge_finder raises pulse for one clock when it sees (by
   sampling) a rising edge on sig_in.
*/

module falling_edge_finder
(
 input wire clk,
 input wire reset_n,
 input wire sig_in,
 output reg pulse
);

`include "global_defs.v"

   reg      stage1;
   reg      stage2;
   reg      stage3;
   reg      stage4;

   always @(posedge clk or negedge reset_n)
     if (!reset_n) begin
        stage1 <= 1'b0;
        stage2 <= 1'b0;
        stage3 <= 1'b0;
        stage4 <= 1'b0;
     end
     else begin
        stage1 <= sig_in;  /* might be metastable */
        stage2 <= stage1;
        stage3 <= stage2;
        stage4 <= stage3; /* stage4 is the oldest data */
        /* look for falling edge */
        if (stage4 == 1'b1 && stage3 == 1'b0)
          pulse <= 1'b1;
        else
          pulse <= 1'b0;
     end
   
endmodule

module rising_edge_finder
(
 input wire clk,
 input wire reset_n,
 input wire sig_in,
 output reg pulse
);

`include "global_defs.v"

   reg      stage1;
   reg      stage2;
   reg      stage3;
   reg      stage4;

   always @(posedge clk or negedge reset_n)
     if (!reset_n) begin
        stage1 <= 1'b0;
        stage2 <= 1'b0;
        stage3 <= 1'b0;
        stage4 <= 1'b0;
     end
     else begin
        stage1 <= sig_in;  /* might be metastable */
        stage2 <= stage1;
        stage3 <= stage2;
        stage4 <= stage3; /* stage4 is the oldest data */
        /* look for falling edge */
        if (stage4 == 1'b0 && stage3 == 1'b1)
          pulse <= 1'b1;
        else
          pulse <= 1'b0;
     end
   
endmodule

