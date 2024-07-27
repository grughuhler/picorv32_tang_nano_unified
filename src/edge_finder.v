/* Copyright 2024 Grug Huhler.  License SPDX BSD-2-Clause.

   edge_finder raises pulse for one clock when it sees (by
   sampling) a rising edge on sig_in.
*/

module edge_finder
(
   input wire  clk,
   input wire  reset_n,
   input wire  sig_in,
   output reg  pulse
);

`include "global_defs.v"

/* states */
localparam AWAIT_LOW = 2'b00;
localparam AWAIT_HIGH = 2'b01;

wire sig;
reg state = AWAIT_LOW;

`ifdef BOARD_20K
   // Button on board is high when pressed
   assign sig = sig_in;
`endif
`ifdef BOARD_9K
   // Button on board is low when pressed
   assign sig = ~sig_in;
`endif

always @(posedge clk or negedge reset_n)
   if (!reset_n) begin
      state <= AWAIT_LOW;
      pulse = 1'b0;
   end
   else
      case (state)
         AWAIT_LOW: begin
            pulse <= 1'b0;
            if (sig)
               state <= AWAIT_LOW;
            else
               state <= AWAIT_HIGH;
         end
         AWAIT_HIGH: begin
            if (sig) begin
               state <= AWAIT_LOW;
               pulse <= 1'b1;
            end else begin
                state <= AWAIT_HIGH;
            end
         end
      endcase

endmodule


