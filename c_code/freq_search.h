#ifndef FREQ_SEARCH_H
#define FREQ_SEARCH_H

#define BOARD_9K 0
#define BOARD_20K 1
#define BOARD_BOTH 2

/* find_freq returns rPLL parameters that yield a target output
 * frequence (as close as possible) give an input frequency of
 * 27 MHz.
 *
 * Inputs:
 *   board - Find parameters for BOARD_9K, BOARD_20K, or that work with both.
 *   target_freq - desired output frequency.
 *
 * Outputs:
 *  The three rPLL parameters.
 */


extern void freq_search(const unsigned int board, const double target_freq,
			double *actual_freq, int *idiv_sel, int *fbdiv_sel,
			int *odiv_sel);

#endif
