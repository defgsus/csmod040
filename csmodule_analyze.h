/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: analyzing tools
 *
 * PURPOSE:
 * value meter
 *
 * @version 2010/12/19 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_ANALYZE_H_INCLUDED
#define CSMODULE_ANALYZE_H_INCLUDED

#include "csmodule.h"

class CSmodule_ValueMeter: public CSmodule
{
	csfloat
		*_in[CSMOD_MAX_CHAN],

		range,
		rangemin,
		rangemax,
		temp[CSMOD_MAX_CHAN];

	CSmoduleConnector
		*__in[CSMOD_MAX_CHAN];

	int nrIn, oldNrIn;
	bool absolute;


	public:
	char *docString();
	CSmodule_ValueMeter *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	void setNrIn(int newNrIn);


	void step();

	virtual void draw(int offx=0, int offy=0, float zoom=1.0);

};



class CSmodule_PitchDetect: public CSmodule
{
	csfloat
		*_in,
		*_follow,
		*_out,

		lin;

	unsigned int
		samples,
		samplesc;

	public:
	char *docString();
	CSmodule_PitchDetect *newOne();
	void init();


	void step();

};




#endif // CSMODULE_ANALYZE_H_INCLUDED
