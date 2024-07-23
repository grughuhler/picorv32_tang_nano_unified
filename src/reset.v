/* Copyright 2024 Grug Huhler.  License SPDX BSD-2-Clause. */

module reset_control
(
 input wire  clk,
 input wire  reset_button,
 output wire reset_n
);

`include "global_defs.v"

   wire reset_in;
   reg [5:0] reset_count = 0;

`ifdef BOARD_20K
   // Button on board is high when pressed
   assign reset_in = reset_button;
`endif
`ifdef BOARD_9K
   // Button on board is low when pressed
   assign reset_in = ~reset_button;
`endif

   // picorv32 must see a reset_n rising edge so hold it active
   // until a count completes.
   assign reset_n = &reset_count;

   always @(posedge clk)
     if (reset_in)
       reset_count <= 'b0;
     else
       reset_count <= reset_count + !reset_n;

endmodule
