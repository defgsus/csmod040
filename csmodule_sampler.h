/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Samplers
 *
 * PURPOSE: to play samples a bit more sophisticated
 *
 * @version 2011/03/21 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_SAMPLER_H_INCLUDED
#define CSMODULE_SAMPLER_H_INCLUDED

#include "csmodule.h"
#include "csmod_wavereader.h"

class CSmodule_SamplerPoly: public CSmodule
{
	public:
	csfloat
		*_nrVoices,
		*_panic, lpanic,
		*_amp,
		*_rootFreq,

		*_gate, lgate,
		*_freq,
		*_vel,
		*_attack, lattack,
		*_decay, ldecay,
		*_startpos,
		//*_activeVoices,
		*_outm,
		*_out[CSMOD_MIDI_MAX_POLY],

		freq[CSMOD_MIDI_MAX_POLY],
		pos[CSMOD_MIDI_MAX_POLY],
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

	CSwaveReader
		*wr;

	char *docString();
	CSmodule_SamplerPoly *newOne();
	~CSmodule_SamplerPoly();

	void init();
	void propertyCallback(CSmodProperty *prop);
	void setNrOut(int newNrOut);

	void loadWave(const char *filename);

	void panic();

	void step();
	void stepLow();

};


#endif // CSMODULE_SAMPLER_H_INCLUDED
