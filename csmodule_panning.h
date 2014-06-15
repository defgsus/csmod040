/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: various 3d panning stuff
 *
 * PURPOSE:
 *   3d sound positioning
 *
 * @version 2011/2/1 v0.1 (yes, i am back!!)
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_PANNING_H_INCLUDED
#define CSMODULE_PANNING_H_INCLUDED

#include "csmodule.h"


class CSmodule_Pos2D: public CSmodule
{
	csfloat
		*_rad,
		*_falloff,
		*_ang,
		*_micamp,
		*_micdly,
		*_sos,
		*_cx,
		*_cz,
		*_mx[CSMOD_MAX_CHAN],
		*_mz[CSMOD_MAX_CHAN],
		*_in[CSMOD_MAX_CHAN],
		*_ax[CSMOD_MAX_CHAN],
		*_az[CSMOD_MAX_CHAN],

		*_out[CSMOD_MAX_CHAN];

	CSmoduleConnector
		*__sos,
		*__mx[CSMOD_MAX_CHAN],
		*__mz[CSMOD_MAX_CHAN],
		*__in[CSMOD_MAX_CHAN],
		*__ax[CSMOD_MAX_CHAN],
		*__az[CSMOD_MAX_CHAN],

		*__out[CSMOD_MAX_CHAN];

	int nrSpeaker, nrIn,
		buflen,newbuflen,
		bufpos,
		usebuf,
		changebuf;

	csfloat
		cursec,
		*buf,
		*buf1,
		*buf2;

	bool doDelay;

	public:
	char *docString();
	CSmodule_Pos2D *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn);
	void setNrSpeaker(int newNrSpeaker);
	void setBuf(float sec);

	void step();

};



class CSmodule_Pos3D: public CSmodule
{
	csfloat
		*_rad,
		*_falloff,
		*_ang,
		*_micamp,
		*_micdly,
		*_sos,
		*_cx,
		*_cy,
		*_cz,
		*_mx[CSMOD_MAX_CHAN],
		*_my[CSMOD_MAX_CHAN],
		*_mz[CSMOD_MAX_CHAN],
		*_in[CSMOD_MAX_CHAN],
		*_ax[CSMOD_MAX_CHAN],
		*_ay[CSMOD_MAX_CHAN],
		*_az[CSMOD_MAX_CHAN],

		*_out[CSMOD_MAX_CHAN];

	CSmoduleConnector
		*__sos,
		*__mx[CSMOD_MAX_CHAN],
		*__my[CSMOD_MAX_CHAN],
		*__mz[CSMOD_MAX_CHAN],
		*__in[CSMOD_MAX_CHAN],
		*__ax[CSMOD_MAX_CHAN],
		*__ay[CSMOD_MAX_CHAN],
		*__az[CSMOD_MAX_CHAN],

		*__out[CSMOD_MAX_CHAN];

	int nrSpeaker, nrIn,
		buflen,newbuflen,
		bufpos,
		usebuf,
		changebuf;

	csfloat
		cursec,
		*buf,
		*buf1,
		*buf2;

	bool doDelay;

	public:
	char *docString();
	CSmodule_Pos3D *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn);
	void setNrSpeaker(int newNrSpeaker);
	void setBuf(float sec);

	void step();

};




#endif // CSMODULE_PANNING_H_INCLUDED
