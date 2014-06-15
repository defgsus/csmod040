/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: default oscillator / clock
 *
 * PURPOSE:
 * generate sine, square, saw wave and triangle wave
 * output gate at BPM
 *
 * @version 2010/10/15 v0.1
 * @version 2010/10/29 (triangle / CSclipFloat() for phase / _OscSpec class)
 * @version 2010/11/05 _Clock module / _OscLU
 * @version 2010/12/07 _MultiSin
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_OSC_H_INCLUDED
#define CSMODULE_OSC_H_INCLUDED

#include "csmodule.h"

#define CSMOD_OSC_MAXVOICE (512)

class CSmodule_Osc: public CSmodule
{
	public:
	csfloat
		*_freq,
		*_phase,
		*_amp,
		*_off,
		*_rst,
		*_pulse,
		*_outSine,
		*_outCosine,
		*_outSquare,
		*_outSaw,
		*_outTri,

		lrst;
	double
		phase;


	CSmoduleConnector
		*o_sine,
		*o_cosine,
		*o_square,
		*o_saw,
		*o_tri;

	int
		// scope rectangle
		sx,sy,sw,sh;

	CStimeline
		*scope;

	public:
	char *docString();
    CSmodule_Osc();
	~CSmodule_Osc();
	CSmodule_Osc* newOne();
	void init();

	void step();

	void draw(int offx=0, int offy=0, float zoom=1.0);

};




class CSmodule_OscPOut: public CSmodule_Osc
{
	public:
	csfloat
		*_outPhase;

	public:
    CSmodule_OscPOut* newOne();
	void init();

	void step();

};


class CSmodule_OscP: public CSmodule_Osc
{
	csfloat
		*_nrPer;

	int
		nrPer;

	public:
	char *docString();
    CSmodule_OscP* newOne();
	void init();

	void step();

};



#define CSMOD_OSC_TABLE 0x10000

class CSmodule_OscLU: public CSmodule
{
	csfloat
		w_sin[CSMOD_OSC_TABLE],
		w_sqr[CSMOD_OSC_TABLE],
		w_saw[CSMOD_OSC_TABLE],
		w_tri[CSMOD_OSC_TABLE];

	csfloat
		*_freq,
		*_phase,
		*_amp,
		*_off,
		*_rst,
		*_outSine,
		*_outSquare,
		*_outSaw,
		*_outTri,

		lrst;
	double
		phase;

	CSmoduleConnector
		*o_sine,
		*o_square,
		*o_saw,
		*o_tri;

	public:
	char *docString();
	CSmodule_OscLU* newOne();
	void init();

	void step();

};





class CSmodule_OscSpec: public CSmodule
{
	csfloat
		*_freq,
		*_phase,
		*_amp,
		*_off,
		*_rst,
		*_nr,
		*_mul,
		*_add,
		*_fade,

		*_outSine,

		lrst;
	double
		phase[CSMOD_OSC_MAXVOICE];

	int
		// scope rectangle
		sx,sy,sw,sh;

	CStimeline
		*scope;

	public:
	char *docString();
    CSmodule_OscSpec();
	~CSmodule_OscSpec();
	CSmodule_OscSpec* newOne();
	void init();

	void step();

	void draw(int offx=0, int offy=0, float zoom=1.0);

};





class CSmodule_Clock: public CSmodule
{
	csfloat
		*_bpm,
		*_rst,
		*_out,
		*_tick[CSMOD_MAX_CHAN],
		*_otick[CSMOD_MAX_CHAN],

		lrst,
		count,
		count0[CSMOD_MAX_CHAN];

	int
		nrIn,
		oldNrIn;

	public:
	char *docString();
	CSmodule_Clock* newOne();
	void init();

	void propertyCallback(CSmodProperty *prop);
	void setNrIn(int newNrIn);

	void step();
};





class CSmodule_MultiSin: public CSmodule
{
	csfloat
		*_freq,
		*_phase,
		*_amp,
		*_off,
		*_rst,
		*_mul,
		*_add,
		*_out[CSMOD_MAX_CHAN],

		lrst,
		phase;

	CSmoduleConnector
		*__out[CSMOD_MAX_CHAN];

	int nrOut, oldNrOut;

	public:
	char *docString();
	CSmodule_MultiSin *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrOut(int newNrOut);

	void step();

};







class CSmodule_PolyOsc: public CSmodule
{
	csfloat
		*_freq,
		*_phase,
		*_amp,
		*_off,
		*_rst,
		*_pulse,
		*_mul,
		*_add,
		*_padd,
		*_out[CSMOD_MAX_CHAN],

		lrst,
		phase[CSMOD_MAX_CHAN];

	CSmoduleConnector
		*__out[CSMOD_MAX_CHAN];

	int nrOut, oldNrOut, wave;

	public:
	char *docString();
	CSmodule_PolyOsc *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrOut(int newNrOut);
	void setWave(int newWave);

	void step();

};




class CSmodule_Osc3: public CSmodule
{
	public:
	csfloat
		*_amp,
		*_off,
		*_freq,
		*_phase,
		*_sync, lsync,
		*_softsync, lsoftsync,

		*_f1,
		*_f2,
		*_f3,
		*_p1,
		*_p2,
		*_p3,
		*_a1,
		*_a2,
		*_a3,
		*_o1,
		*_o2,
		*_o3,

		*_out1,
		*_out2,
		*_out3,

		coeff1;
	double
		phase1, phase1a, phase1b,
		phase2, phase2a, phase2b,
		phase3, phase3a, phase3b;

	public:
	char *docString();
	CSmodule_Osc3* newOne();
	void init();

	void step();

};



#endif // CSMODULE_OSC_H_INCLUDED
