/* Copyright 2024 Grug Huhler.  License SPDX BSD-2-Clause. */

`include "../global_defs.v"

/* pps_wrapper
 *
 * This module allows the RISC-V core access to registers of the pps_timer.
 * The registers are:
 *   0: time stamp low 32 bits (read only from the core)
 *   1: time stamp high 32 bits (read only from the core)
 *   2: accum_incr: (write only from the core)
 *   3: pps_count: (write only from the core)
 *   4: time_incr (write only from the core)
 *   5: event stamp low 32 bits (read only from core)
 *   6: event stamp high 32 bits (read only from core)
 */

module pps_wrapper
  (
   input wire         clk_in, /* The 27 MHz input clock */
   input wire         clk, /* The system clock */
   input wire         reset_n,
   input wire         sel,
   input wire [2:0]   addr, /* This is a word (not byte) address */ 
   input wire         is_write, 
   input wire [31:0]  data_i,
   output reg         ready,
   output wire [31:0] data_o,
   input wire         event_in,
   input wire         pps_in,
   output wire        pps_pulse_out
   );

   localparam AWAIT_SEL = 3'b000;
   localparam FIFO_WRITE = 3'b001;
   localparam AWAIT_FIFO = 3'b010;
   localparam FIFO_READ = 3'b011;
   localparam WAIT = 3'b100;

   wire               clk_pps;
   wire               from_pps_wr_en;
   reg                from_pps_rd_en;
   wire               from_pps_empty;
   wire               from_pps_full;
   reg                to_pps_wr_en;
   wire               to_pps_rd_en;
   wire               to_pps_empty;
   wire               to_pps_full;
   wire [31:0]        data_from_pps;
   wire [35:0]        data_to_pps;
   reg [2:0]          state;

   /* Much of the pps_timer operates in a different clock domain from the
    * main system and picorv32 core.  This module uses a Gowin rPLL to
    * create the clock (clk_pps) for the pps_timer.  For better
    * timestamps, clk_pps should be fast, like 120 MHz.
    * 
    * Gowin FIFOs are used to pass timestamp and register values between
    * the system clock domain and the pps_timer clock domain.
    */

`ifdef BOARD_9K
   localparam TIME_INCR_VAL = 'd20;
   Gowin_rPLL_PPS_9K rpll_pps
     (
      .clkout(clk_pps), /* 60 MHz */
      .clkin(clk_in)
      );
`endif

`ifdef BOARD_20K
   localparam TIME_INCR_VAL = 'd10;
   Gowin_rPLL_PPS_20K rpll_pps
     (
      .clkout(clk_pps), /* 120 MHz */
      .clkin(clk_in)
      );
`endif

   /* FIFOs are needed because pps.v is in a different clock domain */

   fifo_to_pps to_pps
     (
      .Data({is_write, addr, data_i}),
      .Reset(~reset_n),
      .WrClk(clk),
      .RdClk(clk_pps),
      .WrEn(to_pps_wr_en),
      .RdEn(to_pps_rd_en),
      .Q(data_to_pps),
      .Empty(to_pps_empty),
      .Full(to_pps_full)
      );

   fifo_from_pps from_pps
     (
      .Data(data_from_pps),
      .Reset(~reset_n),
      .WrClk(clk_pps),
      .RdClk(clk),
      .WrEn(from_pps_wr_en),
      .RdEn(from_pps_rd_en),
      .Q(data_o),
      .Empty(from_pps_empty),
      .Full(from_pps_full)
      );

   /* This block implements a state machine for reads and writes
    * from the RISC-V core.
    * 
    * For reads and writes, the machine must write {is_write, addr, data_o}
    * to FIFO to_pps.
    * 
    * For writes, the jobs is done.  Just assert ready.  For reads, the
    * machine must await data from FIFO from_pps and pass it out on
    * data_o.
    */

   always @(posedge clk or negedge reset_n)
     if (!reset_n) begin
        ready <= 1'b0;
        from_pps_rd_en <= 1'b0;
        to_pps_wr_en <= 1'b0;
        state <= 'b0;
     end else begin
        case (state)
          AWAIT_SEL:
            begin
               ready <= 1'b0;
               if (sel & ~to_pps_full) begin
                  state <= FIFO_WRITE;
                  to_pps_wr_en <= 1'b1;
               end
               else
                 state <= AWAIT_SEL;
            end
          FIFO_WRITE:
            begin
               /* FIFO accept data in this state */
               to_pps_wr_en <= 1'b0;
               if (is_write) begin
                  ready <= 1'b1;
                  state <= AWAIT_SEL;
               end else
                 state <= AWAIT_FIFO;
            end
          AWAIT_FIFO:
            begin
               if (!from_pps_empty) begin
                  from_pps_rd_en <= 1'b1;
                  state <= FIFO_READ;
               end else
                 state <= AWAIT_FIFO;
            end
          FIFO_READ:
            begin
               /* FIFO is sampled in this state */
               from_pps_rd_en <= 1'b0;
               ready <= 1'b1;
               state <= WAIT;
            end
          WAIT:
            begin
               /* This state is needed to allow sel to go low */
               ready <= 1'b0;
               state <= AWAIT_SEL;
            end
        endcase
     end

   /* Instantiate the module that is the heart of the pps_timer and
    * includes the FIFOs.  Many of the signals relate to the FIFOs.
    */
   
   pps_timer #(.TIME_INCR_VAL(TIME_INCR_VAL)) pps_timer0
     (
      .clk_pps(clk_pps),
      .reset_n(reset_n),
      .pps_in(pps_in),
      .event_in(event_in),
      .data_to_pps(data_to_pps),
      .to_pps_rd_en(to_pps_rd_en),
      .to_pps_empty(to_pps_empty),
      .data_from_pps(data_from_pps),
      .from_pps_wr_en(from_pps_wr_en),
      .from_pps_full(from_pps_full),
      .pps_pulse_out(pps_pulse_out)
      );

endmodule // pps_wrapper
