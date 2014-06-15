/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Controls
 *
 * PURPOSE: various control modules like switsches and stuff
 *
 * @version 2011/03/09 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_CONTROL_H_INCLUDED
#define CSMODULE_CONTROL_H_INCLUDED

#include <FL/Fl.H>
#include <FL/Enumerations.H>

#include "csmodule.h"

#define CSMOD_SWITCH_MAX 32
#define CSMOD_SWITCH_MAX_BANK 256

class CSmodule_Switch: public CSmodule
{
	public:
	csfloat
		*_on,
		*_off,
		*_gate,
		lgate,
		*_out;

	int
		sx,sy,sw,sh,
		on,
		mode,
		wasScreenUpdate;

	char *docString();
	CSmodule_Switch *newOne();

	void copyFrom(CSmodule *mod);
	void storeAdd(FILE *buf);
	void restoreAdd(FILE *buf);

	void init();
	void propertyCallback(CSmodProperty *prop);

	void step();

	void draw(int offx=0, int offy=0, float zoom=1.0);
	bool mouseDown(int mx, int my, int mk);

};


class CSmodule_SwitchGate: public CSmodule_Switch
{
	CSmodule_SwitchGate *newOne();
	void init();
};





class CSmodule_SwitchMatrix: public CSmodule
{
	public:
	csfloat
		*_bankRead,
		*_bankWrite,
		*_in[CSMOD_SWITCH_MAX],
		*_out[CSMOD_SWITCH_MAX],
		o[CSMOD_SWITCH_MAX],

		state[CSMOD_SWITCH_MAX_BANK][CSMOD_SWITCH_MAX][CSMOD_SWITCH_MAX];

	CSmoduleConnector
		*__in[CSMOD_SWITCH_MAX],
		*__out[CSMOD_SWITCH_MAX];

	int
		curReadBank,
		curWriteBank,

		sx,sy,sw,sh,sz,
		offx,offy,
		nrIn, nrOut,
		singleIn, singleOut,
		useFloat,
		editX, editY,
		lastY,
		doFadeBank;
	bool
		bankUsed[CSMOD_SWITCH_MAX_BANK],
		doValDrag;

	char *docString();
	CSmodule_SwitchMatrix *newOne();

	void copyFrom(CSmodule *mod);
	void storeAdd(FILE *buf);
	void restoreAdd(FILE *buf);

	void init();
	void propertyCallback(CSmodProperty *prop);
	void setNrIn(int newNrIn);
	void setNrOut(int newNrOut);

	void setState(int x,int y, csfloat stat);
	void checkStates();
	void resetStates(int bankNr=-1);

	void step();

	void draw(int offx=0, int offy=0, float zoom=1.0);
	bool mouseDown(int mx, int my, int mk);
	bool mouseMove(int mx, int my, int mk);
	bool mouseUp(int mx, int my);

};



#endif // CSMODULE_CONTROL_H_INCLUDED
