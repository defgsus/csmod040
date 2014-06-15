/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Audio Input
 *
 * PURPOSE:
 * pass audio data through from container->inputSamples[]
 *
 * @version 2010/10/17 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_AUDIOIN_H_INCLUDED
#define CSMODULE_AUDIOIN_H_INCLUDED

#include "csmodule.h"


class CSmodule_AudioIn: public CSmodule
{
	// pointers for easy value acces
	csfloat
		*_amp,
		*_outmono,
		*_out[CSMOD_MAX_CHAN];

	CSmoduleConnector
		*__out[CSMOD_MAX_CHAN];

	int nrOut, oldNrOut;

	public:
	char *docString();
	CSmodule_AudioIn* newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrOut(int newNrIn);

	void step();

};



#endif // CSMODULE_AUDIOIN_H_INCLUDED
