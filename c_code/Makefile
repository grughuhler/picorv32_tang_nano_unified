# Copyright 2024 Grug Huhler.  License SPDX BSD-2-Clause.
#
# This makefile builds a test program for a PicoRV32 core
# instantiated on either a Tang Nano 9K or a Tang Nano 20K
# FPGA development board.
#
# Use this Makefile to build the software BEFORE using the Gowin
# IDE to build the Verilog bitfile.
#
# To build enter "make 9k" or "make 20k" after "make clean", especially
# when switching between the Nano 9K and Nano 20K IDE projects.
#
# SRAM_ADDR_WIDTH sets the width of the SRAM's address in the Verilog.
# This implicitly sets the total bytes of SRAM to 4*(1 << SRAM_ADDR_WIDTH)
# which, in turn, sets the starting address of the stack pointer in
# startup.s to the top of the SRAM.
#
# The practical SRAM_ADDR_WIDTH values are
#   11 - giving 8192 bytes of SRAM (using 4 block SRAMS).
#   12 - giving 16384 bytes of SRAM (using 8 block SRAMS).
#   13 - giving 32768 bytes of SRAM  (using 16 block SRAMS).
#   14 - giving 65536 bytes of SRAM  (using 32 block SRAMS, Nano 20K only).
#
# The Gowin GW1NR-LV9 has only 26 block SRAMS so value 14 will fail.
# The GW2AR-LV18 has 46 blocks so 14 is OK for it.
#
# Values less than 11 will work but are pointless on these FPGAs since
# 4 block SRAMs will still be used.
#
# Note that file force_rebuild.v is empty and exists only to cause
# the Gowan IDE to rebuild when this makefile has executed.

# Program conv_to_init creates the Verilog files in ../src.  See the
# comments in conv_to_init.c.


# Change memory size here
SRAM_ADDR_WIDTH = 12

# CLK_FREQ tells both software and Verilog the clock speed.  Change it
# here in this makefile only.  Program conv_to_init finds rPLL parameters
# for the chosen frequency, but the build will fail if that frequency is
# not feasible. In that case, an error message suggests a nearby frequency
# that is feasible.
# Generated file ../src/sys_parameters.v lets the Verilog build use the
# frequency value.  It is passed to software by a -DCLK_FREQ=XXXX during
# the build.

# Change system clock speed here (MHz).
CLK_FREQ = 9000000

ifeq ($(MAKECMDGOALS),9k)
BOARD = BOARD_9K
else
ifeq ($(MAKECMDGOALS),20k)
BOARD = BOARD_20K
else
BOARD = ERROR
endif
endif

COMMONFLAGS = -march=rv32i2p0 -mabi=ilp32
CFLAGS = -mno-save-restore $(COMMONFLAGS) -nostartfiles -nostdlib -static -O1 -D$(BOARD)
CPPFLAGS = -D$(BOARD)
ASFLAGS = --defsym SRAM_ADDR_WIDTH=$(SRAM_ADDR_WIDTH) $(COMMONFLAGS)
LIBS = /usr/lib/gcc/riscv64-unknown-elf/10.2.0/rv32i/ilp32/libgcc.a

CC = riscv64-unknown-elf-gcc
OBJCOPY = riscv64-unknown-elf-objcopy
OBJDUMP = riscv64-unknown-elf-objdump
AS = riscv64-unknown-elf-as

COMMON_CSRCS = main.c countdown_timer.c uart.c leds.c pps_timer.c

NANO9K_CSRCS = uflash.c xorshift32.c
NANO20K_CSRCS = ws2812b.c
ALL_CSRCS = $(COMMON_CSRCS) $(NANO9K_CSRCS) $(NANO20K_CSRCS)

%.o: %.c
	$(CC) -c $(CFLAGS) -DCLK_FREQ=$(CLK_FREQ) $< -o $@

%.d: %.c
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

default:
	echo $(MAKECMDGOALS)
	@echo "Use make 9k or make 20k"
	@echo "The Verilog build will use files from whichever"
	@echo "was build most recently"

.PHONY: 9k
9k: prog_9k.hex

20k: prog_20k.hex

ifneq ($(MAKECMDGOALS),)
ifneq ($(MAKECMDGOALS),clean)
ifeq ($(MAKECMDGOALS),9k)
include $(COMMON_CSRCS:.c=.d) $(NANO9K_CSRCS:.c=.d)
endif
ifeq ($(MAKECMDGOALS),20k)
include $(COMMON_CSRCS:.c=.d) $(NANO20K_CSRCS:.c=.d)
endif
endif
endif

conv_to_init: conv_to_init.c freq_search.c freq_search.h
	gcc -o conv_to_init conv_to_init.c freq_search.c -lm

startup.o: startup.S
	$(CC) $(CFLAGS) -DSRAM_ADDR_WIDTH=$(SRAM_ADDR_WIDTH) -c startup.S

prog_9k.elf: startup.o $(COMMON_CSRCS:.c=.o) $(NANO9K_CSRCS:.c=.o)
	$(CC) $(CFLAGS) -Tlink_cmd.ld -o prog_9k.elf startup.o \
              $(COMMON_CSRCS:.c=.o) $(NANO9K_CSRCS:.c=.o) $(LIBS)

prog_9k.bin: prog_9k.elf conv_to_init 
	$(OBJCOPY) prog_9k.elf -O binary prog_9k.bin
	rm -f ../src/mem_init[0-3].ini ../src/sys_parameters.v
	./conv_to_init $(BOARD) $(CLK_FREQ) $(SRAM_ADDR_WIDTH) prog_9k.bin
	touch ../src/force_rebuild.v

prog_9k.hex: prog_9k.bin conv_to_init
	od -v -Ax -t x4 prog_9k.bin > prog_9k.hex

prog_20k.elf: startup.o $(COMMON_CSRCS:.c=.o) $(NANO20K_CSRCS:.c=.o)
	$(CC) $(CFLAGS) -Tlink_cmd.ld -o prog_20k.elf startup.o \
              $(COMMON_CSRCS:.c=.o) $(NANO20K_CSRCS:.c=.o) $(LIBS)

prog_20k.bin: prog_20k.elf conv_to_init 
	$(OBJCOPY) prog_20k.elf -O binary prog_20k.bin
	rm -f ../src/mem_init[0-3].ini ../src/sys_parameters.v
	./conv_to_init $(BOARD) $(CLK_FREQ) $(SRAM_ADDR_WIDTH) prog_20k.bin
	touch ../src/force_rebuild.v

prog_20k.hex: prog_20k.bin conv_to_init
	od -v -Ax -t x4 prog_20k.bin > prog_20k.hex

clean:
	rm -f prog_9k.elf prog_9k.hex prog_9k.bin prog_20k.elf prog_20k.hex prog_20k.bin \
	      startup.o $(ALL_CSRCS:.c=.d) $(ALL_CSRCS:.c=.o) conv_to_init
