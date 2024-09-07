/* Copyright 2024 Grug Huhler.  License SPDX BSD-2-Clause.
*/

#include "pps_timer.h"
#include "uart.h"

#define PPS_TIMESTAMP_LO ((volatile unsigned int *) 0x80000040)
#define PPS_TIMESTAMP_HI ((volatile unsigned int *) 0x80000044)
#define PPS_ACCUM_INCR ((volatile unsigned int *) 0x80000048)
#define PPS_PPS_COUNT ((volatile unsigned int *) 0x8000004c)
#define PPS_TIME_INCR ((volatile unsigned int *) 0x80000050)
#define PPS_EVENTSTAMP_LO ((volatile unsigned int *) 0x80000054)
#define PPS_EVENTSTAMP_HI  ((volatile unsigned int *) 0x80000058)

static unsigned long long pps_get64(volatile unsigned int *addr)
{
  unsigned int th1, th2, tl;
  unsigned long long ts;

  /* addr is low word; addr+1 is high */

  do {
    th1 = *(addr + 1);
    tl = *addr;
    th2 = *(addr + 1);
  } while (th1 != th2);

  ts = ((unsigned long long) th1 << 32) | tl;

  return ts;
}

unsigned long long pps_get_timestamp(void)
{
  return pps_get64(PPS_TIMESTAMP_LO);
}

unsigned long long pps_get_eventstamp(void)
{
  return pps_get64(PPS_EVENTSTAMP_LO);
}

void pps_set_accum_incr(unsigned int value)
{
  *PPS_ACCUM_INCR = value;
}

void pps_set_pps_count(unsigned int value)
{
  *PPS_PPS_COUNT = value;
}
