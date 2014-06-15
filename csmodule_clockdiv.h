/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: CLOCK DIVIDER
 *
 * PURPOSE:
 * divide frequency of any periodical input signal as square wave
 *
 * @version 2010/10/26 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_CLOCKDIV_H_INCLUDED
#define CSMODULE_CLOCKDIV_H_INCLUDED

#include "csmodule.h"

class CSmodule_ClockDiv: public CSmodule
{
	csfloat
		*_rst,
		*_clock,
		*_div,
		*_pulse,

		*_out,

		lrst,
		lclock;

	unsigned int
        count,
        spc,
        frame,
        oframe;

	public:
	char *docString();
	CSmodule_ClockDiv *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void step();

};



#endif // CSMODULE_CLOCKDIV_H_INCLUDED
