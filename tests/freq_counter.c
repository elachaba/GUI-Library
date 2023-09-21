//
//  freq_counter.c
//  projet_c_LLIG
//
//  Created by François Bérard on 26/3/15.
//  Copyright (c) 2015 LIG-IIHM. All rights reserved.
//

#include "freq_counter.h"

#include "hw_interface.h"
#include <math.h>
#include <stdio.h>



void frequency_init(frequency_counter_t* fc)
{
	fc->report_period		= 2.0;
	fc->count			= -1;
	fc->label			= "frequency_counter";
}

void frequency_tick(frequency_counter_t* fc)
{
	double				now		= hw_now();
	double				this_one;
	double				average;
	double				stddev;

	if (fc->count != -1) {

		this_one		= now - fc->last;

		fc->last		= now;
		fc->count++;
		fc->sum			+= this_one;
		fc->sum2		+= this_one*this_one;

		if ((fc->min == -1.0) || (this_one < fc->min))
			fc->min	= this_one;
		if ((fc->max == -1.0) || (this_one > fc->max))
			fc->max	= this_one;

		if ((now - fc->start) >= fc->report_period) {
			average		= fc->sum / (double)(fc->count);
			stddev		= sqrt(fc->sum2 / (double)(fc->count) - average*average);
			printf("%s: %fHz. Avg. period %f [%f-%f] sttdev %f\n", fc->label, (double)(fc->count) / (now - fc->start), average, fc->min, fc->max, stddev);

			fc->count	= -1;
		}
	}

	if (fc->count == -1) {
		fc->count		= 0;
		fc->start		= now;
		fc->last		= now;
		fc->min			= -1.0;
		fc->max			= -1.0;
		fc->sum			= 0.0;
		fc->sum2		= 0.0;
	}
}
