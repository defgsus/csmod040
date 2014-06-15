/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: random number generator
 *
 * PURPOSE:
 * 	_Rnd: outputs a pseudo random, controllable sequence of random numbers
 *  _RndTrig: outputs a gate signal with random distribution in time
 *  _Noise: simply makes NOISE!!!
 *
 * @version 2010/10/27 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_RND_H_INCLUDED
#define CSMODULE_RND_H_INCLUDED

#include "csmodule.h"

class CSmodule_Rnd: public CSmodule
{
	csfloat

		*_rst,
		*_gate,
		*_seed,
		*_seed1,
		*_seed2,
		*_amp,
		*_off,

		*_out,

        lrst,
        lgate;

	unsigned int
        seed;

	public:
	char *docString();
	CSmodule_Rnd *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void step();

};




class CSmodule_RndTrig: public CSmodule
{
	csfloat

		*_rst,
		*_gate,
		*_seed,
		*_seed1,
		*_seed2,
		*_prob,

		*_out,

        lrst,
        lgate;

	unsigned int
        seed;

	public:
	char *docString();
	CSmodule_RndTrig *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void step();

};





class CSmodule_NoiseWhite: public CSmodule
{
	csfloat

		*_rst,
		*_seed,
		*_amp,
		*_off,
		*_out,

		lrst;

	unsigned int
        seed;

	public:
	char *docString();
	CSmodule_NoiseWhite *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void step();

};



class CSmodule_NoisePitch: public CSmodule
{
	csfloat

		*_rst,
		*_amp,
		*_off,
		*_freq,
		*_pattern,

		*_out,
		*_outL,
		*_outS,

        lrst,

        n0,n1;

	unsigned int
		frame,
        seed;

	CSmoduleConnector
		*outL,
		*outS;

	public:
	char *docString();
	CSmodule_NoisePitch *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void step();

};

#endif // CSMODULE_RND_H_INCLUDED
