/* Copyright 2024 Grug Huhler.  License SPDX BSD-2-Clause. */

module pps_timer #(parameter TIME_INCR_VAL='d10)
  (
   input wire         clk_pps,
   input wire         reset_n,
   input wire         pps_in,
   input wire         event_in,
   /* FIFO coming to here */
   input wire [35:0]  data_to_pps, /* is_write, addr, data in from FIFO */
   output reg         to_pps_rd_en,
   input wire         to_pps_empty,
   /* FIFO from here */
   output wire [31:0] data_from_pps, /* result for reads from core */
   output reg         from_pps_wr_en,
   input wire         from_pps_full,
   output reg         pps_pulse_out
   );

   localparam AWAIT_NONEMPTY = 2'b00;
   localparam FIFO_READ = 2'b01;
   localparam DO_OP = 2'b10;
   
   reg                reset_pps1_n;
   reg                reset_pps2_n;
   reg                reset_pps_n;
   wire               pps_in_sync;
   wire               event_in_sync;
   reg [63:0]         time_ctr;
   reg [31:0]         accum_ctr;
   reg [31:0]         accum_incr;
   reg                carry;
   reg [63:0]         timestamp;
   reg [7:0]          time_incr;
   reg [63:0]         eventstamp;
   reg [31:0]         pps_count;
   reg [31:0]         pps_ctr;
   wire               is_right;
   wire [2:0]         addr;
   wire [31:0]        value_to_write;
   reg [1:0]          state;

   /* Break the data from the incoming FIFO into its parts */
   assign is_write = data_to_pps[35];
   assign addr = data_to_pps[34:32];
   assign value_to_write = data_to_pps[31:0];

   /* For reads, connect the right thing to the outgoing FIFO */
   assign data_from_pps = (addr == 3'b000) ? timestamp[31:0] :
                          (addr == 3'b001) ? timestamp[63:32] :
                          (addr == 3'b101) ? eventstamp[31:0] :
                          (addr == 3'b110) ? eventstamp[63:32] : 32'hcafebabe;

   /* Synchronizer for the reset_n signal passing from the sytem to
    * the pps clock domain.  It helps that system clock domain reset_n
    * is a pretty wide pulse.
    */

   always @(posedge clk_pps)
     begin
        reset_pps1_n <= reset_n;
        reset_pps2_n <= reset_pps1_n;
        reset_pps_n <= reset_pps2_n;
     end

   /* Convert pps_in and event_in to single cycle pulses */

   rising_edge_finder pps_in_finder
     (
      .clk(clk_pps),
      .reset_n(reset_pps_n),
      .sig_in(pps_in),
      .pulse(pps_in_sync)
      );

   rising_edge_finder event_in_finder
     (
      .clk(clk_pps),
      .reset_n(reset_pps_n),
      .sig_in(event_in),
      .pulse(event_in_sync)
      );

   /* Implement the time_ctr, accum_ctr, and pps_ctr counters. Reg accum
    *  increments by accum_incr every clock_pps cycle.  This generates carry.
    * When carry is high, pps_ctr decrments, and time_ctr is incremented by
    * time_incr.
    */

   always @(posedge clk_pps or negedge reset_pps_n)
     if (!reset_pps_n) begin
        timestamp <= 'b0;
        eventstamp <= 'b0;
        time_ctr <= 'b0;
        accum_ctr <= 'b0;
        pps_ctr <= 'b1;
        carry <= 'b0;
        pps_pulse_out <= 'b0;
     end else begin

        {carry,accum_ctr} <= accum_ctr + accum_incr;

        if (pps_in_sync) begin
           timestamp <= time_ctr;
        end

        if (event_in_sync) begin
           eventstamp <= time_ctr;
        end

        if (carry) begin
           time_ctr <= time_ctr + time_incr;
           if (pps_ctr < pps_count)
             pps_ctr <= pps_ctr + 32'b1;
           else
             pps_ctr <= 32'b1;
           /* Set the PPS output pulse with duty cycle ~25% */        
           if (pps_ctr < (pps_count >> 2))
             pps_pulse_out <= 1'b1;
           else
             pps_pulse_out <= 1'b0;
        end

     end

   /* This is a state machine that handles reads and writes from the RISC-V
    * core.  It must read from a FIFO and then act based on what it read.
    * Writes do not require any response, but the result of reads must be
    * written to a FIFO to cross back into the system clock domain.
    */

   always @(posedge clk_pps or negedge reset_pps_n)
     if (!reset_pps_n) begin
        to_pps_rd_en <= 1'b0;
        from_pps_wr_en <= 1'b0;
        state <= 'b0;
        time_incr <= TIME_INCR_VAL;
        pps_count <= 32'd0;
        accum_incr <= 32'h0;
     end else begin
        case (state)
          AWAIT_NONEMPTY:
            begin
               from_pps_wr_en <= 1'b0;
               if (!to_pps_empty) begin
                  to_pps_rd_en <= 1'b1;
                  state <= FIFO_READ;
               end else
                 state <= AWAIT_NONEMPTY;
            end
          FIFO_READ:
            begin
               /* Data read will be valid in the next state */
               to_pps_rd_en <= 1'b0;
               state <= DO_OP;
            end
          DO_OP:
            begin
               if (is_write) begin
                  /* Just do the write */
                  if (addr == 3'b010)
                    accum_incr <= value_to_write;
                  else if (addr == 3'b011)
                    pps_count <= value_to_write;
                  else if (addr == 3'b100)
                    time_incr <= value_to_write[7:0];
                  state <= AWAIT_NONEMPTY;
               end else begin
                  /* For reads, initiate write to outgoing FIFO */
                  if (~from_pps_full) begin
                     from_pps_wr_en <= 1'b1;
                     state <= AWAIT_NONEMPTY;
                  end else
                    state <= DO_OP;
               end
            end
        endcase
     end
endmodule
