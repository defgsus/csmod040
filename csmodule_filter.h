/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: MULTI FILTER
 *
 * PURPOSE:
 * low pass, high pass
 *
 * @version 2010/10/27 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_FILTER_H_INCLUDED
#define CSMODULE_FILTER_H_INCLUDED

#include "csmodule.h"

class CSmodule_Filter: public CSmodule
{
	csfloat
		*_in,
		*_amp,
		*_freq,
		*_reso,

		*_outLP,
		*_outBP,
		*_outHP,

		sam,
		prevsam0,
		prevsam1,
		prevsam0b,
		prevsam1b,
		lfreq,
		lreso,
		coeff1,
		coeff2;

	CSmoduleConnector
		*BPcon;

	public:
	char *docString();
	CSmodule_Filter *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	void setSampleRate(int sr);

	void step();

};




class CSmodule_Filter24: public CSmodule
{
	csfloat
		*_in,
		*_amp,
		*_freq,
		*_ripple,

		*_outLP,
		*_outHP,

		lfreq,
		lripple,
		lout0,
		lout1;

	double
		k,
		sg,
		cg,
		coeff0,
		coeff1,
		coeff2,
		coeff3,
		a0,a1,a2,a3,a4,a5,
		b0,b1,b2,b3,b4,b5,
		stage1,state0,state1,state2,state3;


	public:
	char *docString();
	CSmodule_Filter24 *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	void setSampleRate(int sr);
	void step();

};






class CSmodule_Follow: public CSmodule
{
	csfloat
		*_in,
		*_up,
		*_down,

		*_out;

	public:
	char *docString();
	CSmodule_Follow *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void step();

};




class CSmodule_FollowMulti: public CSmodule
{
	csfloat
		*_up,
		*_down,
		*_in[CSMOD_MAX_CHAN],
		*_out[CSMOD_MAX_CHAN];

	CSmoduleConnector
		*__in[CSMOD_MAX_CHAN],
		*__out[CSMOD_MAX_CHAN];

	int
		nrIn, oldNrIn,
		mop;

	public:
	char *docString();
	CSmodule_FollowMulti* newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn);

	void step();

};










class CSmodule_Eq: public CSmodule
{
	csfloat
		*_input,
		*_freq[CSMOD_MAX_CHAN],
		*_reso[CSMOD_MAX_CHAN],
		*_amp[CSMOD_MAX_CHAN],
		*_out,

		saml[CSMOD_MAX_CHAN],
		samb[CSMOD_MAX_CHAN],
		prevsam0[CSMOD_MAX_CHAN],
		prevsam1[CSMOD_MAX_CHAN],
		prevsam0b[CSMOD_MAX_CHAN],
		prevsam1b[CSMOD_MAX_CHAN],
		lfreq[CSMOD_MAX_CHAN],
		lreso[CSMOD_MAX_CHAN],
		coeff1[CSMOD_MAX_CHAN],
		coeff2[CSMOD_MAX_CHAN];

	CSmoduleConnector
		*__freq[CSMOD_MAX_CHAN],
		*__reso[CSMOD_MAX_CHAN],
		*__amp[CSMOD_MAX_CHAN];

	int
		nrIn;


	public:
	char *docString();
	CSmodule_Eq* newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn);
	void setSampleRate(int sr);

	void step();

};

#endif // CSMODULE_FILTER_H_INCLUDED
