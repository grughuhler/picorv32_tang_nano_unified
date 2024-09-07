module two_flop_sync
(
 input wire clk_b, // clk of receiving domain
 input wire reset_n,
 input wire sig_in,
 output reg sig_out
 );

   reg 	    sync_flop;
   
   always @(posedge clk_b or negedge reset_n)
     if (!reset_n) begin
	sync_flop <= 0;
	sig_out <= 0;
     end else begin
	sync_flop <= sig_in;
	sig_out <= sync_flop;
     end

endmodule
