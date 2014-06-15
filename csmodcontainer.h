/**
 *
 *  CS MODULAR EDITOR
 *  complete module handling class
 *
 *  @version 2010/09/22 v0.1 (created)
 *  @version 2011/03/24 container input/output module id name
 *  @author def.gsus-
 *
 *
 *
 **/

#ifndef CSMODCONTAINER_H_INCLUDED
#define CSMODCONTAINER_H_INCLUDED

#include <stdlib.h>

using namespace std;

#include "csmod_menu.h"
#include "csmodule.h"

/* need this to swap containers with CSmodule_Container */
#include "csmodule_container.h"



/** easy structure to hold arrays of modules */
struct CSmoduleList
{
	/** list of module pointers */
	CSmodule **m;
	int
	/** nr assigned */
		nr,
	/** nr allocated */
		nrAlloc;
};


// forward declaration (see CSmod.h)
class CSmod;



/**
	this class contains all gimmicks to
	handle (add/move/delete/connect/process) CSmodules
	*/
class CSmodContainer
{
	public:

	CSmod
		/** parent module GUI */
		*parent;

	CSmodContainer
		/** always points to the top-most container (CSmod->rootContainer),
			points to itself if top container */
		*rootContainer;

	int
		/** nr of contained modules */
		nr,
		/** nr of connections between modules */
		nrCon;

	CSmodule
		/** list of contained modules */
		**module;

	CSconnection
		/** list of connections */
		**con;

	CSmodule_Container
		/** if this container is within a container module */
		*containerModule;

	int
		/** the file version to store the container */
		fileVersion,
		/** the last read file version the container has restored from */
		readFileVersion;

	// ---- audio / processing -----

	bool
		/** this must be TRUE to enable audio calculation.
			also this MUST be set false, when changing the @see module[] list
			or stuff like this. */
		ready,

		/** true by default. if a @see CSmodule_Container is around this container,
			it can deactivate the container. */
		active;


	float
		/** the input ring buffer. during audio processing, the audio out stream
			is the driving thread to ask for the calculation of @see CSmod::bufferSizeOut samples,
			while the audio in thread will continously fill the inputSamples buffer.
			format is @param inputSamples[ @see CSmod::bufferSizeIn ][ @see CSmod::nrChannelsIn ] */
		*inputSamples,
		/** buffer for current output sample of audio out modules */
		outputSample[CSMOD_MAX_CHAN],

		/** array of pairs of input values that have to be updated */
		**valueUpdateList;
	int
		/** half length of @see valueUpdateList */
		nrValueUpdateList,
		/** allocated length of @see valueUpdateList */
		nrValueUpdateListAlloc;

	int
		/** size of the @see inputSamples buffer (in floats) */
		inputSampleSize,
		/** ring buffer write position in @see inputSamples */
		inputSamplePos,
		/** ring buffer read position in @see inputSamples */
		inputSampleReadPos;

	bool
		/** true when disk rendering (at least for root) */
		isRendering;

	// --- midi ----

	csfloat
		/** buffer of each midi note's velocity */
		noteOn[CSMOD_MAX_NOTE],
		/** last note velocity */
		lastNoteVelMono;
	int
		/** last note nr */
		lastNoteMono;
	bool
		/** true for one sample if monophonic note has changed */
		lastNoteMonoChanged;

	csfloat
		/** all controller values */
		controllerValue[CSMOD_MAX_CONTROLLER],
		/** all nrpn controller values */
		controller14Value[CSMOD_MAX_NRPN_CONTROLLER];

	int
		/** index in nrpn special control change sequence,
			-1 if waiting */
		nrpnSequence,
		/** to gather the nrpn controller number */
		nrpnNumber,
		/** to gather nrpn controller value */
		nrpnValue,
		/** last control change number */
		lastCCNumber;


	bool
		/** true when any midi string was received during last screen update */
		onMidiInChangeScr,
		/** true when notein happened during last screen update */
		onMidiNoteChangeScr,
		/** true when controlchange happened during last screen update */
		onMidiControlChangeScr
		;

	// ---- gui -------

	int
		/** view offset x (top of screen) */
		offx,
		/** view offset y (left of screen) */
		offy,
		/** actual used */
		offx_,
		/** actual used */
		offy_,
		/** view offset speed */
		offxs,
		/** view offset speed */
		offys,
		/** old view offset x  */
		offxo,
		/** old view offset y */
		offyo
		;
	float
		/** zoom factor (limited to be >= 0.1 ) */
		zoom,
		/** actual used */
		zoom_,
		/** start zoom factor (mousedown) */
		zoomo;

	int
		/** last mouse x (depends on action if this is screen or patch coordinates) */
		lmx,
		/** last mouse y (depends on action if this is screen or patch coordinates) */
		lmy,
		/** last mouse x (backup) */
		lmx1,
		/** last mouse y (backup) */
		lmy1,
		/** last mouse key ( @see mouseDown) */
		lmk,
		/** connection drag-to x */
		conx,
		/** connection drag-to y */
		cony;
	time_t
		/** last timer state on @see mouseDown (seconds) */
		lastTimeDown;
	bool
		/** mouse has moved since last @see mouseDown event */
		mouseHasMoved;

	CSmodule
		/** points to module if mouse is on header */
		*selHead,
		/** points to module if dragging a module */
		*dragHead,
		/** selected module */
		*selModule,
		/** module which has edit focus (mouse events) */
		*editModule,
		/** the module who's resize handle is in focus */
		*selResizeModule,
		/** the module which is currently resized with mouse */
		*resizeModule;

	CSmoduleConnector
		/** points to a connector if mouse in over */
		*selCon,
		/** start connector for cable connections */
		*dragCon,
		/** connector value in focus */
		*selValue,
		/** connector value that is changed with mouse */
		*dragValue,
		/** for multiple cable dragging */
		*dragBundle,
		/** current editing connector value (TODO) */
		*editValue;
	int
		/** 1 when dragging view offset, 2 for zooming, 0 otherwise */
		dragView;

	CSconnection
		/** cable in focus */
		*selCable;

	csfloat
		/** minimum value that passed the cable in focus */
		selCableMin,
		/** maximum value that passed the cable in focus */
		selCableMax;

	bool
		/** true when multiple modules are select. in that case
			the @see CSmodule::isSelected flag will be set for the modules */
		isMultiSelect,
		/** true when selecting an area */
		selArea,
		/** set the cables behind the modules */
		doCableBehind;

	int
		/** the last width that was passed to the @see draw() routine */
		curWidth,
		/** the last height that was passed to the @see draw() routine */
		curHeight,
		/** selection area start point */
		selAreaX0,
		/** selection area start point */
		selAreaY0,
		/** selection area top-left */
		selAreaX1,
		/** selection area top-left */
		selAreaY1,
		/** selection area bottom-right */
		selAreaX2,
		/** selection area bottom-right */
		selAreaY2;

	int
		/** cursor position for value edit */
		editCursor,
		/** relative x position of cursor */
		editCursorX;

	// -------- colors -----

	Fl_Color
		/** background color */
		backgroundColor,
		/** color of the line grid in background */
		backgroundGridColor,
		/** color when a containing module is inactive */
		backgroundInactiveColor,
		/** color of cables */
		connectionColor,
		/** color of connection that is in focus */
		connectionSelColor,
		/** color of cable that is dragged */
		connectionDragColor,
		/** color of the selection area frame */
		selAreaColor;

	// ---- midi stuff ------
	CSmodPopUp
		/** midi popup menu */
		*pu_midi;


	// ...........................

	/** constructor */
	CSmodContainer();

	/** destructor */
	~CSmodContainer();

	/** reset all fields / DONT CALL */
	virtual void clear();

	/** release alloc. memory / DONT CALL */
	virtual void release();

	//------------------------ load/save ---------------------------------------

	/** store all module connections within this container */
	void storeConnections(FILE *buf);

	/** store container and all of it's contents */
	void store(FILE *buf);

	/** store all module connections within this container */
	void restoreConnections(FILE *buf);

	/** load container contents from buf,
		halts program on error
		returns 0 if ok,
		returns 1 if this container is root but patch is sub-container and resets filepos */
	int restore(FILE *buf);

	//------------------------- module handling --------------------------------

	/** when a module is added, this function will setup some module parameters
		so it knows where it's contained.
		module will then be init()ed */
	void registerModule(CSmodule *m);

	/** add a NEW COPY of the supplied module @param m to the container, returns link to copy */
	CSmodule *addModule(CSmodule *m);

	/** reposition @param mod if it is on the same position as another module */
	void repositionModule(CSmodule *mod);

	/** change the name if it is already in this container */
	char* getUniqueName(const char *name);

	/** return index in module[] of module with ID @param name or -1 */
	int getModuleIndex(const char *name);

	/** return index in module[] of @param mod or -1 */
	int getModuleIndex(CSmodule *mod);

	/** delete a module from list and dealloc */
	void deleteModule(int index);

	/** create a duplicate of the module @param mod, which MUST BE a registered module. */
	CSmodule *duplicateModule(CSmodule *mod);

	/** wipe out anything */
	void deleteAll();

	/** returns the min and max extend of the container's modules */
	void getExtend(int *x1, int *y1, int *x2, int *y2);

	/** fill @see valueUpdateList[] with pairs of float pointers to update on each step */
	void getValueUpdateList();

	// --------------- module list handling ------------------------------------

	/** get a setup list to hold modules */
	CSmoduleList *getNewModuleList();

	/** free a module list */
	void freeModuleList(CSmoduleList *list);

	/** add a module to the list */
	void addModuleList(CSmoduleList *list, CSmodule *m);

	/** add a modulelist to the list */
	void addModuleList(CSmoduleList *list, CSmoduleList *listadd);

	// ------------------------- connection handling ---------------------------

	/** return a new CSconnection structure,
		returns NULL if the connection already exists! */
	CSconnection *createConnection(CSmoduleConnector *output, CSmoduleConnector *input);

	/** release any data AND free structure */
	void releaseConnection(CSconnection *conn);

	/** return the connection's index in list or -1 */
	int getConnectionIndex(CSconnection *conn);

	/** add a connection to list of connections */
	void addConnection(CSconnection *conn);

	/** delete a connection from list / free data */
	void deleteConnection(int index);

	/** connect the two connectors, doesnt matter if input/output or output/input.
		returns NULL if input/input or output/output !
		returns NULL if connection exists
		otherwise returns and adds connection to list. */
	CSconnection *connect(CSmoduleConnector *con1, CSmoduleConnector *con2);

	/** connect module's (id=mod1) output (id=con2) with module's (id=mod2) input (id=con2)
		returns NULL if one/some of the id names is not found
		returns NULL if input/input or output/output !
		returns NULL if connection exists
		otherwise returns and adds connection to list. */
	CSconnection *connect(const char *mod1, const char *con1, const char *mod2, const char *con2);

	/** this will connect con1 and con2 with already existing connection conn.
		if con1 and con2 are connected already, then nothing happens ;) */
	void reconnect(CSconnection *conn, CSmoduleConnector *con1, CSmoduleConnector *con2);

	/** delete all ingoing and outgoing connections from the module */
	void disconnectModule(CSmodule *mod);

	/** delete all outgoing / incoming connections from / to conn* */
	void disconnectConnector(CSmoduleConnector *conn);

	/** test if px,py (view offset) lies on a connection, return index or -1 */
	int isOnConnection(int px, int py);

	//-------------------------- value edit ------------------------------------

	/** go to next possible input connector, if selValue points to one.
		dir = 0-left, 1-up, 2-right, 3-down */
	void nextSelValue(int dir);

	/** start editing a value (input) */
	void startEdit(CSmoduleConnector *conn);

	/** set the uvalue field to the value in valueStr,
		update editCursorX, ... */
	void updateEditValue();

	/** insert char at current cursorpos in editValue */
	void editInsertChar(char c);

	/** delete char at current cursorpos in editValue */
	void editDeleteChar();

	//------------------------------- G U I ------------------------------------

	/** force redraw of screen */
	void refresh();

	/** set the actual zoom/offset values. 0 < @param fade < 1 sets smooth */
	void fadeView(float fade);

	/** try to set the viewport to include all modules */
	void setViewAll();

	/** pass mouse down events here, with position mx,my and key mk (FLTK constant).
		returns true if something changed */
	bool mouseDown(int mx, int my, int mk);

	/** pass mouse move events here, with position mx,my and key mk (FLTK constant)
		returns true if something changed */
	bool mouseMove(int mx, int my, int mk);

	/** pass mouse up events here, with position mx,my.
		returns true if something changed */
	bool mouseUp(int mx, int my);

	/** pass key events here (key = FLTK's Fl::event_key() ),
		returns true if something changed */
	bool keyDown(int key);

	//---------------------- multi-select handling -----------------------------

	/** multiselect the module @param m,
		when the module is multiselected then "de-multiselect" the module. */
	void multiSelect(CSmodule *m);

	/** multiselect (area flag) the current select area */
	void multiSelectArea();

	/** remove all @see isSelected flags from the modules, and set @see isMultiSelect to false */
	void unselect();

	/* ------
	   all following m_* functions work on a single module, or if the isMultiSelect flag is set,
	   on all selected modules */

	/** move module(s) by an offset */
	void m_move(CSmodule *m, int ofx, int ofy);

	/** delete the module(s) */
	void m_delete(CSmodule *m);

	/** duplicate the module(s) (with connections) */
	void m_duplicate(CSmodule *m);

	/** puts the module(s) into a new container and returns this container */
	CSmodContainer *m_contain(CSmodule *m);

	//-------------------------------- midi ------------------------------------

	void onMidiIn(DWORD data1, DWORD data2);

	/** simply reset everything */
	void midi_panic();

	void midi_noteOn(int noteNr, int velocity=CSMOD_MAX_VEL);

	void midi_controlChange(int controller, int value);

	void midi_control14Change(int controller, int value);

	//---------------------------- calculation ---------------------------------

	/** set a sampleRate for all modules and sub-modules */
	void setSampleRate(int newSampleRate);

	/** just go through all modules to see if a start gate module should be
		acknowledged. */
	void start();

	/** used by @see start() to recursively check subcontainers */
	void setStart(CSmodContainer *c);

	/** transport all values through the connections and ->step() all modules */
	void step();

	/** low level priority step() of modules */
	void stepLow();

	/** messures and prints out the calculation time consumed by every module / sub-container */
	void getCalculationTime();

	//----------------------------- GFX ----------------------------------------

	/** draw one connection */
	void drawConnection(CSconnection *con);

	/** update screen buffer */
	void draw(int width, int height);

	/** go (recursively) through container and all subcontainers and
		update the module's external windows, if any */
	void drawExternalWindows();

	/** return all modules wich have an external window */
	CSmoduleList *getExternalWindowModules();

};



#endif // CSMODCONTAINER_H_INCLUDED
