/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: int2bin
 *
 * PURPOSE:
 * bit de-and-composition
 *
 * @version 2010/12/02 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_BIT_H_INCLUDED
#define CSMODULE_BIT_H_INCLUDED

#include "csmodule.h"

#define CSMOD_BIT_MAX 32

class CSmodule_BitIn: public CSmodule
{
	csfloat
		*_in[CSMOD_BIT_MAX],
		*_out;

	CSmoduleConnector
		*__in[CSMOD_BIT_MAX];

	int nrIn, oldNrIn;

	public:
	char *docString();
	CSmodule_BitIn *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn);

	void step();

};




class CSmodule_BitOut: public CSmodule
{
	csfloat
		*_in,
		*_out[CSMOD_BIT_MAX];

	CSmoduleConnector
		*__out[CSMOD_BIT_MAX];

	int nrOut, oldNrOut;

	public:
	char *docString();
	CSmodule_BitOut *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrOut(int newNrOut);

	void step();

};



#endif // CSMODULE_BIT_H_INCLUDED
