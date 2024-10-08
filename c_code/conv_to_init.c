/* Copyright 2024 Grug Huhler.  License SPDX BSD-2-Clause.

   This program converts a binary file into four text files
   that are used to initialize four inferred  8-bit wide
   SRAMs used in the picorv32 project.

   It also produces some Verilog files that help with
   conditional compilation and parameterization in the
   Verilog.

   Usage: conv_to_init sram_addr_width input_file_name

   The memory is then assumed to be 4*2**addr_with bytes
   The output file names are fixed: 

   1. ../src/global_defs.v

      This file contains

         `define BOARD_XXX

      where XXX is the board we are building for.  This enables
      conditional compilation in the Verilog.

   2. ../src/sys_parameters.v

      This file contains the lines

         localparam SRAM_ADDR_WIDTH = sram_addr_width;
         localparam CLK_FREQ = clk_freq;

      and also rPLL parameters.

   3. ../src/mem_initX.ini, where X = 0, 1, 2, 3.
       File mem_init0.ini contains the least significant byte.

   See the Makefile.  It invokes conv_to_init.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freq_search.h"

int main(int argc, char **argv)
{
  char fname[80], *board;
  FILE *fp_in, *fp_param, *fp_out[4];
  int v, i, byte_count = 0;
  int sram_addr_width, mem_bytes, clk_freq, target_clk_freq;
  int idiv_sel, fbdiv_sel, odiv_sel;
  double actual_freq;
  
  if (argc != 5) {
    fprintf(stderr, "Usage: board_type conv_to_init clk_freq sram_addr_width filename\n");
    exit(EXIT_FAILURE);
  }

  board = argv[1];

  target_clk_freq = atoi(argv[2]);
  if (target_clk_freq <= 0) {
    fprintf(stderr, "clk_freq must be posiive\n");
    exit(EXIT_FAILURE);
  }

  /* Get rPLL parameters for clk_freq and supported value for clk_freq */
  freq_search(BOARD_BOTH, target_clk_freq/1000000.0, &actual_freq, &idiv_sel,
	      &fbdiv_sel, &odiv_sel);
  clk_freq = 1000000.0*actual_freq + 0.5;

  if (clk_freq != target_clk_freq) {
    fprintf(stderr, "\nERROR:\n");
    fprintf(stderr, "   Clock frequency %d is not feasible.  Instead try %d\n",
	    target_clk_freq, clk_freq);
    fprintf(stderr, "   BUILD FAILED\n\n");
    exit(EXIT_FAILURE);
  }
    

  sram_addr_width = atoi(argv[3]);
  if (sram_addr_width <= 0) {
    fprintf(stderr, "sram_addr_width must be posiive\n");
    exit(EXIT_FAILURE);
  }

  mem_bytes = 4*(1 << sram_addr_width);
  
  fp_in = fopen(argv[4], "rb");
  if (fp_in == NULL) {
    fprintf(stderr, "Could not open %s\n", argv[3]);
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < 4; i++) {

    sprintf(fname, "../src/mem_init%d.ini", i);
    
    fp_out[i] = fopen(fname, "wb");
    if (fp_out[i] == NULL) {
      fprintf(stderr, "could not open file %s\n", fname);
      exit(EXIT_FAILURE);
    }
  }

  fp_param = fopen("../src/sys_parameters.v", "w");
  if (fp_param == NULL) {
    fprintf(stderr, "could not open file %s\n", "../src/sys_parameters.v");
    exit(EXIT_FAILURE);
  }
  fprintf(fp_param, "localparam IDIV_SEL = %d;\n", idiv_sel);
  fprintf(fp_param, "localparam FBDIV_SEL = %d;\n", fbdiv_sel);
  fprintf(fp_param, "localparam ODIV_SEL = %d;\n", odiv_sel);
  fprintf(fp_param, "localparam SRAM_ADDR_WIDTH = %d;\n", sram_addr_width);
  fprintf(fp_param, "localparam CLK_FREQ = %d;\n", clk_freq);
  fclose(fp_param);

  fp_param = fopen("../src/global_defs.v", "w");
  if (fp_param == NULL) {
    fprintf(stderr, "could not open file %s\n", "../src/global_defs.v");
    exit(EXIT_FAILURE);
  }
  fprintf(fp_param, "`define %s\n", board);
  fclose(fp_param);

  while ((v = fgetc(fp_in)) != EOF) {
    fprintf(fp_out[byte_count % 4], "%02X\n", v);
    byte_count += 1;
  }

  /* Initialize to 16 byte boundary just in case */
  while ((byte_count % 16) != 0) {
    fprintf(fp_out[byte_count % 4], "%02X\n", 0);
    byte_count += 1;
  }

  fclose(fp_in);
  for (i = 0; i < 4; i++) fclose(fp_out[i]);

  if (byte_count > mem_bytes) {
      fprintf(stderr, "ERROR: PROGRAM IS TOO LARGE: %d bytes is\n", byte_count);
      fprintf(stderr, "       greater than %d bytes\n", mem_bytes);
      fprintf(stderr, "And don't forget to leave room for the stack\n");
    return(EXIT_FAILURE);
  }

  printf("NOTE: program occupies %d bytes\n", byte_count);

  return EXIT_SUCCESS;
}
