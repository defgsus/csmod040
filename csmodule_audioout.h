/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Audio Output
 *
 * PURPOSE:
 * pass audio data through to container->outputSample[]
 *
 * @version 2010/10/13 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_AUDIOOUT_H_INCLUDED
#define CSMODULE_AUDIOOUT_H_INCLUDED

#include "csmodule.h"


class CSmodule_AudioOut: public CSmodule
{
	// pointers for easy value acces
	csfloat
		*_amp,
		*_inmono,
		*_in[CSMOD_MAX_CHAN];

	CSmoduleConnector
		*__in[CSMOD_MAX_CHAN];

	int nrIn, oldNrIn;

	public:
	char *docString();
	CSmodule_AudioOut* newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn);

	void step();

};



#endif // CSMODULE_AUDIOOUT_H_INCLUDED
