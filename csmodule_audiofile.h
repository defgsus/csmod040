/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: PlayWave
 *
 * PURPOSE:
 * stream riff wave files
 *
 * @version 2010/11/15 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_AUDIOFILE_H_INCLUDED
#define CSMODULE_AUDIOFILE_H_INCLUDED

#include "csmodule.h"
#include "csmod_wavereader.h"

class CSmodule_PlayWave: public CSmodule
{
	csfloat
		*_play,
		*_pitch,
		*_rst,
		*_pos,

		*_outP,
		*_outM,
		*_out[CSMOD_MAX_CHAN],

		lrst,

		samples[CSMOD_MAX_CHAN],
		playpos;

	CSmoduleConnector
		*__out[CSMOD_MAX_CHAN];

	int
		nrOut;

	CSwaveReader *wr;

	Fl_Color
		oldBodyColor,
		invBodyColor;

	public:
	char *docString();
	CSmodule_PlayWave();
	~CSmodule_PlayWave();
	CSmodule_PlayWave *newOne();

	void init();
	void propertyCallback(CSmodProperty *prop);
	void setNrOut(int newNrOut);

	void openFile(const char *filename);

	void step();

	void stepLow();

	void draw(int offx=0, int offy=0, float zoom=1.0);

};




#endif // CSMODULE_AUDIOFILE_H_INCLUDED
