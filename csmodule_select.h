/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Select
 *
 * PURPOSE:
 * crossfade between several inputs
 *
 * @version 2010/11/03 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_SELECT_H_INCLUDED
#define CSMODULE_SELECT_H_INCLUDED

#include "csmodule.h"

class CSmodule_Select: public CSmodule
{
	csfloat
		*_sel,
		*_in[CSMOD_MAX_CHAN],
		*_out;

	CSmoduleConnector
		*__in[CSMOD_MAX_CHAN];

	int nrIn, oldNrIn;

	public:
	char *docString();
	CSmodule_Select *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn);

	void step();

};





class CSmodule_SelectOut: public CSmodule
{
	csfloat
		*_sel,
		*_in,
		*_out[CSMOD_MAX_CHAN];

	CSmoduleConnector
		*__out[CSMOD_MAX_CHAN];

	int nrOut, oldNrOut;

	public:
	char *docString();
	CSmodule_SelectOut *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrOut(int newNrOut);

	void step();

};





class CSmodule_SelectInNN: public CSmodule
{
	csfloat
		*_sel,
		*_in[CSMOD_MAX_CHAN*CSMOD_MAX_CHAN],
		*_out[CSMOD_MAX_CHAN];

	CSmoduleConnector
		*__in[CSMOD_MAX_CHAN*CSMOD_MAX_CHAN],
		*__out[CSMOD_MAX_CHAN];

	int nrIn, nrChan;

	public:
	char *docString();
	CSmodule_SelectInNN *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn, int newNrChan);

	void step();

};

#endif // CSMODULE_SELECT_H_INCLUDED
