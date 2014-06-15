/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Band Detector
 *
 * PURPOSE:
 *	detect single band volume through goertzel DFT
 *
 * @version 2010/10/27 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_BAND_H_INCLUDED
#define CSMODULE_BAND_H_INCLUDED

#include "csmodule.h"

class CSmodule_Band: public CSmodule
{
	csfloat

		*_in,
		*_freq,

		*_out,

		*buf;

	int
		bufpos,
		buflen;

	public:
	char *docString();
	~CSmodule_Band();
	CSmodule_Band *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void step();

};


#endif // CSMODULE_BAND_H_INCLUDED
