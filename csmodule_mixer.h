/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Constant
 *
 * PURPOSE:
 * send a value, or scale/offset some input
 *
 * @version 2010/09/24 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_MIXER_H_INCLUDED
#define CSMODULE_MIXER_H_INCLUDED

#include "csmodule.h"


class CSmodule_Mixer: public CSmodule
{
	csfloat
		*_mainAmp,
		*_in[CSMOD_MAX_CHAN],
		*_amp[CSMOD_MAX_CHAN],
		*_out;

	CSmoduleConnector
		*__in[CSMOD_MAX_CHAN],
		*__amp[CSMOD_MAX_CHAN];


	int nrIn, oldNrIn;

	public:
	char *docString();
	CSmodule_Mixer* newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn);

	void step();

};


#endif // CSMODULE_MIXER_H_INCLUDED
