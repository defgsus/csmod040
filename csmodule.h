/**
 *
 *  CS MODULAR EDITOR module declaration
 *
 *  @version 2010/09/21 v0.1 (created)
 *  @version 2011/03/24 container input/output module id name (confilever 4)
 *  @author def.gsus-
 *
 *
 *
 **/

#ifndef CSMODULE_H_INCLUDED
#define CSMODULE_H_INCLUDED


typedef float csfloat;

/** libpng 1.4.4 (or lower or higher :) */
#include <png.h>

#include <stdio.h>

using namespace std;

#include "FL/Fl.H"
#include "FL/x.H"
#include "FL/gl.h"
#include "FL/fl_draw.H"
#include "FL/Fl_Window.H"


/** file version for writing containers/modules */
#define CSMOD_CONTAINER_FILEVERSION 4

/** maximum allowed channels (inputs/outputs/mixers/and such).
	this number can be adjusted to your needs. no prob. */
#define CSMOD_MAX_CHAN 32

/** initial external window size */
#define CSMOD_INIT_EXTWIN_WIDTH 512
/** initial external window size */
#define CSMOD_INIT_EXTWIN_HEIGHT 512

// midi stuff
#define CSMOD_MAX_NOTE 128
#define CSMOD_MAX_VEL 127
#define CSMOD_MAX_CONTROLLER 128
#define CSMOD_MAX_NRPN_CONTROLLER 16384

#define CSMOD_MIDI_MAX_POLY 128


// ------------------------------------- small helper -----------------------------------------------------

// at least the win calc says so...
#define HALF_PI 1.5707963267948966192313216916398
#define TWOTHIRD_PI 2.0943951023931954923084289221863
#define PI 3.1415926535897932384626433832795
#define TWO_PI 6.283185307179586476925286766559
#define SQRT_2 1.4142135623730950488016887242097
#define TANH_1 0.761594
#define TANH_1_I 1.313035

/** create a new allocated string copy */
char *copyString(const char *str);

/** return new allocated string consisting of str + suffix */
char *addString(const char *str, const char *suffix);

/** convert int to char */
char *int2char(int i);

/** convert int to char, with prefix */
char *int2char(const char* prefix, int i);

/** read the following string, skip whitespace, check also for surrounding ""s */
char *readString(FILE *buf);

/** current *state-of-the-art* error handling */
#define CSERROR(string) \
	{ printf("ERROR: %s\n", (string)); exit(1); }

/** removes the denormals, infinites, and NaNs from a 32 bit float,
	gotten from musicdsp.org */
#define CSclipFloat(F) \
		unsigned int *V = (unsigned int*)&(F); \
		unsigned int sample = *V; \
		unsigned int exponent = sample & 0x7F800000;\
        int aNaN = exponent < 0x7F800000; \
        int aDen = exponent > 0; \
        *V = sample * ( aNaN & aDen ); \
/** removes the denormals, infinites, and NaNs from a 32 bit float,
	gotten from musicdsp.org */
#define CSclipFloatAgain(F) \
		V = (unsigned int*)&(F); \
		sample = *V; \
		exponent = sample & 0x7F800000;\
        aNaN = exponent < 0x7F800000; \
        aDen = exponent > 0; \
        *V = sample * ( aNaN & aDen ); \

/** just a wrapper to assign and int with the FLTK color code */
#define CSsetColor(col, red,green,blue) col = fl_rgb_color((red), (green), (blue))

/** make @param colB the scaled version of @param col, col and colB should be int,
	@param amp can be float. */
#define CSmulColor(col, colB, amp) colB = fl_rgb_color( \
	max(0,min(255, (int)((((col)>>24) & 0xff)*amp) )), \
	max(0,min(255, (int)((((col)>>16) & 0xff)*amp) )), \
	max(0,min(255, (int)((((col)>>8) & 0xff)*amp) )) )

/** make @param colB the scaled version of @param col, col and colB should be int,
	@param ampR, @param ampG and @param ampB can be float. */
#define CSmulColorRGB(col, colB, ampR, ampG, ampB) colB = fl_rgb_color( \
	max(0,min(255, (int)((((col)>>24) & 0xff)*ampR) )), \
	max(0,min(255, (int)((((col)>>16) & 0xff)*ampG) )), \
	max(0,min(255, (int)((((col)>>8) & 0xff)*ampB) )) )

/** a MODULO for floats */
#define MODULO(A,B) \
	fmodf(A, B)
//	( ( ((A)/(B)) - (int)((A)/(B)) ) * B )

/** test if point x,y is within a box */
#define CS_POINT_IN_BOX(x,y, x1,y1,x2,y2) \
	( ((x)>=x1) && ((x)<=x2) && ((y)>=y1) && ((y)<=y2) )

/** test if the first box overlaps with the second box */
#define CS_BOX_IN_BOX2(x1a,y1a,x2a,y2a, x1b,y1b,x2b,y2b) \
	( CS_POINT_IN_BOX(x1a, y1a,  x1b, y1b, x2b, y2b) || \
	  CS_POINT_IN_BOX(x2a, y1a,  x1b, y1b, x2b, y2b) || \
	  CS_POINT_IN_BOX(x1a, y2a,  x1b, y1b, x2b, y2b) || \
	  CS_POINT_IN_BOX(x2a, y2a,  x1b, y1b, x2b, y2b) || \
	  \
	  ( ((x1b)>=(x1a))&&((x1b)<=(x2a))&&((y1b)<(y1a))&&((y2b)>(y2a)) ) || \
	  ( ((y1b)>=(y1a))&&((y1b)<=(y2a))&&((x1b)<(x1a))&&((x2b)>(x2a)) ) )

/** test if two boxes overlap */
#define CS_BOX_IN_BOX(x1a,y1a,x2a,y2a, x1b,y1b,x2b,y2b) \
	( CS_BOX_IN_BOX2(x1a, y1a, x2a, y2a,  x1b, y1b, x2b, y2b) || \
	  CS_BOX_IN_BOX2(x1b, y1b, x2b, y2b,  x1a, y1a, x2a, y2a) )


#define CSdist0(x,y,z) \
	sqrt( (x)*(x)+(y)*(y)+(z)*(z) )

#define CSdist(x1,y1,z1, x2,y2,z2) \
	sqrt( ((x2)-(x1))*((x2)-(x1)) + \
		  ((y2)-(y1))*((y2)-(y1)) + \
		  ((z2)-(z1))*((z2)-(z1)) )\

#define CSdistSqr(x1,y1,z1, x2,y2,z2) \
	((x2)-(x1))*((x2)-(x1)) + \
	((y2)-(y1))*((y2)-(y1)) + \
	((z2)-(z1))*((z2)-(z1)) \


/** TODO !!! wait for calculation to finish */
#define CSwait() \
	if (isStep) { \
		printf("CALLBACK CONFLICT!! please repeat your wish..\n"); \
		return; \
	}
/*
no, this thing wont work. it waits forever
#define CSwait() \
	if (isStep) { \
		printf("waiting for audio callback\n"); \
		for (;isStep;); \
		printf("ok\n"); \
	}
*/
// -------------------------------- CSmoduleWindow --------------------------------------------------------

// forward declaration, see below
class CSmodule;


/** an external window used by @see CSmodule */
class CSmoduleExtWindow : public Fl_Window
{
	CSmodule
		/** the module this window belongs to */
		*parent;

		/** tracks old window position to only pass resize events in @see resize() */
	int oldx,oldy,oldw,oldh;

	bool
		/** true after full setup of window */
		ready;

	public:
	/** constructor */
	CSmoduleExtWindow(CSmodule *parent);

	/** overrides the FLTK draw method, and calls @see CSmodule::drawWindow */
	void draw();

//	/** calls @see draw() and stores a png file */
//	void drawAndSave(const char *filename);

	/** used to pass the resize event to the module */
	void resize(int x,int y,int w,int h);

};


// ----------------------------------- CSmodule -----------------------------------------------------------


/** connector directions for @see CSmoduleConnector */
enum
{
	/** default audio signal */
	CS_CONDIR_IN,
	/** fake scope signal */
	CS_CONDIR_OUT
};

/** connector types for @see CSmoduleConnector */
enum
{
	/** default audio signal */
	CS_CON_AUDIO,
	/** fake scope signal */
	CS_CON_SCOPE
};

/** general purpose module connector, used for both inputs and outputs.
	for inputs: this is also a parameter, since any parameter can be controlled */
struct CSmoduleConnector
{
	CSmodule
		/** parent module this connector belongs to */
		*parent;

	int
		/** direction of connector CS_CON_IN/OUT */
		dir,
		/** type of connector */
		typ,
		/** nr of ingoing / outgoing cables
			(updated by @see CSmodContainer) */
		nrCon,
		/** relative x position */
		x,
		/** relative y position */
		y;

	char
		/** (ID) name of the input (must never change for persistence) */
		*name,
		/** user name of this connector (free choosable) */
		*uname,
		/** a string with the current user value (inputs) or value (outputs) */
		*valueStr,
		/** for inputs: a string with the current user value + a cursor char */
		*valueStrE;

	csfloat
		/** the actual value of that input/output */
		value,
		/** for inputs: user defined value (offset) */
		uvalue,
		/** for inputs: default value (offset) */
		defaultValue;

	bool
		/** active, true by default */
		active,
		/** automatically arrange connector, true by default */
		autoArrange,
		/** display name and value, true by default */
		showLabel;

};


/** a connection between two CSmoduleConnectors */
struct CSconnection
{
	CSmoduleConnector
		/** connector from which this connection comes from */
		*from,
		/** connector to which this connection goes to */
		*to;

	bool
		/** signifies if connection should be used by audio thread */
		ready;
};


/** possible property types (see @see CSmodProperty) */
enum
{
	/** an int value */
	CS_PROP_INT,
	/** csfloat value */
	CS_PROP_FLOAT,
	/** char* */
	CS_PROP_STRING
};


/** modules can have additional (non-audio-update-time) parameters which
	are then controlled from the property view ( @see CSpropertyWindow in @see CSmod.h)  */
struct CSmodProperty
{
	char
		/** ID name, this should not change for persitence of saving/loading */
		*name,
		/** name of the property */
		*uname;

	int
		/** the type of the property */
		typ;

	/** int value */
	int	ivalue;
	/** float value */
	csfloat fvalue;
	/** string */
	char *svalue;

	csfloat
		/** value lower range */
		minValue,
		/** value high range */
		maxValue;

	CSmodule
		/** parent module */
		*parent;
};


/** structure used to record data over a period of time */
struct CStimeline
{
	int
		/** length of the timeline */
		length,
		/** nr of samples to skip when recording */
		skip,
		/** skip counter */
		skipc,
		/** current write pointer */
		pos;
	csfloat
		/** data buffer */
		*data;
};






// forward declaration for the module container (defined in csmodcontainer.h)
class CSmodContainer;

#define CSMOD_ICONSIZE 17

/**
  * base class for all kinds of modules
  *
  *
  * @note must overrides for derrived modules:
  *   init() - to setup your specific parameters/in-outputs/properties
  *            MUST SET: bname, name and uname
  *			   it's also a good idea to call createNameProperty() to make
  *            your module's username editable
  *
  *   newOne() - create a new, clean instantiation of your derrived class
  *
  * @note should overrides:
  *   ~destructor - if you alloc any additional memory in init()
  *	  step() - calculate whatever the module must calculate (one sample)
  *   stepLow() - do things here (systemcalls or whatever) that should not
  *               be done in the audio thread.
  *
  * @note can overrides:
  *   registerInputs() - called any time the input list is changed
  *                      to let you track variable pointers and stuff.
  *						 this is only nescessary if your module can change
  *						 nr of inputs and outputs at runtime.
  *   copyFrom() - if your module has certain additional parameters that
  *				   are not covered by the properties.
  *   storeAdd()
  *   restoreAdd() - same as for copyFrom() / to store/load additional parameter to/from file
  *
  */
class CSmodule
{

	public:

	char icon[CSMOD_ICONSIZE*CSMOD_ICONSIZE+1];

	CSmodContainer
		/** the parent CSmodContainer */
		*parent,
		/** the parent root CSmodContainer (top one) */
		*parentRoot;

	bool
		/** determines if this module should be included in
			calculation. the background audio thread could lead
			into trouble while initializing the module. */
		ready;

	char
		/** module's base name (class name) */
		*bname,
		/** module's name as ID (possibly suffixed at runtime) */
		*name,
		/** freely user given name */
		*uname,
		/** a group name, doesn't have to be set */
		*gname;

	int
		/** nr of module's connectors */
		nrCon;

	CSmoduleConnector
		/** list of input/output connectors */
		**con;

	int
		/** nr of properties of the module */
		nrProp;

	CSmodProperty
		/** list of properties */
		**property;

	csfloat
		/** the samplerate as defined in CSmod */
		sampleRate,
		/** the reciprocal of the samplerate as defined in CSmod */
		isampleRate,
		/** this value will be 44100 / sampleRate */
		normSampleRate;

	bool
		/** for multiselection */
		isSelected,
		/** for area selection */
		isSelectedArea,
		/** signifies if the module can be added to a container
			from the installedModules[] list. true by default */
		canAdd,
		/** this will be TRUE during the @see step() function. */
		isStep,
		/** true when module can be resized with mouse */
		canResize;

	int
		/** use this for backwards compatibility when storing/restoring modules.
			(see also: readFileVersion)
			default is 1 */
		fileVersion,
		/** this holds the fileVersion which was read by restore() */
		readFileVersion;

	// ----- GUI stuff -----

	int
		/** x position of module */
		x,
		/** y position of module */
		y,
		/** minimum width of module, set in init() */
		minWidth,
		/** minimum height of module */
		minHeight,
		/** actual minimum height of module, set in init(), adjusted by arrangeInputs()/arrangeOutputs() */
		minHeight1,
		/** width of module */
		width,
		/** height of module (including header) */
		height,
		/** height of header in pixels */
		headerHeight,
		/** horizontal offset between header and first input */
		inputStart,
		/** horizontal pixels between two inputs */
		inputHeight,
		/** horizontal offset between header and first output */
		outputStart,
		/** horizontal pixels between two outputs */
		outputHeight,
		/** radius of connectors */
		connectorRad,
		/** radius of focused connectors */
		connectorFocusRad;

	Fl_Color
		/** rgb color of body */
		bodyColor,
		/** rgb color of body (bright) */
		bodyColorB,
		/** rgb color of body (dark) */
		bodyColorD,
		/** rgb color of body (selected module) */
		bodySelColor,
		/** rgb color of header */
		headerColor,
		/** rgb color of header (bright) */
		headerColorB,
		/** rgb color of header (dark) */
		headerColorD,
		/** rgb color of selected header */
		headerSelColor,
		/** rgb color if header is in focus */
		headerFocusColor,
		/** rgb color if header is in focus (bright) */
		headerFocusColorB,
		/** rgb color if header is in focus (dark) */
		headerFocusColorD,
		/** rgb color of header font */
		headerFontColor,
		/** color of input / output connection handles */
		connectorColor,
		/** color of input / output connection handles when in focus */
		connectorFocusColor,
		/** color of input / output connection handles when not connected */
		connectorZeroColor,
		/** color of values and labels */
		labelColor,
		/** color of values and labels which are in focus */
		labelFocusColor,
		/** when editing values (inputs) */
		labelEditColor;


	CSmoduleExtWindow
		/** this can be a separate window, see @see createWindow() and @see drawWindow() */
		*window;

	bool
		/** questioned by @see CSmodContainer::drawExternalWindows() if the @see window should
			be drawn. true by default */
		doDrawWindow,
		/** questioned by @see CSmodContainer::drawExternalWindows().
			if the @see window should NOT be drawn, but the @see drawWindow() procedure
			should be called still. false by default */
		doCallDrawWindow;

	// -------- methods -----------

	/** constructor */
	CSmodule();

	/** destructor */
	virtual ~CSmodule();

	/** reset all fields / DONT CALL */
	virtual void clear();

	/** release alloc. memory / DONT CALL
		@bug it seems that an overridden release() method will not be
			called from the original destructor. don't know what that shit means.
			so it seems to be better to override the destructor instead! */
	virtual void release();

	/** set your module's individual properties here */
	virtual void init() = 0;

	/** here you should return a new instantiation of your module class */
	virtual CSmodule* newOne() = 0;

	/** copy the default settings from the module @param mod as
		username, properties and input values.
		the copyFrom() method will be called AFTER the modules init() method,
		means the module has been registered already (added to a @see CSmodContainer).
		you might want to extend the method to care for special module parameters */
	virtual void copyFrom(CSmodule *mod);

	//------------------ load/save ---------------------------------------------

	/** store the input user values */
	void storeValues(FILE *buf);

	/** store the list of properties to buf */
	void storeProperties(FILE *buf);

	/** store the module as text */
	void store(FILE *buf);

	/** store basic connector settings (only used with @see CSmodule_Container) */
	void storeConnectors(FILE *buf);

	/** store additional data of your derrived modules before the value block */
	virtual void storeAddBeforeValue(FILE *buf);

	/** store additional data of your derrived modules */
	virtual void storeAdd(FILE *buf);

	/** restore input user values */
	void restoreValues(FILE *buf);

	/** restore module properties from buf */
	void restoreProperties(FILE *buf);

	/** restore module settings from buf */
	void restore(FILE *buf);

	/** create connectors from file info (only used with @see CSmodule_Container) */
	void restoreConnectors(FILE *buf);

	/** restore additional data for your derrived modules before the value block */
	virtual void restoreAddBeforeValue(FILE *buf);

	/** restore additional data for your derrived modules */
	virtual void restoreAdd(FILE *buf);



	//------------------- properties -------------------------------------------

	/** return a documentation for the module here */
	virtual char *docString();

	/** create a new property and add to list,
		on change in CSpropertyWindow, the propertyCallback() function of the module
		will be called */
	CSmodProperty *createProperty(int typ, const char *name, const char *uname);

	/** create an integer type property (see also createProperty()) */
	CSmodProperty *createPropertyInt(const char *name, const char *uname,
						int theValue=0, int minValue=-0xfffffff, int maxValue=0xfffffff);

	/** create a csfloat type property (see also createProperty()) */
	CSmodProperty *createPropertyFloat(const char *name, const char *uname,
						csfloat theValue=0, csfloat minValue=-0xfffffff, csfloat maxValue=0xfffffff);

	/** create a string type property (see also createProperty()) */
	CSmodProperty *createPropertyString(const char *name, const char *uname, const char *theValue);

	/** find a property's index in list by it's id name, or -1 if not found */
	int getPropertyIndex(const char *name);

	/** default property for changing the modules name */
	CSmodProperty *createNameProperty();

	/** release data AND release structure */
	void releaseProperty(CSmodProperty *prop);

	/** add a property to the list (automatically done by createProperty() ) */
	void addProperty(CSmodProperty *prop);

	/** default property change callback function.
		NOTE! it's adviseable to call propertyCallback of the base class (CSmodule)
		somewhere in your implementation, since it checks for module name changes. */
	virtual void propertyCallback(CSmodProperty *prop);

	/** change @see uname and the modname property */
	void rename(const char *newName);

	//------------------- connectors -------------------------------------------

	/** return a new char* with a name that is unique among connectors */
	char *uniqueConnectorName(const char *name);

	/** create a CSmoduleConnector structure */
	CSmoduleConnector *createConnector(int dir, int x,int y, const char *name, const char *uname, csfloat defaultValue);

	/** create a CSmoduleConnector structure as input, and add to list. */
	CSmoduleConnector *createInput(const char *name, const char *uname, csfloat defaultValue = 0.0);

	/** create a CSmoduleConnector structure as input, and insert at list[ @param index ]. */
	CSmoduleConnector *insertInput(int index, const char *name, const char *uname, csfloat defaultValue = 0.0);

	/** create a CSmoduleConnector structure as output, and add to list,
		if @param insertIndex >= 0 then insert the connector rather than append.  */
	CSmoduleConnector *createOutput(const char *name, const char *uname);

	/** releases contents AND connection itself */
	void releaseConnector(CSmoduleConnector *con);

	/** you should override this function to account for any changes
		of the connector list */
	virtual void registerConnectors();

	/** this will normally automatically set the connection positions,
		should be called by registerConnectors() */
	virtual void arrangeConnectors();

	/** count nr of input connectors */
	int nrInputs();

	/** count nr of output connectors */
	int nrOutputs();

	/** find the first connector whos ID is name, return index or -1 */
	int getConnectorIndex(const char *name);

	/** return index of the connector or -1 */
	int getConnectorIndex(CSmoduleConnector *inp);

	/** append and register an connector */
	void addConnector(CSmoduleConnector *inp);

	/** insert and register an connector in @see con[ @param index ]*/
	void insertConnector(CSmoduleConnector *inp, int index);

	/** remove connector from list and free the structure */
	void deleteConnector(int index);

	/** update the valueStr */
	void updateConnector(CSmoduleConnector *con);

	/** for inputs, change the uservalue */
	void setConnectorValue(CSmoduleConnector *con, csfloat newValue);

	/** set active or not active. if not active connections will be thrown away!! */
	void setConnectorActive(CSmoduleConnector *con, bool act);

	/** return a list of pointers to connectors that are connected to @param co.
		the list has to be initialized to the nr of connections (co->nrCon).
		returns the number of list entries. */
	int getConnectedConnectors(CSmoduleConnector *co, CSmoduleConnector **conlist);

	// -------------------------- timeline -------------------------------------

	/** return an allocate CStimeline structure to record data,
		with certain @param length and @param skip skips between input samples.
		*/
	CStimeline *createTimeline(int length, int skip=0);

	/** remove data and free CStimeline struct */
	void releaseTimeline(CStimeline *timeline);

	/** record a value (also count skips) into a CStimeline struct */
	void recordTimeline(CStimeline *tl, csfloat value);

	// ----------------------------- GUI ---------------------------------------

	/** force CSmodContainer to update screen */
	void refresh();

	/** this will be called when the module size changes */
	virtual void onResize();

	/** set new position */
	void setPos(int newx, int newy);

	/** move position relative */
	void move(int offx, int offy);

	/** see if position is on header */
	bool isOnHeader(int px, int py);

	/** returns -1 or nr of connector px,py is on */
	int isOnConnector(int px, int py);

	/** returns -1 or nr of connector on which value px,py is on */
	int isOnValue(int px, int py);

	/** test if px,py is within module boundaries */
	bool isOnModule(int px,int py);

	/** test if px,py is on resize handle */
	bool isOnResize(int px,int py);

	/** return true if the module is (completely) within the box defined by x1,y1 - x2,y2 */
	bool isInBox(int x1,int y1, int x2, int y2);

	/** pass mouse down events here, with position mx,my and key mk (FLTK constants).
		return true if something changed */
	virtual bool mouseDown(int mx, int my, int mk);

	/** pass mouse move events here, with position mx,my and key mk (FLTK constant)
		return true if something changed */
	virtual bool mouseMove(int mx, int my, int mk);

	/** pass mouse up events here, with position mx,my.
		return true if something changed */
	virtual bool mouseUp(int mx, int my);


	// -------------------------- external windows ----------------------------------

	/** create a separate user-resizeable display window in @see window. the drawing update should be
		done in @see drawWindow(). the window will be automatically destroy be the
		CSmodule destructor. */
	void createWindow();

	/** if created a separate window with @see createWindow(), this method will be called
		when screen update is desired, and the 'FLTK current window' will be set to
		the modules @see window on function entry, so you can just start drawing. */
	virtual void drawWindow();

	/** if created a separate window with @see createWindow(), this method will be called
		on graphics render-on-disk update. there is NO FLTK drawing context. you got to
		paint and store your internal buffer!! */
	virtual void drawAndSaveWindow(const char *filename);

	/** store the image buffer @param img (R,G,B) to a file. @param spacing is the number
		of bytes per pixel (default=3, can be 3 or 4). */
	void storePNG(const char *filename, unsigned char *img, int w, int h, int spacing=3);

	/** this method will be called whenever the user changes the size of the @see window. */
	virtual void onResizeWindow(int newW, int newH);

	/** store the contents of the window as PNG */
	void storeWindow(const char *filename);

	// --------------------- interaction with container ------------------------

	/** set/reset the sampleRate of the module.
		override this, if you have to recalculate some coefficients or stuff..,
		but don't forget to call the original setSampleRate() to calculate
		@see isampleRate and @see normSampleRate as well. */
	virtual void setSampleRate(int sr);

	/** called when the module is added to a container */
	virtual void onAddModule();

	// ---------------------- calculation --------------------------------------

	/** reset the value to the uvalue field for each input */
	void updateInputs();

	/** limit the outputs */
	void updateOutputs();

	/** calculate one sample. on function entry @see isStep will be set TRUE and set FALSE after. */
	virtual void step();

	/** a lower priority step. do anything which shouldn't be done in an audio callback here */
	virtual void stepLow();

	// --------------------------- GFX -----------------------------------------

	/** this will automatically set the bright and dark colors from the module colors set so far */
	void updateColors();

	/** simple text statements to stdout */
	void printModuleInfo();

	/** draw a default CSmoduleConnector */
	virtual void drawConnector(CSmoduleConnector *con, int offx=0, int offy=0, float zoom=1.0);

	/** draw the module's body */
	virtual void drawBody(int offx=0, int offy=0, float zoom=1.0);

	/** draw the CStimeline @param tl in the specified rectrangle with current color,
		optionally with background */
	virtual void drawTimeline(CStimeline *tl, int x, int y, int w, int h,
					bool withBackground=false, int backColor=0, int offset=0);

	/** draw complete module */
	virtual void draw(int offx=0, int offy=0, float zoom=1.0);

};




///////////////////////////////////////////////////////////////////////////////////////////////////



#endif // CSMODULE_H_INCLUDED
