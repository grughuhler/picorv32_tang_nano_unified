/* Copyright 2024 Grug Huhler.  License SPDX BSD-2-Clause.
*/

#ifndef PPS_TIMER_H
#define PPS_TIMER_H

extern unsigned long long pps_get_timestamp(void);
extern unsigned long long pps_get_eventstamp(void);
extern void pps_set_accum_incr(unsigned int value);
extern void pps_set_pps_count(unsigned int value);
extern void pps_set_time_incr(unsigned int value);

#endif
