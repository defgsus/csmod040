/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Basic Math
 *
 * PURPOSE:
 * add, sub, mul or div a variable number of inputs
 * sin to cos
 *
 * @version 2010/09/27 v0.1
 * @version 2010/11/27 v0.2 sin to cos
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_MATH_H_INCLUDED
#define CSMODULE_MATH_H_INCLUDED

#include "csmodule.h"


class CSmodule_Math: public CSmodule
{
	csfloat
		*_in[CSMOD_MAX_CHAN],
		*_out;

	CSmoduleConnector
		*__in[CSMOD_MAX_CHAN];

	int
		nrIn, oldNrIn,
		mop;

	public:
	char *docString();
	CSmodule_Math* newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn);
	void setMop(int newMop);

	void step();

};


class CSmodule_MathSub: public CSmodule_Math
{
	public:
	CSmodule_MathSub* newOne();
	void init();
};

class CSmodule_MathMul: public CSmodule_Math
{
	public:
	CSmodule_MathMul* newOne();
	void init();
};

class CSmodule_MathDiv: public CSmodule_Math
{
	public:
	CSmodule_MathDiv* newOne();
	void init();
};




class CSmodule_Abs: public CSmodule
{
	csfloat
		*_in,
		*_out;


	public:
	char *docString();
	CSmodule_Abs* newOne();
	void init();

	void step();

};



class CSmodule_Modulo: public CSmodule
{
	csfloat
		*_A,
		*_B,
		*_out;

	public:
	char *docString();
	CSmodule_Modulo* newOne();
	void init();

	void step();

};



class CSmodule_Power: public CSmodule
{
	csfloat
		*_A,
		*_B,
		*_out;

	public:
	char *docString();
	CSmodule_Power* newOne();
	void init();

	void step();

};


class CSmodule_Sin: public CSmodule
{
	csfloat
		*_in,
		*_out;

	public:
	char *docString();
	CSmodule_Sin* newOne();
	void init();

	void step();

};

class CSmodule_Sin1: public CSmodule
{
	csfloat
		*_in,
		*_out;

	public:
	char *docString();
	CSmodule_Sin1* newOne();
	void init();

	void step();

};




class CSmodule_Sin2Cos: public CSmodule
{
	csfloat
		*_in,
		*_phase,
		*_out;


	public:
	char *docString();
	~CSmodule_Sin2Cos();
	CSmodule_Sin2Cos* newOne();
	void init();

	void step();

};





class CSmodule_Dist: public CSmodule
{
	csfloat
		*_in1[CSMOD_MAX_CHAN],
		*_in2[CSMOD_MAX_CHAN],
		*_out;

	CSmoduleConnector
		*__in1[CSMOD_MAX_CHAN],
		*__in2[CSMOD_MAX_CHAN];

	int
		nrIn;

	public:
	char *docString();
	CSmodule_Dist* newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn);

	void step();

};


class CSmodule_Mag: public CSmodule
{
	csfloat
		*_in[CSMOD_MAX_CHAN],
		*_out;

	CSmoduleConnector
		*__in[CSMOD_MAX_CHAN];

	int
		nrIn;

	public:
	char *docString();
	CSmodule_Mag* newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn);

	void step();

};




class CSmodule_DistFactor: public CSmodule
{
	csfloat
		*_x1,
		*_y1,
		*_z1,
		*_x2,
		*_y2,
		*_z2,
		*_amp,
		*_pow,

		*_ox,
		*_oy,
		*_oz,
		*_out;

	public:
	char *docString();
	CSmodule_DistFactor* newOne();
	void init();
	void step();
};




#endif // CSMODULE_MATH_H_INCLUDED
