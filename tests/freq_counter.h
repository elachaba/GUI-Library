//
//  freq_counter.h
//  projet_c_LLIG
//
//  Created by François Bérard on 26/3/15.
//  Copyright (c) 2015 LIG-IIHM. All rights reserved.
//

#ifndef EI_FREQ_COUNTER
#define EI_FREQ_COUNTER


/**
 * @brief	A frequency counter. Store information and statistics about a counter
 *		used to measure the frequency of calls to \ref frequency_tick.
 *		***must*** be initialized by a call to \ref frequency_init before being
 *		passed to \ref frequency_tick.
 */
typedef struct {
	double				report_period;	///< Time (s.) between two reportings on the standard output of the frequency.
	const char*			label;		///< A string used as a prefix to the reporting.

	int				count;		///< Current number of calls.
	double				start;		///< Date of the last reporting.
	double				last;		///< Date of the last call to \ref frequency_tick.
	double				min;		///< Minimum amount of time between two calls since last reporting.
	double				max;		///< Maximum amount of time between two calls since last reporting.
	double				sum;		///< Used to compute average amount of time between two calls.
	double				sum2;		///< Used to compute standard deviation of the amount of time between two calls.
} frequency_counter_t;

/**
 * @brief	Initialized a \ref frequency_counter_t. Must be called once before using the \ref frequency_counter_t
 *		in calls to \ref frequency_tick.
 *
 * @param	fc		A pointer to the \ref frequency_counter_t that must be initialized.
 */
void frequency_init(frequency_counter_t* fc);

/**
 * @brief	Call this function regularly to get statistics about the frequency of the calls.
 *		The statistics are displayed on the standard output at regular time intervals
 *		(see "report_period" in \ref frequency_counter_t).
 *		The statistics are only displayed during a call to this function.
 *
 * @param	fc		A pointer to the \ref frequency_counter_t that stores statistics about these calls.
 */
void frequency_tick(frequency_counter_t* fc);

#endif

