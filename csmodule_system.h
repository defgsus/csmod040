/**
 *
 * CS MODULAR EDITOR
 *
 * MODULES: Sample Gate, Start/Stop Gate
 *
 * PURPOSE:
 * Sample Gate: output gate every other sample
 * Start/Stop Gate: output gate on every start of audio engine
 *
 * @version 2010/11/15 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_SYSTEM_H_INCLUDED
#define CSMODULE_SYSTEM_H_INCLUDED

#include "csmodule.h"
#include "csmodcontainer.h"

class CSmodule_StartGate: public CSmodule
{
	csfloat
		*_out;

	bool started;

	public:
	char *docString();
	CSmodule_StartGate *newOne();

	void init();

	void start();

	void step();

};




class CSmodule_SampleGate: public CSmodule
{
	csfloat
		*_out;

	public:
	char *docString();
	CSmodule_SampleGate *newOne();
	void init();

	void step();

};



class CSmodule_SampleRate: public CSmodule
{
	csfloat
		*_out;

	public:
	char *docString();
	CSmodule_SampleRate *newOne();

	void init();

	void step();

};


class CSmodule_OnRender: public CSmodule
{
	csfloat
		*_out;

	public:
	char *docString();
	CSmodule_OnRender *newOne();

	void init();

	void step();

};

#endif // CSMODULE_SYSTEM_H_INCLUDED
