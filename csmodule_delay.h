/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Delays
 *
 * PURPOSE:
 * sub-sample accurate delay
 *
 * @version 2010/10/25 v0.1
 * @version 2010/11/05 _DelaySpread
 * @version 2010/11/16 _DelaySam
 * @version 2011/2/5 _DelayR
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_DELAY_H_INCLUDED
#define CSMODULE_DELAY_H_INCLUDED

#include "csmodule.h"

class CSmodule_Delay: public CSmodule
{
	public:

	csfloat
		*_in,
		*_amp,
		*_sam,
		*_sec,
		*_rst,
		lrst,

		*_out,

		*buf;
	int
		buflen,
		bufpos,
		quality;
	csfloat
		buflensec;

	char *docString();
	~CSmodule_Delay();
	CSmodule_Delay *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	void setSampleRate(int sr);
	void setBuf(float sec);

	void step();

};

class CSmodule_DelaySam: public CSmodule
{
	public:

	csfloat
		*_in,
		*_sam,

		*_out,

		*buf;
	int
		buflen,
		bufpos;

	char *docString();
	~CSmodule_DelaySam();
	CSmodule_DelaySam *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setBuf(int sam);

	void step();

};


class CSmodule_DelaySpread: public CSmodule_Delay
{
	csfloat
		*_spr,
		*_sprl,
		*_spra;

	public:
	char *docString();
	CSmodule_DelaySpread *newOne();
	void init();

	void step();

};






class CSmodule_DelayOneSample: public CSmodule
{
	csfloat
		*_in,
		*_out;

	public:
	char *docString();
	CSmodule_DelayOneSample *newOne();
	void init();

	void step();

};








class CSmodule_RoomDelay: public CSmodule
{
	public:

	csfloat
		*_in,
		*_amp,
		*_X,
		*_Y,

		*_refl,
		*_sizeX,
		*_sizeY,
		*_sos,

		*_A,
		*_B,
		*_C,
		*_D,

		*buf;
	int
		buflen,
		bufpos;

	char *docString();
	~CSmodule_RoomDelay();
	CSmodule_RoomDelay *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setBuf(float sec);

	void step();

};



class CSmodule_Reverb: public CSmodule
{
	public:

	csfloat
		*_in,
		*_sec,
		*_len,
		*_wet,
		*_amp,
		*_freq,	lfreq, coeff1,
		*_fb,
		*_rst,
		lrst,

		*_out, sam,

		*buf,
		*revamp,
		*revpos,

		revtfac,
		revafac,
		revofac;
	int
		buflen,
		buflenand,
		bufpos,
		revlen;

	char *docString();
	~CSmodule_Reverb();
	CSmodule_Reverb *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	void setSampleRate(int sr);
	void setBuf(int po);
	void setRev(int nr);

	void step();

};



#endif // CSMODULE_DELAY_H_INCLUDED
