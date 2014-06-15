/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Sequencer / Timeline
 *
 * PURPOSE:
 * output painted sequence
 *
 * @version 2010/11/07 v0.1
 * @version 2010/11/09 v0.2 banks
 * @version 2010/11/24 v0.3 _Timeline
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_SEQ_H_INCLUDED
#define CSMODULE_SEQ_H_INCLUDED

#include "csmodule.h"

/** maximum allowed sequencer steps */
#define CSMOD_SEQ_MAX 256
/** maximum allowed sequencer banks */
#define CSMOD_SEQ_MAX_BANK 256

/** minimum allocated timeline points */
#define CSMOD_TIMELINE_MIN 256

class CSmodule_Seq: public CSmodule
{
	public:

	csfloat
		*_steps,
		*_rst,
		*_step,
		*_amp,
		*_off,
		*_bank,
		*_bankedit,
		*_out,
		*_outG,

		lrst,
		lstep,

		data[CSMOD_SEQ_MAX * CSMOD_SEQ_MAX_BANK];

	int
		pos,
		sx,sy,sw,sh,
		drawing;

	bool
		sentGate,
		bankChanged[CSMOD_SEQ_MAX_BANK];

	char
		*valueStr;

	public:
	char *docString();
	~CSmodule_Seq();
	CSmodule_Seq *newOne();
	void copyFrom(CSmodule *mod);

	void storeAdd(FILE *buf);
	void restoreAdd(FILE *buf);

	void init();

	void step();

	bool mouseDown(int mx, int my, int mk);
	bool mouseMove(int mx, int my, int mk);

	void draw(int offx=0, int offy=0, float zoom=1.0);

};


#define CS_TL_MAXWINDOW 8

class CSmodule_Timeline: public CSmodule
{
	public:

	csfloat
		*_pos,
		*_amp,
		*_off,
		*_smth,
		*_out,
		*_outL,
		*_outS,
		*_outC,

		*_outFirst,
		*_outLast;

	CSmoduleConnector
		*outL,
		*outS,
		*outC;

	int
		// background window
		sx,sy,sw,sh;
	float
		// view offset in timeline
		voffx,voffy,
		// old view offset
		voffxo, voffyo,
		// zoom in timeline view
		vzoomx,vzoomy,
		// old..
		vzoomxo,vzoomyo,

		// different view windows
		voffx_[CS_TL_MAXWINDOW],
		voffy_[CS_TL_MAXWINDOW],
		vzoomx_[CS_TL_MAXWINDOW],
		vzoomy_[CS_TL_MAXWINDOW];
	int
		// selected window
		selWindow,
		// display mode (curve)
		viewMode,

		// last mouse pos
		lmx,lmy;
	float
		// last real mouse pos
		lmxr,lmyr;
	int
		editMode,
		focusPoint;

	bool doLoop;

	char
		valueStr[64];

	csfloat
		*X, *Y; // xy positions of points
	int nrp, // nr of points
		nrpa; // nr allocated points

	public:
	char *docString();
	~CSmodule_Timeline();
	CSmodule_Timeline *newOne();
	void copyFrom(CSmodule *mod);

	void storeAdd(FILE *buf);
	void restoreAdd(FILE *buf);

	void onResize();

	void init();

	void propertyCallback(CSmodProperty *prop);

	void step();

	void setWindow(int newWindow, bool doStore = true);

	bool mouseDown(int mx, int my, int mk);
	bool mouseMove(int mx, int my, int mk);
	bool mouseUp(int mx, int my);

	void draw(int offx=0, int offy=0, float zoom=1.0);

	// get position in timeline view
	int getVX(csfloat x);
	int getVY(csfloat y);

	// transform position-in-timeline-view to real coordinates
	csfloat getRX(int x);
	csfloat getRY(int y);

	int getClosestPoint(csfloat x);
	int addPoint(csfloat x, csfloat y);
	void deletePoint(int k);

	csfloat getValue(csfloat x);
};



#endif // CSMODULE_SEQ_H_INCLUDED
