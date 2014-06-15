/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Sample & Hold / Counter
 *
 * PURPOSE:
 * hold input value until positive edge
 * count input gates
 *
 * @version 2010/10/25 v0.1
 * @version 2010/10/29 _Counter Module
 * @version 2010/11/05 _SampleGate Module
 * @version 2010/11/15 _SampleGate moved to csmodule_system.h
 * @version 2011/10/02 _SHfollow/multi/poly
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_SH_H_INCLUDED
#define CSMODULE_SH_H_INCLUDED

#include "csmodule.h"

class CSmodule_SH: public CSmodule
{
	csfloat
		*_in,
		*_rst,
		*_out,
		lrst;

	public:
	char *docString();
	CSmodule_SH *newOne();
	void init();

	void step();

};


class CSmodule_SHfollow: public CSmodule
{
	csfloat
		*_in,
		*_rst,
		*_time,
		*_out,
		*_outS,

		lrst,
		sam,samo;

	CSmoduleConnector
		*__outS;

	unsigned int frame,mframe;

	public:
	char *docString();
	CSmodule_SHfollow *newOne();
	void init();

	void step();

};



class CSmodule_SHmulti: public CSmodule
{
	csfloat
		*_in,
		*_rst,
		*_out[CSMOD_MAX_CHAN],
		lrst;

	CSmoduleConnector
		*__out[CSMOD_MAX_CHAN];

	int nrOut, oldNrOut;

	public:
	char *docString();
	CSmodule_SHmulti *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	void setNrOut(int newNrOut);

	void step();

};



class CSmodule_SHpoly: public CSmodule
{
	csfloat
		*_in,
		*_gate,
		*_rst,
		*_nrVoices,
		*_outnr,
		*_out[CSMOD_MAX_CHAN],

		lgate,
		lrst;

	CSmoduleConnector
		*__out[CSMOD_MAX_CHAN];

	int nrOut, oldNrOut, chan;

	public:
	char *docString();
	CSmodule_SHpoly *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	void setNrOut(int newNrOut);

	void step();

};


class CSmodule_Counter: public CSmodule
{
	csfloat
		*_gate,
		*_min,
		*_max,
		*_rst,
		*_load,
		*_dir,
		*_out,
		*_outM,
		lrst,
		lgate;

	public:
	char *docString();
	CSmodule_Counter *newOne();
	void init();

	void step();

};






class CSmodule_SampleCounter: public CSmodule
{
	csfloat
		*_min,
		*_max,
		*_rst,
		*_load,
		*_dir,
		*_out,
		*_outM,
		lrst,
		lgate;

	public:
	char *docString();
	CSmodule_SampleCounter *newOne();
	void init();

	void step();

};


class CSmodule_CounterSec: public CSmodule
{
	csfloat
		*_gate,
		*_max,
		*_out,
		*_outM,
		lgate;

	unsigned int
		seconds,
		samples;

	public:
	char *docString();
	CSmodule_CounterSec *newOne();
	void init();

	void step();

};




#endif // CSMODULE_SH_H_INCLUDED
