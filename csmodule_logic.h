/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Basic Logix
 *
 * PURPOSE:
 * bit/gate AND/OR/XOR
 *
 * @version 2010/12/25 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_LOGIC_H_INCLUDED
#define CSMODULE_LOGIC_H_INCLUDED

#include "csmodule.h"


class CSmodule_Logic: public CSmodule
{
	csfloat
		*_in[CSMOD_MAX_CHAN],
		*_out,
		*_nout;

	CSmoduleConnector
		*__in[CSMOD_MAX_CHAN];

	int
		nrIn, oldNrIn,
		mop;

	public:
	char *docString();
	CSmodule_Logic* newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn);
	void setMop(int newMop);

	void step();

};


class CSmodule_LogicOR: public CSmodule_Logic
{
	public:
	CSmodule_LogicOR* newOne();
	void init();
};

class CSmodule_LogicXOR: public CSmodule_Logic
{
	public:
	CSmodule_LogicXOR* newOne();
	void init();
};






class CSmodule_GateHold: public CSmodule
{
	csfloat
		*_gate, lgate,
		*_hold,
		*_holds,
		*_out;

	unsigned int time;

	public:
	char *docString();
	CSmodule_GateHold* newOne();
	void init();

	void step();

};


class CSmodule_GateDelay: public CSmodule
{
	csfloat
		*_gate, lgate,
		*_sec,
		*_out;

	unsigned int time;

	public:
	char *docString();
	CSmodule_GateDelay* newOne();
	void init();

	void step();

};


class CSmodule_GateRepeat: public CSmodule
{
	csfloat
		*_gate, lgate, gate,
		*_rep,
		*_out;

	unsigned int time;

	public:
	char *docString();
	CSmodule_GateRepeat* newOne();
	void init();

	void step();

};


class CSmodule_OnChange: public CSmodule
{
	csfloat
		*_in, lin,
		*_out;

	unsigned int time;

	public:
	char *docString();
	CSmodule_OnChange* newOne();
	void init();

	void step();

};








#define CSMOD_MERGE_MAX_BUF_SHIFT 13
#define CSMOD_MERGE_MAX_PARAM_SHIFT 4
#define CSMOD_MERGE_MAX_PARAM (1<<CSMOD_MERGE_MAX_PARAM_SHIFT)
#define CSMOD_MERGE_MAX_BUF (1<<CSMOD_MERGE_MAX_BUF_SHIFT)
#define CSMOD_MERGE_MAX_BUF_SIZE (CSMOD_MERGE_MAX_BUF<<CSMOD_MERGE_MAX_PARAM_SHIFT)

class CSmodule_Merge: public CSmodule
{
	csfloat
		*_rst,lrst,
		*_param[CSMOD_MAX_CHAN*CSMOD_MERGE_MAX_PARAM],
		*_out[CSMOD_MERGE_MAX_PARAM],

		lgate[CSMOD_MAX_CHAN];

	CSmoduleConnector
		*__param[CSMOD_MAX_CHAN*CSMOD_MERGE_MAX_PARAM],
		*__out[CSMOD_MERGE_MAX_PARAM];

	csfloat
		buf[CSMOD_MERGE_MAX_BUF_SIZE];

	int
		nrIn,
		nrParam,
		bufpos,
		toSend,
		waitFrame;

	public:
	char *docString();
	CSmodule_Merge* newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn, int newNrParam);

	void step();

};




#define CSMOD_STACK_MAX_PARAM_SHIFT 5
#define CSMOD_STACK_MAX_PARAM (1<<CSMOD_STACK_MAX_PARAM_SHIFT)
#define CSMOD_STACK_MAX (1<<14)

class CSmodule_StackFILO: public CSmodule
{
	csfloat
		*_push, lpush,
		*_pop, lpop,
		*_clear, lclear,
		*_in[CSMOD_STACK_MAX_PARAM],

		*_stackpos,
		*_out[CSMOD_STACK_MAX_PARAM],

		stack[CSMOD_STACK_MAX_PARAM*CSMOD_STACK_MAX];

	CSmoduleConnector
		*__in[CSMOD_STACK_MAX_PARAM],
		*__out[CSMOD_STACK_MAX_PARAM];

	int
		nrIn,
		stackpos;

	public:
	char *docString();
	CSmodule_StackFILO* newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn);

	void step();

};




class CSmodule_CounterFloat: public CSmodule
{
	csfloat
		*_direction,
		*_rst, lrst,
		*_load,
		*_min,
		*_max,
		*_out,
		*_gmin,
		*_gmax;

	public:
	char *docString();
	CSmodule_CounterFloat* newOne();
	void init();

	void step();

};


#endif // CSMODULE_LOGIC_H_INCLUDED
