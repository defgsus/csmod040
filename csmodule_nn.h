/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: feed forward neuronal network
 *
 * PURPOSE:
 * feed-forward error-back-propagation neuronal network with variable
 * number of layers and cells..
 *
 * liquid state machine
 *
 * @version 2010/10/24
 * @version 2010/12/19 liquid state machine
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_NN_H_INCLUDED
#define CSMODULE_NN_H_INCLUDED

#include "csmodule.h"

#define CSMOD_NN_MAX 100
#define CSMOD_NN_MAX_IN 4
#define CSMOD_NN_MAX_OUT 4

class CSmodule_NN: public CSmodule
{
	csfloat
		*_rst,
		*_seed,
		*_ramp,
		*_lr,
		*_in[CSMOD_NN_MAX_IN],
		*_ine[CSMOD_NN_MAX_IN],
		*_des[CSMOD_NN_MAX_OUT],
		*_out[CSMOD_NN_MAX_OUT],
		*_err,

		lrst;

	CSmoduleConnector
		*__in[CSMOD_NN_MAX_IN],
		*__ine[CSMOD_NN_MAX_IN],
		*__des[CSMOD_NN_MAX_OUT],
		*__out[CSMOD_NN_MAX_OUT];

	// --- nnstuff ---

	bool
		/** to call initNet() in stepLow() */
		requestInit;

	int // actual used
		nrOut,
		nrIn,
		nrHidden,
		// previous used
		oldNrIn,
		oldNrOut,

		weightSeed,
		activation;

	csfloat
		/** hidden layer */
		cell[CSMOD_NN_MAX],
		/** inputs -> hidden layer [nrHidden][nrIn]*/
		weightIn[CSMOD_NN_MAX][CSMOD_NN_MAX_IN],
		/** hidden layer -> outputs [nrOut][nrHidden] */
		weightOut[CSMOD_NN_MAX_OUT][CSMOD_NN_MAX],
		/** input layer error */
		errorIn[CSMOD_NN_MAX_IN],
		/** hidden layer error */
		error[CSMOD_NN_MAX],
		/** output layer error */
		errorOut[CSMOD_NN_MAX_OUT];


	public:
	char *docString();
	CSmodule_NN *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrOut(int newNrOut);
	void setNrIn(int newNrIn);

	void step();

	// --  nnstuff  --

	/** request a new net */
	void initNet(int nrIn, int nrHid, int nrOut);

	void brainWash();

	void draw(int offx=0, int offy=0, float zoom=1.0);

};














#define CSMOD_LSM_MAX_WIDTH 4
#define CSMOD_LSM_MAX_HEIGHT 16
#define CSMOD_LSM_MAX (CSMOD_LSM_MAX_WIDTH * CSMOD_LSM_MAX_WIDTH * CSMOD_LSM_MAX_HEIGHT)

class CSmodule_NNLSM: public CSmodule
{
	csfloat
		*_rst,
		*_seed,
		*_rampl,
		*_ramp,
		*_gate,
		*_lr,
		*_in[CSMOD_NN_MAX_IN],
		*_des[CSMOD_NN_MAX_OUT],
		*_out[CSMOD_NN_MAX_OUT],
		*_err,

		lrst,
		lgate;

	CSmoduleConnector
		*__gate,
		*__in[CSMOD_NN_MAX_IN],
		*__des[CSMOD_NN_MAX_OUT],
		*__out[CSMOD_NN_MAX_OUT];

	// --- nnstuff ---

	csfloat
		connectRad,
		filterPower;

	int // actual used
		nrOut,
		nrIn,
		nrInCon,
		nrHid,
		nrHidW,nrHidH,
		nrHidCon,
		nrFB,
		// previous used
		oldNrIn,
		oldNrOut,

		activation;

	csfloat
		/** hidden layer */
		cell[CSMOD_LSM_MAX],
		/** temp hidden layer */
		celltemp[CSMOD_LSM_MAX],
		/** lowpass coefficient */
		celllp[CSMOD_LSM_MAX],
		/** inputs -> hidden layer */
		weightIn[CSMOD_LSM_MAX],
		/** hidden layer -> outputs [nrOut][nrHidden] */
		weightOut[CSMOD_NN_MAX_OUT][CSMOD_LSM_MAX],
		/** hidden layer -> hidden layer */
		weightHid[CSMOD_LSM_MAX],
		/** output layer error */
		errorOut[CSMOD_NN_MAX_OUT];


	csfloat
		/** inputs -> hidden / index of input */
		*weightInIndex[CSMOD_LSM_MAX],
		/** inputs -> hidden / index of hidden cell */
		*weightInHidIndex[CSMOD_LSM_MAX],
		/** hidden -> hidden index */
		*weightHidFromIndex[CSMOD_LSM_MAX],
		/** hidden -> hidden index */
		*weightHidToIndex[CSMOD_LSM_MAX];


	public:
	char *docString();
	CSmodule_NNLSM *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrOut(int newNrOut);
	void setNrIn(int newNrIn);

	void step();

	// --  nnstuff  --

	void initNet(int nrIn, int nrInCon, int nrHidW, int nrHidH, int nrHidCon, int nrOut);

	void brainWash();

	void draw(int offx=0, int offy=0, float zoom=1.0);

};






#define CSMOD_SOM_MAX_WIDTH_SHIFT 7 // 64
#define CSMOD_SOM_MAX_WIDTH (1<<CSMOD_SOM_MAX_WIDTH_SHIFT)
#define CSMOD_SOM_MAX_PARAM_SHIFT 5 // 16
#define CSMOD_SOM_MAX_PARAM (1<<CSMOD_SOM_MAX_PARAM_SHIFT)

class CSmodule_SOM: public CSmodule
{
	csfloat
		*_clr,
		*_seed,
		*_ramp,

		*_gateStore,
		*_opac,
		*_rad,
		*_gateRead,
		*_x,
		*_y,
		*_ox,
		*_oy,
		*_gateMatch,
		*_p_in[CSMOD_SOM_MAX_PARAM],
		*_p_out[CSMOD_SOM_MAX_PARAM],

		lclr,
		lgateStore,
		lgateMatch,
		lgateRead;

	CSmoduleConnector
		*__gate,
		*__p_in[CSMOD_SOM_MAX_PARAM],
		*__p_out[CSMOD_SOM_MAX_PARAM];

	// --- nnstuff ---

	int
		nrParam,
		nrX,
		nrY,

		sx,sy,sw,sh;

	csfloat
		// the map (y*x*param)
		cell[((1<<CSMOD_SOM_MAX_WIDTH_SHIFT)<<CSMOD_SOM_MAX_WIDTH_SHIFT)<<CSMOD_SOM_MAX_PARAM_SHIFT],
		// precalculated radius amplitude (y*x*rad)
		radamp[CSMOD_SOM_MAX_WIDTH*CSMOD_SOM_MAX_WIDTH*CSMOD_SOM_MAX_WIDTH];
		;

	public:
	char *docString();
	CSmodule_SOM *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	void onResize();

	void setNrParam(int newNr);

	void step();

	void draw(int offx=0, int offy=0, float zoom=1.0);

	// --  nnstuff  --

	void brainWash();
	/** return position x,y of best matching cell for set of @param param */
	void bestMatch(csfloat **param, int *x, int *y);

};















#endif // CSMODULE_NN_H_INCLUDED
