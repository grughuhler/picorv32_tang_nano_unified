/* Copyright 2024 Grug Huhler.  License SPDX BSD-2-Clause.

Top level module of simple SoC based on picorv32

See top level README for capabilities.

The picorv32 core has a very simple memory interface.
See https://github.com/YosysHQ/picorv32

In this SoC, slave (target) device has signals:

   * SLAVE_sel - this is asserted when mem_valid == 1 and mem_addr targets the slave.
     It "tells" the slave that it is active.  It must accept a write for provide data
     for a read.
   * SLAVE_ready - this is asserted by the slave when it is done with the transaction.
     Core signal mem_ready is the OR of all of the SLAVE_ready signals.
   * Core mem_addr, mem_wdata, and mem_wstrb can be passed to all slaves directly.
     The latter is a byte lane enable for writes.
   * Each slave drives SLAVE_data_o.  The core's mem_rdata is formed by selecting the
     correct SLAVE_data_o based on SLAVE_sel.
*/

`include "global_defs.v"

// Define this for logic analyer connections and enable picorv32_la.cst.
//`define USE_LA

module top (
            input wire        clk_in,  // Must be 27 MHz
            input wire        reset_button,
`ifdef BOARD_20K
            output wire       ws2812b_din,
`endif
            input wire        uart_rx,
            output wire       uart_tx,
`ifdef USE_LA
            output wire       clk_out,
            output wire       mem_instr, 
            output wire       mem_valid,
            output wire       mem_ready,
            output wire       b25,
            output wire       b24,
            output wire       b17,
            output wire       b16,
            output wire       b09,
            output wire       b08,
            output wire       b01,
            output wire       b00,
            output wire [3:0] mem_wstrb,
`endif
            output wire [5:0] leds
            );

   parameter BARREL_SHIFTER = 0;
   parameter ENABLE_MUL = 0;
   parameter ENABLE_DIV = 0;
   parameter ENABLE_FAST_MUL = 0;
   parameter ENABLE_COMPRESSED = 0;
   parameter ENABLE_IRQ_QREGS = 0;

   parameter          MEMBYTES = 4*(1 << SRAM_ADDR_WIDTH); 
   parameter [31:0] STACKADDR = (MEMBYTES);         // Grows down.  Software should set it.
   parameter [31:0] PROGADDR_RESET = 32'h0000_0000;
   parameter [31:0] PROGADDR_IRQ = 32'h0000_0000;

   // This include gets SRAM_ADDR_WIDTH from software build process
   `include "sys_parameters.v"

   wire                       clk;
   wire                       reset_n; 
   wire                       mem_valid;
   wire                       mem_instr;
   wire [31:0]                mem_addr;
   wire [31:0]                mem_wdata;
   wire [31:0]                mem_rdata;
   wire [3:0]                 mem_wstrb;
   wire                       mem_ready;
   wire                       mem_inst;
   wire                       leds_sel;
   wire                       leds_ready;
   wire [31:0]                leds_data_o;
   wire                       sram_sel;
   wire                       sram_ready;
   wire [31:0]                sram_data_o;
   wire                       cdt_sel;
   wire                       cdt_ready;
   wire [31:0]                cdt_data_o;
   wire                       uart_sel;
   wire [31:0]                uart_data_o;
   wire                       uart_ready;
`ifdef BOARD_9K
   wire                       uflash_sel;
   wire [31:0]                uflash_data_o;
   wire                       uflash_ready;
`endif
`ifdef BOARD_20K
   wire                       ws2812b_sel;
   wire                       ws2812b_ready;
`endif
   // default_sel causes a response when nothing else does
   wire                       default_sel;
   reg                        default_ready;

`ifdef USE_LA
   // Assigns for external logic analyzer connction
   assign clk_out = clk;
   assign b25 = mem_rdata[25];
   assign b24 = mem_rdata[24];
   assign b17 = mem_rdata[17];
   assign b16 = mem_rdata[16];
   assign b09 = mem_rdata[9];
   assign b08 = mem_rdata[8];
   assign b01 = mem_rdata[1];
   assign b00 = mem_rdata[0];
`endif

   // Set clk's frequency to CLK_FREQ from c_code/Makefile
   Gowin_rPLL #(.IDIV_SEL(IDIV_SEL), .FBDIV_SEL(FBDIV_SEL), .ODIV_SEL(ODIV_SEL)) (
      .clkout(clk),
      .clkin(clk_in)
   );

   // Establish memory map for all slaves:
   //    SRAM 00000000 - 0001ffff
   //  uflash 00020000 - 00032fff  (9K only)
   //    LED  80000000
   //    UART 80000008 - 8000000f
   //    CDT  80000010 - 80000013
   // ws2812b 80000020 - 80000023  (20k only)

   assign sram_sel = mem_valid && (mem_addr < MEMBYTES);
`ifdef BOARD_9K
   assign uflash_sel = mem_valid && ((mem_addr >= 32'h2_0000) && (mem_addr < 32'h3_3000));
`endif
   assign leds_sel = mem_valid && (mem_addr == 32'h80000000);
   assign uart_sel = mem_valid && ((mem_addr & 32'hfffffff8) == 32'h80000008);
   assign cdt_sel = mem_valid && (mem_addr == 32'h80000010);
`ifdef BOARD_20K
   assign ws2812b_sel = mem_valid && (mem_addr == 32'h80000020);
`endif

   // Core can proceed based on which slave was targetted and is now ready.
   assign mem_ready = mem_valid &
      (sram_ready | leds_ready | uart_ready | cdt_ready | 
`ifdef BOARD_9K
        uflash_ready | 
`endif
`ifdef BOARD_20K
        ws2812b_ready |
`endif
        default_ready);

   // Select which slave's output data is to be fed to core.
   assign mem_rdata = sram_sel    ? sram_data_o :
                      leds_sel    ? leds_data_o :
                      uart_sel    ? uart_data_o :
`ifdef BOARD_9K
                      uflash_sel  ? uflash_data_o :
`endif
                      cdt_sel     ? cdt_data_o  : 32'hdeadbeef;

   assign leds = ~leds_data_o[5:0]; // Connect to the LEDs off the FPGA

   // The default devices responds to accesses to addresses that don't
   // map to any device.

   assign default_sel = mem_valid & !sram_sel & !leds_sel & ~uart_sel &
`ifdef BOARD_9K
                       ~uflash_sel &
`endif
`ifdef BOARD_20K
                       ~ws2812b_sel &
`endif
                       ~cdt_sel;

   always @(posedge clk or negedge reset_n)
     if (!reset_n)
       default_ready <= 1'b0;
     else
        if (default_sel)
           default_ready <= 1'b1;
        else
           default_ready <= 1'b0;

   reset_control reset_controller
     (
      .clk(clk),
      .reset_button(reset_button),
      .reset_n(reset_n)
      );

   uart_wrap uart
     (
      .clk(clk),
      .reset_n(reset_n),
      .uart_tx(uart_tx),
      .uart_rx(uart_rx),
      .uart_sel(uart_sel),
      .addr(mem_addr[3:0]),
      .uart_wstrb(mem_wstrb),
      .uart_di(mem_wdata),
      .uart_do(uart_data_o),
      .uart_ready(uart_ready)
      );

   countdown_timer cdt
     (
      .clk(clk),
      .reset_n(reset_n),
      .cdt_sel(cdt_sel),
      .cdt_data_i(mem_wdata),
      .we(mem_wstrb),
      .cdt_ready(cdt_ready),
      .cdt_data_o(cdt_data_o)
      );

`ifdef BOARD_20K
   /* ws2812b_tgt is 32b write only */
   ws2812b_tgt #(.CLK_FREQ(CLK_FREQ)) ws2812b_led
     (
      .clk(clk),
      .reset_n(reset_n),
      .ws2812b_sel(ws2812b_sel),
      .we(&mem_wstrb),
      .wdata({mem_wdata[15:8], mem_wdata[23:16], mem_wdata[7:0]}),
      .ws2812b_ready(ws2812b_ready),
      .to_din(ws2812b_din)
      );
`endif

`ifdef BOARD_9K
   uflash #(.CLK_FREQ(CLK_FREQ)) uflash0 (
     .reset_n(reset_n),
     .clk(clk),
     .sel(uflash_sel),
     .wstrb(mem_wstrb),
     .addr(mem_addr[16:2]), // word address, 9-bits row, 6 bits col
     .data_i(mem_wdata),
     .ready(uflash_ready),
     .data_o(uflash_data_o)
   );
`endif

   sram #(.SRAM_ADDR_WIDTH(SRAM_ADDR_WIDTH)) memory
     (
      .clk(clk),
      .reset_n(reset_n),
      .sram_sel(sram_sel),
      .wstrb(mem_wstrb),
      .addr(mem_addr[SRAM_ADDR_WIDTH + 1:0]),
      .sram_data_i(mem_wdata),
      .sram_ready(sram_ready),
      .sram_data_o(sram_data_o)
      );
   
   tang_leds soc_leds
     (
      .clk(clk),
      .reset_n(reset_n),
      .leds_sel(leds_sel),
      .leds_data_i(mem_wdata[5:0]),
      .we(mem_wstrb[0]),
      .leds_ready(leds_ready),
      .leds_data_o(leds_data_o)
      );

   picorv32
     #(
       .STACKADDR(STACKADDR),
       .PROGADDR_RESET(PROGADDR_RESET),
       .PROGADDR_IRQ(PROGADDR_IRQ),
       .BARREL_SHIFTER(BARREL_SHIFTER),
       .COMPRESSED_ISA(ENABLE_COMPRESSED),
       .ENABLE_MUL(ENABLE_MUL),
       .ENABLE_DIV(ENABLE_DIV),
       .ENABLE_FAST_MUL(ENABLE_FAST_MUL),
       .ENABLE_IRQ(1),
       .ENABLE_IRQ_QREGS(ENABLE_IRQ_QREGS)
       ) cpu
       (
        .clk         (clk),
        .resetn      (reset_n),
        .mem_valid   (mem_valid),
        .mem_instr   (mem_instr),
        .mem_ready   (mem_ready),
        .mem_addr    (mem_addr),
        .mem_wdata   (mem_wdata),
        .mem_wstrb   (mem_wstrb),
        .mem_rdata   (mem_rdata),
        .irq         ('b0)
        );

endmodule // top
