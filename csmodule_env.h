/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Envelope Generator(s)
 *
 * PURPOSE:
 * different envelope implementations
 *
 * @version 2010/10/27 v0.1
 * @version 2011/02/19 v0.2 _Fade
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_ENV_H_INCLUDED
#define CSMODULE_ENV_H_INCLUDED

#include "csmodule.h"

class CSmodule_EnvD: public CSmodule
{
	csfloat
		*_rst,
		*_velo,
		*_amp,
		*_decay,

		*_out,

		level,
		lrst,
		ldecay,
		coeff;

	public:
	char *docString();
	CSmodule_EnvD *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	void setSampleRate(int sr);

	void step();

};




class CSmodule_EnvAD: public CSmodule
{
	csfloat
		*_rst,
		*_amp,
		*_velo,
		*_attack,
		*_decay,

		*_out,

		level,
		velo,
		lrst,
		lattack,
		ldecay,
		coeff2,
		coeff1;

	int mode;

	public:
	char *docString();
	CSmodule_EnvAD *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	void setSampleRate(int sr);

	void step();

};




class CSmodule_EnvADMulti: public CSmodule
{
	csfloat
		*_rst[CSMOD_MAX_CHAN],
		*_amp,
		*_velo[CSMOD_MAX_CHAN],
		*_attack,
		*_decay,

		*_out[CSMOD_MAX_CHAN],

		level[CSMOD_MAX_CHAN],
		velo[CSMOD_MAX_CHAN],
		lrst[CSMOD_MAX_CHAN],
		lattack,
		ldecay,
		coeff2,
		coeff1;

	CSmoduleConnector
		*__rst[CSMOD_MAX_CHAN],
		*__velo[CSMOD_MAX_CHAN],
		*__out[CSMOD_MAX_CHAN];

	int mode[CSMOD_MAX_CHAN],
		nrOut;

	public:
	char *docString();
	CSmodule_EnvADMulti *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	void setNrOut(int newNrOut);
	void setSampleRate(int sr);

	void step();

};







class CSmodule_Fade: public CSmodule
{
	public:

	csfloat
		*_v0,
		*_v1,
		*_rst,
		*_reset,
		*_sec,
		*_shape,

		*_out,
		*_outS,

		lrst, lreset;

	unsigned long int
		samples2Pass,
		samplesPassed;

	char *docString();
	CSmodule_Fade *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	//void setSampleRate(int sr);

	void step();

};



class CSmodule_FadeFF: public CSmodule_Fade
{
	csfloat
		*_preset,
		lpreset;
	int dir;
	public:
	char *docString();
	CSmodule_FadeFF *newOne();
	void init();
	void step();

};








class CSmodule_FadeMulti: public CSmodule
{
	public:

	csfloat
		*_v0[CSMOD_MAX_CHAN],
		*_v1[CSMOD_MAX_CHAN],
		*_rst,
		*_sec,
		*_shape,

		*_out[CSMOD_MAX_CHAN],
		*_outS[CSMOD_MAX_CHAN],

		lrst;

	CSmoduleConnector
		*__v0[CSMOD_MAX_CHAN],
		*__v1[CSMOD_MAX_CHAN],
		*__out[CSMOD_MAX_CHAN],
		*__outS[CSMOD_MAX_CHAN];

	unsigned long int
		samples2Pass,
		samplesPassed;

	int nrIn;

	char *docString();
	CSmodule_FadeMulti *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	void setNrIn(int newNrIn);

	void step();

};


class CSmodule_FadeMultiFF: public CSmodule_FadeMulti
{
	csfloat
		*_reset,
		lreset;
	int dir;
	public:
	char *docString();
	CSmodule_FadeMultiFF *newOne();
	void init();
	void step();

};


#endif // CSMODULE_ENV_H_INCLUDED
