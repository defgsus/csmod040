/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: trans sum matrix
 *
 * PURPOSE:
 * two or three dimensional sum matrix for cymasonic pan fade
 *
 * @version 2010/10/24 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_SUM_H_INCLUDED
#define CSMODULE_SUM_H_INCLUDED

#include "csmodule.h"

class CSmodule_Sum: public CSmodule
{
	csfloat
		*_amp,
		*_off,
		*_X,
		*_Y,
		*_Z,
		*_A,
		*_B,
		*_C,
		*_D;

	public:
	char *docString();
	CSmodule_Sum *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void step();

};




class CSmodule_QuadPos: public CSmodule
{
	csfloat
		*_amp,
		*_off,
		*_ang,
		*_A,
		*_B,
		*_C,
		*_D;

	public:
	char *docString();
	CSmodule_QuadPos *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void step();

};












class CSmodule_QuadMix: public CSmodule
{
	public:
	csfloat
		*_amp,
		*_off,
		*_in[CSMOD_MAX_CHAN],
		*_ang[CSMOD_MAX_CHAN],
		*_A,
		*_B,
		*_C,
		*_D;

	CSmoduleConnector
		*__in[CSMOD_MAX_CHAN],
		*__ang[CSMOD_MAX_CHAN];

	int nrIn, oldNrIn;

	char *docString();
	CSmodule_QuadMix *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	void setNrIn(int newNrIn);

	void step();

};



class CSmodule_QuadMixMod: public CSmodule_QuadMix
{
	csfloat
		*_amt;

	public:
	char *docString();
	CSmodule_QuadMixMod *newOne();
	void init();
	void step();
};


#endif // CSMODULE_SUM_H_INCLUDED
