/* Copyright 2024 Grug Huhler.  License SPDX BSD-2-Clause.

   This is a test program for the simple SoC based on the PicoRV32 core.
   It can be used on either the Tang Nano 9K or the Tang Nano 20K, but
   only the former contains user flash.
*/

#include "leds.h"
#include "uart.h"
#include "countdown_timer.h"
#include "readtime.h"
#include "pps_timer.h"
#if defined(BOARD_9K)
#include "uflash.h"
#include "xorshift32.h"
#elif defined(BOARD_20K)
#include "ws2812b.h"
#else
#error "BOARD NOT PROPERLY DEFINED.  Try make clean; make 9k|10k"
#endif

extern unsigned int timer_instr(unsigned int val);   /* from startup.S */
extern unsigned int maskirq_instr(unsigned int val); /* from startup.S */

/* These defines are associated with the pps_timer.
 *    ACCUM_INCR_NOMINAL = 0xd5555555 is correct for a 120 MHz clk_pps
 *         and a 100 MHz Fcount clock, i.e. a ratio of 1.2.
 *    PPS_COUNT_VALUE = 0x5f5e100 gives a 1 second PPS period also
 *         assuming a 100 MHz Fcount clock.  0x5f5e100 is 100e6.
 *    PPS_TARGET_HZ = 1000000000 is based on the timestamps being in
 *         nanosecond.  This matches with time_incr value of 10 hardcoded
 *         in pps.v.
 */

#define ACCUM_INCR_NOMINAL 0xd5555555
#define PPS_TARGET_HZ 1000000000

#if defined(BOARD_20K)
#define PPS_COUNT_VALUE 100000
#elif defined(BOARD_9K)
#define PPS_COUNT_VALUE 50000
#endif

unsigned long long last_ts;
unsigned int timer_irq_count;
unsigned int illegal_irq_count;
unsigned int buserr_irq_count;
unsigned int irq3_count;
unsigned int pps_irq_count; // IRQ 4
unsigned int event_irq_count; // IRQ 5
unsigned char inhibit_pps_printing;
unsigned char inhibit_pps_updating;
unsigned int accum_incr;


#define MEMSIZE 512
unsigned int mem[MEMSIZE];
unsigned int test_vals[] = {0, 0xffffffff, 0xaaaaaaaa, 0x55555555, 0xdeadbeef};

/* A simple memory test.  Delete this and also array mem
   above to free much of the SRAM for other things
*/

int mem_test (void)
{
  int i, test, errors;
  unsigned int val, val_read;

  errors = 0;
  for (test = 0; test < sizeof(test_vals)/sizeof(test_vals[0]); test++) {

    for (i = 0; i < MEMSIZE; i++) mem[i] = test_vals[test];

    for (i = 0; i < MEMSIZE; i++) {
      val_read = mem[i];
      if (val_read != test_vals[test]) errors += 1;
    }
  }

  for (i = 0; i < MEMSIZE; i++) mem[i] = i + (i << 17);

  for (i = 0; i < MEMSIZE; i++) {
    val_read = mem[i];
    if (val_read != i + (i << 17)) errors += 1;
  }

  return(errors);
}


void endian_test(void)
{
  volatile unsigned int test_loc = 0;
  volatile unsigned int *addr = &test_loc;
  volatile unsigned char *cp0, *cp3;
  char byte0, byte3;
  unsigned int i, ok;

  cp0 = (volatile unsigned char *) addr;
  cp3 = cp0 + 3;
  *addr = 0x44332211;
  byte0 = *cp0;
  byte3 = *cp3;
  *cp3 = 0xab;
  i = *addr;

  ok = (byte0 == 0x11) && (byte3 == 0x44) && (i == 0xab332211);
  uart_puts("\r\nEndian test: at ");
  uart_print_hex((unsigned int) addr);
  uart_puts(", byte0: ");
  uart_print_hex((unsigned int) byte0);
  uart_puts(", byte3: ");
  uart_print_hex((unsigned int) byte3);
  uart_puts(",\r\n     word: ");
  uart_print_hex(i);
  if (ok)
    uart_puts(" [PASSED]\r\n");
  else
    uart_puts(" [FAILED]\r\n");
}


/* la_functions are useful for looking at the bus using a logic
   analyzer.
*/

void la_wtest(void)
{
  unsigned int v;
  volatile unsigned int *ip = (volatile unsigned int *) &v;
  volatile unsigned short *sp = (volatile unsigned short *) &v;
  volatile unsigned char *cp = (volatile unsigned char *) &v;

  *ip = 0x03020100;  // addr 0x00

  *sp = 0x0302;      // addr 0x00
  *(sp+1) = 0x0100;  // addr 0x02

  *cp = 0x03;        // addr 0x00
  *(cp+1) = 0x02;    // addr 0x01
  *(cp+2) = 0x01;    // addr 0x02
  *(cp+3) = 0x00;    // addr 0x03
}


void la_rtest(void)
{
  unsigned int v;
  volatile unsigned int *ip = (volatile unsigned int *) &v;
  volatile unsigned short *sp = (volatile unsigned short *) &v;
  volatile unsigned char *cp = (volatile unsigned char *) &v;

  *ip = 0x03020100;  // addr 0x00

  *ip;     // addr 0x00

  *sp;     // addr 0x00
  *(sp+1); // addr 0x02

  *cp;     // addr 0x00
  *(cp+1); // addr 0x01
  *(cp+2); // addr 0x02
  *(cp+3); // addr 0x03
}


void countdown_timer_test(void)
{
  unsigned int val;
  unsigned int test_errors = 0;
  
  // If register is little-endian, write to 0x80000013 should set
  // the MSB,  Does it?

  cdt_wbyte3(0xff);
  val = cdt_read();
  if ((val == 0xff000000) || (val < 0xfe000000)) test_errors = 1;

  // Write zero to most significant half-word.
  cdt_whalf2(0);
  val = cdt_read();
  if (val > 0xffff) test_errors |= 2;

  uart_puts("Countdown timer test ");
  if (test_errors) {
    uart_puts("FAILED, mask = ");
    uart_print_hex(test_errors);
    uart_puts("\r\n");
  } else {
    uart_puts("PASSED\r\n");
  }
}

void cycle_delay(unsigned int cycles)
{
  uart_puts("delay ");
  uart_print_hex(cycles);
  uart_puts(" cycles\r\n");
  cdt_delay(cycles);
  uart_puts("done\r\n");
}

void read_led(void)
{
  unsigned char v;
  
  v = get_leds();
  uart_puts("LED = ");
  uart_print_hex(v);
  uart_puts("\r\n");
}

void incr_led(void)
{
  unsigned char v;
  
  v = get_leds();
  set_leds(v+1);
}

void set_led(unsigned int value)
{
  set_leds(value);
}

void memory_test(void)
{
  if (mem_test())
    uart_puts("memory test FAILED.\r\n");
  else
    uart_puts("memory test PASSED.\r\n");
}

void read_clock(void)
{
  uart_puts("time is ");
  uart_print_hex(readtime());
  uart_puts("\r\n");
}

void read_clock_ll(void)
{
  time_ll_t t;

  t = readtime_ll();
  uart_puts("time is ");
  uart_print_hex(t.s.time_high);
  uart_putchar(':');
  uart_print_hex(t.s.time_low);
  uart_puts("\r\n");
}

#if defined(BOARD_9K)
void erase_all_flash(void)
{
  volatile unsigned char *p = (volatile unsigned char *) 0x20000;
  time_ll_t start, end;
  unsigned int diff;
  int i;

  start = readtime_ll();
  for (i = 0; i < 38; i++) {
    *p = 0;
    p += 2048;
    uart_print_hex(i);
    uart_puts("\r\n");
  }
  end = readtime_ll();

  diff = end.time_val - start.time_val;
  uart_puts("done in ");
  uart_print_hex(diff);
  uart_puts(" cycles\r\n");
}

void write_all_flash(void)
{
  volatile unsigned int *p = (volatile unsigned int *) 0x20000;
  unsigned int state = 1;
  time_ll_t start, end;
  unsigned int diff;
  int i;

  start = readtime_ll();
  for (i = 0; i < 19456; i++) {
    *p++ = xorshift32(&state);
  }
  end = readtime_ll();

  diff = end.time_val - start.time_val;
  uart_puts("done in ");
  uart_print_hex(diff);
  uart_puts(" cycles\r\n");
}

void check_all_flash(void)
{
  volatile unsigned int *p = (volatile unsigned int *) 0x20000;
  unsigned int state = 1;
  int i, j, errors = 0;

  for (i = 0; i < 304; i++) {
    uart_print_hex(0x20000 + (i << 8)); /* 0x20000 + 4*64*i */
    uart_putchar(' ');
    for (j = 0; j < 64; j++) {
      if (*p++ != xorshift32(&state)) {
	errors += 1;
	uart_putchar('x');
      } else {
	uart_putchar('.');
      }
    }
    uart_puts("\r\n");
  }
  
  uart_puts("errors = ");
  uart_print_hex(errors);
  uart_puts("\r\n");
}
#endif


void help(void)
{
  uart_puts("ct            : test countdown timer\r\n");
  uart_puts("dc cycles     : delay for cycles\r\n");
#if defined(BOARD_9K)
  uart_puts("cf            : check that uflash values match wf\r\n");
  uart_puts("ef addr       : erase page of uflash\r\n");
  uart_puts("af            : erase all uflash\r\n");
  uart_puts("wf            : write all uflash\r\n");
#endif
  uart_puts("ix            : toggle pps printing\r\n");
  uart_puts("iu            : toggle pps updating\r\n");
  uart_puts("ia            : set pps accum_incr\r\n");
  uart_puts("ip            : set pps pps_count\r\n");
  uart_puts("it            : read pps timestamp\r\n");
  uart_puts("et            : endian test\r\n");
  uart_puts("rl            : read LEDs\r\n");
  uart_puts("il            : increment LEDs\r\n");
  uart_puts("sl value      : set LEDs to value\r\n");
  uart_puts("mt            : memory test\r\n");
  uart_puts("rc            : read clock low\r\n");
  uart_puts("rd            : read clock hi:low\r\n");
  uart_puts("he            : print help\r\n");
  uart_puts("bi            : cause bus error\r\n");
  uart_puts("ii            : cause illegal instruction\r\n");
  uart_puts("mi            : set IRQ mask\r\n");
  uart_puts("pi            : print IRQ count\r\n");
  uart_puts("ti            : set timer to val\r\n");
#if defined(BOARD_20K)
  uart_puts("sn            : set ws2812b LED\r\n");
#endif
  uart_puts("rb addr       : read byte\r\n");
  uart_puts("rh addr       : read half word\r\n");
  uart_puts("rw addr       : read word\r\n");
  uart_puts("wb addr value : write byte\r\n");
  uart_puts("wh addr value : write half word\r\n");
  uart_puts("ww addr value : write word\r\n");
  uart_puts("   all numbers are hex\r\n");
}

void read_byte(unsigned int addr)
{
  volatile unsigned char *p = (volatile unsigned char *) addr;

  uart_print_hex(*p);
  uart_puts("\r\n");
}

void read_half(unsigned int addr)
{
  volatile unsigned short *p = (volatile unsigned short *) addr;

  uart_print_hex(*p);
  uart_puts("\r\n");
}

void read_word(unsigned int addr)
{
  volatile unsigned int *p = (volatile unsigned int *) addr;

  uart_print_hex(*p);
  uart_puts("\r\n");
}

void write_byte(unsigned int addr, unsigned int value)
{
  volatile unsigned char *p = (volatile unsigned char *) addr;

  *p = value;
}

void write_half(unsigned int addr, unsigned int value)
{
  volatile unsigned short *p = (volatile unsigned short *) addr;

  *p = value;
}

void write_word(unsigned int addr, unsigned int value)
{
  volatile unsigned int *p = (volatile unsigned int *) addr;

  *p = value;
}

#if defined(BOARD_20K)
void change_ws2812b(unsigned int value)
{
  uart_puts("setting neopixel to ");
  uart_print_hex(value);
  uart_puts("\r\n");
  set_ws2812b(value);
}
#endif

/* accum_incr
 *
 * Return new value for accum_incr based on
 *  - The current accum_incr
 *  - The target period (i.e. expected) signal period.
 *  - The observed period which if not equal to the target represents
 *    error in the time_ctr counting rate.
 */

unsigned int adjust(unsigned long long accum_incr,
		    unsigned long long target_period,
		    unsigned long long observed_period)
{
  unsigned int new_accum_incr;

  /* The /4 is to slow the changes to accum_incr to avoid chasing noise */

  if (observed_period >= target_period)
    observed_period = target_period + (observed_period - target_period)/4;
  else
    observed_period = target_period - (target_period - observed_period)/4;
  
  new_accum_incr = (accum_incr*target_period)/observed_period;

  return new_accum_incr;
}


void process_pps()
{
  unsigned long long ts;
  unsigned long long diff;
  unsigned int observed_period, new_accum_incr, diff_ideal;
  unsigned char diff_ideal_sign, do_update;

  ts = pps_get_timestamp();

  /* Some checks are needed to avoid convergence problems.  One can
   * get junk readings from the GPS when it is aquiring a fix.  Also,
   * it can miss pulses if reception is not good.
   * Also, trying to go faster than clk_pps results in overflow in
   * The adjust function.  It's a good idea to process only observed periods
   * that are plausible.
   */

  if (ts <= last_ts) {
    /* Reject negative period */
    do_update = 0;
  } else {
    diff = ts - last_ts;
    /* reject periods that are not plausible */
    if ((diff < 990000000LL) || (diff >= 1100000000LL)) {
      do_update = 0;
    } else {
      do_update = 1;
    }
  }

  last_ts = ts;
  observed_period = diff;
  
  if (!inhibit_pps_updating && do_update) {
    accum_incr = adjust(accum_incr, PPS_TARGET_HZ, observed_period);
    pps_set_accum_incr(accum_incr);
  }

  if (!inhibit_pps_printing) {

    if (observed_period >= PPS_TARGET_HZ) {
      diff_ideal = observed_period - PPS_TARGET_HZ;
      diff_ideal_sign = 0;
    } else {
      diff_ideal = PPS_TARGET_HZ - observed_period;
      diff_ideal_sign = 1;
    }

    uart_puts("Obs Period: ");
    uart_print_hex(observed_period);
    uart_puts(", err: ");
    if (diff_ideal_sign)
      uart_puts("-");
    else
      uart_puts(" ");
    uart_print_hex(diff_ideal);
    uart_puts(", new accum_incr: ");
    uart_print_hex(accum_incr);
    if (do_update == 0)
      uart_puts(" no update\r\n");
    else
      uart_puts("\r\n");
  }
}

void toggle_pps_printing(void)
{
  inhibit_pps_printing = 1 - inhibit_pps_printing;
}

void toggle_pps_updating(void)
{
  inhibit_pps_updating = 1 - inhibit_pps_updating;
}

void pps_read_timestamp(void)
{
  long long ts;
  unsigned int *p = (unsigned int *) &ts;

  ts = pps_get_timestamp();
  uart_puts("timestamp: ");
  uart_print_hex(*(p+1));
  uart_print_hex(*p);
  uart_puts("\r\n");
}

unsigned int *irq(unsigned int *regs, unsigned int irqs)
{
  if ((irqs & 1) != 0) {
    timer_irq_count++;
  }

  if ((irqs & 2) != 0) {
    illegal_irq_count++;
  }

  if ((irqs & 4) != 0) {
    buserr_irq_count++;
  }

  if ((irqs & 8) != 0) {
    irq3_count++;
  }

  if ((irqs & 16) != 0) {
    pps_irq_count++;
    process_pps();
  }

  if ((irqs & 32) != 0) {
    event_irq_count++;
  }

  return regs;
}

void print_irq_counts(void)
{
  uart_puts("timer: ");
  uart_print_hex(timer_irq_count);
  uart_puts("\r\nillegal: ");
  uart_print_hex(illegal_irq_count);
  uart_puts("\r\nbus error: ");
  uart_print_hex(buserr_irq_count);
  uart_puts("\r\nIRQ3: ");
  uart_print_hex(irq3_count);
  uart_puts("\r\nPPS: ");
  uart_print_hex(pps_irq_count);
  uart_puts("\r\nevent: ");
  uart_print_hex(event_irq_count);
  uart_puts("\r\n");
}

void do_timer_instr(unsigned int val)
{
  unsigned int old_val;

  old_val = timer_instr(val);
  uart_puts("old value was ");
  uart_print_hex(old_val);
  uart_puts("\r\n");
}

void do_maskirq_instr(unsigned int val)
{
  unsigned int old_val;

  old_val = maskirq_instr(val);
  uart_puts("old value was ");
  uart_print_hex(old_val);
  uart_puts("\r\n");
}

void illegal(void)
{
  asm volatile ("unimp"); /* Illegal instruction */
}

void buserr(void)
{
  int x, v;
  int *p = &x;

  /* If the compiler detects a misaligned access, it breaks it into a
     sequence of smaller aligned accesses so I use an asm to force a
     misaligned short read */
  asm volatile ("lh %[rd], 1(%[rs])" : [rd] "=r" (v) : [rs] "r" (p));
}


/* struct command lists available commands.  See function help.
 * Each function takes one or two arguments.  The table below
 * contains a pointer to the function to call for each command.
 */

struct command {
  char *cmd_string;
  int num_args;
  union {
    void (*func0)(void);
    void (*func1)(unsigned int val);
    void (*func2)(unsigned int val1, unsigned int val2);
  } u;
} commands[] = {
  {"ct", 0, .u.func0=countdown_timer_test},
  {"dc", 1, .u.func1=cycle_delay}, // cycles
#if defined(BOARD_9K)
  {"ef", 1, .u.func1=erase_page_uflash}, // addr
  {"cf", 0, .u.func0=check_all_flash},
  {"af", 0, .u.func0=erase_all_flash},
  {"wf", 0, .u.func0=write_all_flash},
#endif
  {"ix", 0, .u.func0=toggle_pps_printing},
  {"iu", 0, .u.func0=toggle_pps_updating},
  {"ia", 1, .u.func1=pps_set_accum_incr},
  {"ip", 1, .u.func1=pps_set_pps_count},
  {"it", 0, .u.func0=pps_read_timestamp},
  {"et", 0, .u.func0=endian_test},
  {"rl", 0, .u.func0=read_led},
  {"il", 0, .u.func0=incr_led},
  {"sl", 1, .u.func1=set_led},   // val
  {"mt", 0, .u.func0=memory_test},
  {"rc", 0, .u.func0=read_clock},
  {"rd", 0, .u.func0=read_clock_ll},
  {"he", 0, .u.func0=help},
  {"bi", 0, .u.func0=buserr},
  {"ii", 0, .u.func0=illegal},
  {"mi", 1, .u.func1=do_maskirq_instr},
  {"pi", 0, .u.func0=print_irq_counts},
  {"ti", 1, .u.func1=do_timer_instr},
#if defined(BOARD_20K)
  {"sn", 1, .u.func1=change_ws2812b},
#endif
  {"rb", 1, .u.func1=read_byte}, // addr
  {"rh", 1, .u.func1=read_half}, // addr
  {"rw", 1, .u.func1=read_word}, // addr
  {"wb", 2, .u.func2=write_byte}, // addr, val
  {"wh", 2, .u.func2=write_half}, // addr, val
  {"ww", 2, .u.func2=write_word} // addr, val
};


void eat_spaces(char **buf, unsigned int *len)
{
  while (len > 0) {
    if (**buf == ' ') {
      *buf += 1;
      *len -= 1;
    } else
      break;
  }
}

/* Returns 1 if a number found, else 0.  Number is in *v */

unsigned int get_hex(char **buf, unsigned int *len, unsigned int *v)
{
  int valid = 0;
  int keep_going;
  char ch;

  keep_going = 1;

  *v = 0;
  while (keep_going && (*len > 0)) {

    ch = **buf;
    *buf += 1;
    *len -= 1;

    if ((ch >= '0') && (ch <= '9')) {
      *v = 16*(*v) + (ch - '0');
      valid = 1;
    } else if ((ch >= 'a') && (ch <= 'f')) {
      *v = 16*(*v) + (ch - 'a' + 10);
      valid = 1;
    } else if ((ch >= 'A') && (ch <= 'F')) {
      *v = 16*(*v) + (ch - 'A' + 10);
      valid = 1;
    } else {
      keep_going = 0;
    }
  }

  return valid;
}


void parse(char *buf, unsigned int len)
{
  int i, cmd_not_ok;
  unsigned int val1, val2;

  cmd_not_ok = 1;
  eat_spaces(&buf, &len);
  if (len < 2) goto err;

  for (i = 0; i < sizeof(commands)/sizeof(commands[0]); i++)
    if ((buf[0] == commands[i].cmd_string[0]) && (buf[1] == commands[i].cmd_string[1])) {
      buf += 2;
      len -= 2;
      switch (commands[i].num_args) {
      case 0:
	commands[i].u.func0();
	cmd_not_ok = 0;
	break;
      case 1:
	eat_spaces(&buf, &len);
	if (get_hex(&buf, &len, &val1)) {
	  commands[i].u.func1(val1);
	  cmd_not_ok = 0;
	}
	break;
      case 2:
	eat_spaces(&buf, &len);
	if (get_hex(&buf, &len, &val1)) {
	  eat_spaces(&buf, &len);
	  if (get_hex(&buf, &len, &val2)) {
	    commands[i].u.func2(val1, val2);
	    cmd_not_ok = 0;
	  }
	}
	break;
      default:
	break;
      }
      break;
    }

 err:
  if (cmd_not_ok)
    uart_puts("illegal command, he for help\r\n");
}

#define BUFLEN 64

int main()
{
  char buf[BUFLEN];
  unsigned int len;
  
  set_leds(6);

  accum_incr = ACCUM_INCR_NOMINAL;
  pps_set_accum_incr(accum_incr);
  pps_set_pps_count(PPS_COUNT_VALUE);
  last_ts = 0LL;
  event_irq_count = 0;
  pps_irq_count = 0;
  inhibit_pps_printing = 0;
  inhibit_pps_updating = 0;
  
  timer_irq_count = 0;
  illegal_irq_count = 0;
  buserr_irq_count = 0;
  irq3_count = 0;
  
  la_wtest(); /* Could delete these.  They produce transactions that are */
  la_rtest(); /* nice to view on a logic analyzer. */

  uart_set_div(CLK_FREQ/115200.0 + 0.5);
  
  uart_puts("\r\nStarting, CLK_FREQ: 0x");
  uart_print_hex(CLK_FREQ);
  uart_puts("\r\nenter he for a list of commands\r\n");

  while (1) {
    len = uart_gets(buf, BUFLEN);
    if (len != 0) parse(buf, len);
  }

  return 0;
}
