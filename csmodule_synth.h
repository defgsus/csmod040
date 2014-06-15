/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Synthesizers
 *
 * PURPOSE: stuff to make polyphonic voices easier
 *
 * @version 2011/03/11 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_SYNTH_H_INCLUDED
#define CSMODULE_SYNTH_H_INCLUDED

#include "csmodule.h"


class CSmodule_Synth: public CSmodule
{
	public:
	csfloat
		*_nrVoices,
		*_panic, lpanic,
		*_amp,

		*_gate, lgate,
		*_freq,
		*_vel,
		*_attack, lattack,
		*_decay, ldecay,
		//*_activeVoices,
		*_outm,
		*_out[CSMOD_MIDI_MAX_POLY],

		freq[CSMOD_MIDI_MAX_POLY],
		phase[CSMOD_MIDI_MAX_POLY],
		velo[CSMOD_MIDI_MAX_POLY],
		env[CSMOD_MIDI_MAX_POLY],

		coeffi1, coeffi2,
		coeff1[CSMOD_MIDI_MAX_POLY],
		coeff2[CSMOD_MIDI_MAX_POLY];

	CSmoduleConnector
		*__out[CSMOD_MIDI_MAX_POLY];

	int
		active[CSMOD_MIDI_MAX_POLY],
		voiceNr,

		nrOut,
		nrActive,

		imidiateEnvelope;


	char *docString();
	CSmodule_Synth *newOne();

	void init();
	void propertyCallback(CSmodProperty *prop);
	void setNrOut(int newNrOut);

	virtual void panic();

	void step();

};



class CSmodule_SynthMulti: public CSmodule_Synth
{
	public:
	csfloat
		*_pw,
		*_phm,
		*_wave,

		pw[CSMOD_MIDI_MAX_POLY];
	int
		waveForm[CSMOD_MIDI_MAX_POLY];

	char *docString();
	CSmodule_SynthMulti *newOne();

	void init();

	void step();

};





class CSmodule_SynthMultiQuadPos: public CSmodule_SynthMulti
{
	public:
	csfloat
		*_angle,
		*_mod,

		*_outrl,
		*_outrr,
		*_outfr,
		*_outfl,

		amprl[CSMOD_MIDI_MAX_POLY],
		amprr[CSMOD_MIDI_MAX_POLY],
		ampfl[CSMOD_MIDI_MAX_POLY],
		ampfr[CSMOD_MIDI_MAX_POLY];

	char *docString();
	CSmodule_SynthMultiQuadPos *newOne();

	void init();

	void step();

};





#define CSMOD_SYNTH3D_MAX_POLY 64
#define CSMOD_SYNTH3D_CHAN 6
#define CSMOD_SYNTH3D_MAX_DELAY_SHIFT 13
#define CSMOD_SYNTH3D_MAX_DELAY (1<<CSMOD_SYNTH3D_MAX_DELAY_SHIFT)

class CSmodule_SynthMulti3D: public CSmodule_SynthMulti
{
	public:
	csfloat
		*_rad,
		*_falloff,
		*_sos,
		*_cx,
		*_cy,
		*_cz,

		*_x,
		*_y,
		*_z,

		*_outsp[CSMOD_SYNTH3D_CHAN],

		x[CSMOD_SYNTH3D_MAX_POLY],
		y[CSMOD_SYNTH3D_MAX_POLY],
		z[CSMOD_SYNTH3D_MAX_POLY],
		dist[CSMOD_SYNTH3D_MAX_POLY],

		mx[CSMOD_SYNTH3D_CHAN],
		my[CSMOD_SYNTH3D_CHAN],
		mz[CSMOD_SYNTH3D_CHAN],
		cmx[CSMOD_SYNTH3D_CHAN],
		cmy[CSMOD_SYNTH3D_CHAN],
		cmz[CSMOD_SYNTH3D_CHAN],
		dcm[CSMOD_SYNTH3D_CHAN],

		dbuf[CSMOD_SYNTH3D_MAX_POLY * CSMOD_SYNTH3D_MAX_DELAY];

	CSmoduleConnector
		*__outsp[CSMOD_SYNTH3D_CHAN];

	int
		bufpos;

	char *docString();
	CSmodule_SynthMulti3D *newOne();

	void init();
	void propertyCallback(CSmodProperty *prop);

	void calcMicros();
	void step();

};







class CSmodule_SynthMultiFilter3D: public CSmodule_SynthMulti3D
{
	public:
	csfloat
		*_cutoff,
		*_cutoff_res,
		*_cutoff_kf,
		*_fenv,
		*_fattack,
		*_fdecay,

		fcoeff1[CSMOD_SYNTH3D_MAX_POLY],
		fcoeff2[CSMOD_SYNTH3D_MAX_POLY],
		fecoeff1[CSMOD_SYNTH3D_MAX_POLY],
		fecoeff2[CSMOD_SYNTH3D_MAX_POLY],
		fenv[CSMOD_SYNTH3D_MAX_POLY],
		fenvmax[CSMOD_SYNTH3D_MAX_POLY],
		// filter sample buffer
		fbuf[CSMOD_SYNTH3D_MAX_POLY << 1];

	int
		fenvmode[CSMOD_SYNTH3D_MAX_POLY];

	char *docString();
	CSmodule_SynthMultiFilter3D *newOne();

	void init();
	void propertyCallback(CSmodProperty *prop);

	void panic();

	void step();

};

#endif // CSMODULE_SYNTH_H_INCLUDED
