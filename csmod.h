/**
 *
 *  CS MODULAR EDITOR
 *  MAIN PROJECT FILE
 *
 *  (c) 0x7da def.gsus- (cdefsus -> gmail.com)
 *      http://cymatrix.org
 *
 *  @version 2010/09/25 v0.1.0 (created)
 *  @version 2010/09/29 v0.1.1 (basic functionality / containers / loadsave / 99k-6sec-256k)
 *  @version 2010/10/16 v0.1.2 (audio out / oscillator'n'stuff / some fancy / ready flags / 140k-9sec-347k)
 *  @version 2010/10/17 v0.1.3 (audio in (v0.1) / fft / file dialog / 157k-10sec-1.18mb)
 *  @version 2010/10/27 v0.1.4 (callback conflict (v0.01) / more modules / 204k-16sec-1.21mb )
 *  @version 2010/10/29 v0.1.5 (more modules / some bugfixes some new bugs / 239k-20sec-1.23mb )
 *  @version 2010/11/07 v0.1.6 (multi select/edit / osc fixes / scope3d samples / modules / 286k-23sec-1.26mb )
 *  @version 2010/11/24 v0.1.7 (seq/table banks / waveplay / diskrender / 321k-23sec-1.39mb )
 *  @version 2010/12/07 v0.1.8 (stuff/modules 370k-24sec-1.41mb )
 *  @version 2011/03/02 v0.2.0 (midi in / 549k-31sec-1.54mb)
 *  @version 2011/03/09 v0.2.1 ()
 *  @version 2011/03/15 v0.2.2 (lots new mods / 95files-18679lines-621k-34sec-1.59mb)
 *  @version 2011/03/24 v0.2.3 (container ins/outs fixed / 114files-20996lines-696k-35sec-1.64mb)
 *  @author def.gsus-
 *
 *  purpose:
 *    defines some helper widgets and the CSmod class which provides
 *    complete GUI handling + properties + audio interfacing
 *
 *  the CSmod and CSpropertyWindow and CSaudioWindow are subclasses of
 *  FLTK's Fl_Window (or Fl_Double_Window) widget
 *
 *  the audio interfacing is done through portaudio.
 *  audio device is selected with the CSaudioWindow class.
 *  any audio input/output within the patch will read/mix it's data from/to
 *  CSmod::rootContainer->inputSamples/outputSample. so multiple audio outs
 *  will simply mix it's channels to the one hardware audio out device.
 *
 *  PNG storage for diskrendering is done through libpng
 *
 *  -----------------------------------------------------------------------
 *
 *  @note
 *    used IDE: code::blocks (project file CSMODULAR.cbp)
 *    tested compiler: MinGW with GCC++ 4.3.3-tdm-1
 *
 *  @note
 *    always use CSmodInit() before instantiating the first CSmod class!!
 *    end program with CSmodDestroy()
 *
 *  @note compile switches:
 *    CSMOD_DEBUGE - excessive!! debugging info on nearly every! function entry (except _DEBUGG)
 *    CSMOD_DEBUGI - all constructor/destructor/clear/init/release function entries
 *    CSMOD_DEBUGF - file operation function entries
 *    CSMOD_DEBUGG - all GUI / GFX function entries
 *	  CSMOD_GLGUI - use opengl for GUI (todo/experimental)
 *
 *  @todo
 *	ok 	-define a during-calculation flag for each module to avoid conflicts with
 *		the property-window callback
 *			-and probably some other thread safe maker
 *			-GET THE GRIP ON C THREAD PROGRAMMING
 *	OK	-show module's ext.windows when container is not visible
 *		-check possible memory holes through the handling of docString()s
 *		-save and load of input user values of container modules!!
 *		-maybe put CSmodule_Container::copyFrom() stuff into CSmodContainer
 *		-amp and offset for connections
 *		-fix audio in
 *		-special asio setup integration (individual channel select)
 *		-midi / osc
 *		-opengl gui drawing and visualization modules
 *	OK	-thousands of modules!! and a way of grouping them meaningfully
 *		-ring input buffer for fft to have a fixed small latency
 *		-reset for all modules (like reread samplerate and stuff)
 *  OK  -midi-module velocity out / velocity sensitive envelopes
 *  OK  -module: true x,y,z to a set of microphones/speakers (matrix optimizer)
 *      -switch graph library to SDL !!
 *      -different samplerates for containers
 *      -container inactive switch
 *
 *
 *	@bug seems to be a bug of GNU C (???)
 *		@see CSmodule::release()
 *		it seems that an overridden release() method will not be
 *		called from the original destructor. don't know what that shit means.
 *		so it's better to override the destructor instead!
 *
 *  @bug crash on new/add container/add input/rename/add output/doubleclick on output (also ctrl-p)
 *       no crash when load patch/rename
 *  @bug no external window when csmod::load() before csmod::show()
 *  @bug crash in csmodule_nn::initNet()
 *  @bug false connections between original and duplicate modules in duplicate container
 *  @bug crash on display file chooser (audio on)
 *  @bug oscillator stands still after while at 0.007hz
 *  @bug no cable connections created in contained(contained(container))
 *  @bug cable from container inputs swapped after duplicate container
 *       (sometimes port index swapped!?)
 *  @bug SOLVED! envelopeAD stopped after <changing samplerate> and decay shortly 0
 *  @bug container connections sometimes mixed up/missing when duplicating/saving
 *
 *  @bug --------------- the callback issue -----------------
 *
 *		lot's of hangs at different times in different places..
 *		or as mark points out: this is c, check the strings
 *		PROBABLY ALSO: conflict between user- and audio- callbacks
 *
 *      crash on cable connect!!!
 *
 *		read of address 0x0000 after loading the same patch several times..
 *
 *		access read vio @ 0xc1...., on save (while running) (written empty file)
 *		again, seems to be callback conflict, though i don't know why in container->store() ?!?
 *		-> happened again even with (my hacky) callback-conflict-avoidance
 *
 *		crashed on first <start audio> after setting up some usual stuff and answering the phone ;)
 *
 *		crashed on insert module <clip soft>
 *
 *		exception on CLOSE APP, with active audio in.
 *		tried again same patch: no exception
 *		-might have to do with noisephaseosc patch which was active before
 *		 which i deleted manually module by module ? probably not
 *		-same exception appeared in different contexts, always on close
 *       should seriously check for thread safety
 *
 *  possibly solved!
 *		exception on load/add audio out/start/add container/add container input ??
 *		bug is unreliable in reproduction
 *      -> very likely removed through CSmodule::ready flag
 *
 *  @bug
 *		access read via 0x0000, on close, while running (organSchwebung.csmod)
 *		->happened again sometimes (different patches) also on NEW PATCH, also when not running
 *		->CTRL-DEL wouldnt do anything -> seems to be corelated
 *
 *	@bug --- float under/overflow ---
 *		the oscillator module went off inside a container (grid_phasemod6.csmod)
 *		ahh, it's the phase value, which gets infinite for too high frequencies.
 *		oscillator can be restarted with positive edge in sync
 *
 *  @bug SOLVED!
 *		exception on closewindow (phase osc patch)
 *		-> had used releaseModule instead of deleteModule in CSmodContainer::release()
 *
 *  @bug SOLVED!
 *		exception: read of address 0x0000 (or something like this)
 *		while duplicating or adding modules when running
 *		seems that only audio-thread is crashed / could even restart
 *		-> added ready flag to whole container as well.
 *			was no good idea to realloc the module[] array without telling the audio thread...
 *
 *  @bug SOLVED! (but dont know how.. prob went away with updates)
 *		crashes on jan X's system (winxp / creamware pulsar) since a while,
 *		wether read of address 0x0000 or unreadable (fast disappearing) bluescreen.
 *		-> runs on same computer with an older xp installation...
 *
 *  @bug
 *		still audio-in problem with ASIO on jan X's creamware system
 *			won't open device, but tries to use it...
 *			->SOLVED last part, hadn't checked availability of
 *				rootContainer->inputSamples[] in CSmodule_AudioIn
 *
 *	@bug SOLVED!
 *		sometimes crash on load, start audio (especially phaseAngriff3.csmod)
 *		write of adress 0x4exxxx
 *		->problem was delay module, forgot to initialize bufpos
 *
 **/

#ifndef CSMOD_H_INCLUDED
#define CSMOD_H_INCLUDED

#define CSMOD_VERSION_MAJ 0 // this will take a while :)
#define CSMOD_VERSION_MIN 4 // coolness counter
#define CSMOD_VERSION_TINY 0 // development version


/* FLTK specific stuff */
#include "FL/Fl.H"
#include "FL/filename.H"
#ifdef CSMOD_GLGUI
#	include "FL/gl.H"
#endif
#include "FL/Fl_Box.H"
#include "FL/Fl_Button.H"
#include "FL/Fl_Check_Button.H"
#ifdef CSMOD_USE_DOUBLE_WINDOW
#   include "FL/Fl_Double_Window.H"
#else
#   include "FL/Fl_Window.H"
#endif
#include "FL/Fl_File_Chooser.H"
#include "FL/Fl_Hold_Browser.H"
#include "FL/Fl_Menu_Button.H"
#include "FL/Fl_Text_Display.H"
#include "FL/Fl_Value_Input.H"

/* the audio host engine */
#include <portaudio.h>

/* the complete module functionality */
#include "csmodcontainer.h"

#include "csmod_midi.h"
//#include "csmod_menu.h"

/* for render output */
#include "csmod_wavewriter.h"

#include "csmod_midi.h"
/* this is included cause it defines min() and max() within std::
   the FLTK widgets override min/max names. :( */
#include <algorithm>


#define CSMOD_TITLE "CS Modular Editor"

using namespace std;

// forward decl
class CSmod;


/** call this on start of your program */
void CSmodInit();

/** call this on exit of your program */
void CSmodDestroy();

#ifdef CSMOD_USE_MIDI
extern CSmidiDevices
	/** list of available midi devices */
	*CSmidiInDevices,
	/** list of available midi devices */
	*CSmidiOutDevices;
#endif

//////////////////////////// CSaudioWindow ///////////////////////////////////////////////////

/**
	audio preferences window
	*/
class CSaudioWindow : public Fl_Window
{
	public:

	Fl_Hold_Browser
        /** browser for audio devices */
        *browser
#ifdef CSMOD_USE_MIDI
		/** midi in */
        ,*browserMidiIn
#endif
    ;

	Fl_Value_Input
		*sampleRate;

	CSmod
		/** parent CSmod class which called this CSaudioWindow */
		*parent;

	/** constructor */
	CSaudioWindow(CSmod *parent);

	/** destructor */
	~CSaudioWindow();

	/** get all available devices and fill the browser widgets */
	void checkAudioDevices();

	/** event callback */
	int handle(int event);
};


//////////////////////////// CSpropertyWindow ////////////////////////////////////////////////

/**
	this is a general purpose property window,
	with which the properties of selected modules can be altered.
	*/
class CSpropertyWindow : public Fl_Window
{

	Fl_Group
		/** the main widget where everything's in */
		*group;

	public:

	/** constructor */
	CSpropertyWindow();

	/** destructor */
	~CSpropertyWindow();

	private:
	/** clear all data */
	void clear();

	/** release all data */
	void release();

	public:

	//--------------------------- gui --------------------------------

	/** kill all widgets if any */
	void releaseWidgets();

	/** assign a module to the property window, build all nescessary widgets, show window */
	void assignModule(CSmodule *mod);

};


//------------------ property widgets ------------------------------------


/** an integer input field */
class CSPinteger : public Fl_Value_Input
{
	public:
	CSmodProperty *property;

	int lastValue;

	CSPinteger (int x,int y,int w,int h,const char *label, CSmodProperty *prop)
		: Fl_Value_Input(x,y,w,h,label) {
			step(1); property = prop;
			value((int)prop->ivalue); lastValue = (int)prop->ivalue; }

	int handle(int event)
	{
		int r=Fl_Value_Input::handle(event);
		if (event==FL_KEYDOWN)
		{
			//if (changed()) { labelcolor(FL_RED); parent->redraw(); }
			if (Fl::event_key()==FL_Enter)
			{
				value(
					std::max(property->minValue,
					std::min(property->maxValue, (csfloat)value() ))); // limit
				lastValue = value();
				property->ivalue = lastValue;
				property->parent->propertyCallback(property);
				//color(FL_WHITE); redraw();
				return 1;
			}
		}
		return r;
	}
};

/** a csfloat input field */
class CSPfloat : public Fl_Value_Input
{
	public:
	CSmodProperty *property;

	csfloat lastValue;

	CSPfloat (int x,int y,int w,int h,const char *label, CSmodProperty *prop)
		: Fl_Value_Input(x,y,w,h,label) {
			property = prop; value(prop->fvalue); lastValue = prop->fvalue; }

	int handle(int event) {
		if ((event==FL_KEYDOWN)&&(Fl::event_key()==FL_Enter)) {
			value( std::max(property->minValue,std::min(property->maxValue, (csfloat)value() ))); // limit
			lastValue = value();
			property->fvalue = lastValue;
			property->parent->propertyCallback(property);
			return 1;
			}
		return Fl_Value_Input::handle(event);
		}
};


/** a string input field */
class CSPstring : public Fl_Input
{
	public:
	CSmodProperty *property;

	CSPstring (int x,int y,int w,int h,const char *label, CSmodProperty *prop)
		: Fl_Input(x,y,w,h,label) { property = prop; value(prop->svalue); }

	int handle(int event) {
		if ((event==FL_KEYUP)) {
			if (property->svalue) free(property->svalue);
			property->svalue = copyString(value());
			property->parent->propertyCallback(property);
			return 1;
			}
		return Fl_Input::handle(event);
		}
};


///////////////////////// render window ///////////////////////////////////////////

/** a render dialog */
class CSrenderWindow : public Fl_Window
{
	public:
	// parent window
	CSmod *csmod;

	Fl_Value_Input *rlen, *rfps, *rchan, *rsr;
	Fl_Check_Button *raudio, *rgfx, *rmultichan;
	Fl_Input *rpath;
	Fl_Button *ok;
	Fl_Label *info;

	/** create window and associate with main form */
	CSrenderWindow(CSmod *csmod);

	void show();
	int handle(int event);
};


//////////////////////////// CSmod ////////////////////////////////////////////////

/* forward @see csmod_installed_modules.h */
class CSmoduleWindow;

/**
	ok, this class will handle all the windowing, event handling,
	moduleContainer handling and all the stuff using FLTK lib,
	and the audio engine setup and buffer passing using portaudio lib.
	it uses CSmodContainer (in CSmod::rootContainer) to acces the module functionality
	*/
class CSmod : public
	#ifdef CSMOD_GLGUI
	Fl_Window
    #elif defined(CSMOD_USE_DOUBLE_WINDOW)
	Fl_Double_Window
    #else
    Fl_Window
	#endif
{
	public:

	CSmodContainer
		/** the current active module container */
		*container,
		/** the topmost container */
		*rootContainer,
		/** list of containers in the stack.
			when opening a sub-container (in CSmodule_Container) the current
			active container will be pushed on this stack. when exiting this
			sub-container the active one will be popped from this stack.
			*/
		**containerStack;

	int
		/** nr of containers in the stack */
		nrStack;

	bool
		/** signifies if audio processing is active.
		    means that CSaudioCallback and CSscreenCallback are
		    called repeatedly. */
		running;

	float
		/** seconds between screen refreshs when running.
			uses FLTK's Fl::add_timeout() which does not seem to
			make more than 10 fps. */
		screenUpdateTime;

	// ----- midi -------

	int
		midiInHandle,
		midiOutHandle;

	// ----- audio ------

	bool
		/** switched off when audio should do nothing */
		ready,
		/** true during @see step() routine */
		isStep;

	int
		/** nr of current input channels */
		nrChannelsIn,
		/** nr of current output channels */
		nrChannelsOut,
		/** the current samplerate, default 44100 */
		sampleRate,
        /** the size of the input & output audio buffer */
        bufferSize;

	PaStream
        /** portaudio input/output stream */
        *stream;


	// ------ gui -------

	int
		/** FLTK constant, used to pass the mousebutton for mouse drag/move events */
		lastMouseButton;

	CSmoduleWindow
		/** link to the module creation window */
		*moduleWindow;

	CSpropertyWindow
		/** the window used for altering module properties */
		*propertyWindow;

	CSaudioWindow
		/** the window used for setting up the audio properties */
		*audioWindow;

	CSrenderWindow
		/** rendering settings window */
		*renderWindow;

	/** file browser */
	Fl_File_Chooser *browser;
	/** a general purpose text display */
	Fl_Window *textWindow;
	/** the textbox within @see textWindow */
	Fl_Text_Display	*textBox;
	/** the textbuffer associated with @see textWindow */
	Fl_Text_Buffer *textBuffer;

	bool
		/** define wheter to show external windows or not,
			change with @see enableExternalWindows() and @see disableExternalWindows(). */
		showExternalWindows;


	// ------- methods ---------

	/** constructor */
	CSmod();

	/** destructor */
	~CSmod();


	/** general help string */
	public: char *docString();

	private:

	/** reset all fields / DONT CALL */
	void clear();

	/** release alloc. memory / DONT CALL */
	void release();

	// ------------------------ containers -------------------------------------------

	/** push the current container onto stack
		(only if it's different from the top of stack) */
	void pushContainer();

	public:
	/** pop a container from stack, if any */
	bool popContainer();

	/** assign a new container. the current will be pushed on stack */
	void assignContainer(CSmodContainer *con);

	private:
	/** the actual (private) "assign new container" method, w/o stack
		use assignContainer() instead. */
	void doAssignContainer(CSmodContainer *con);

	/** assign conn's rootContainer field and recurse through all
		sub-containers within container modules */
	void updateSubContainers(CSmodContainer *conn);

	public:
	/** assign the rootContainer field for every sub-container within
		the rootContainer */
	void updateSubContainers();


	// ------------------------ patch ------------------------------------------------

	/** delete everything except rootContainer */
	void newPatch();

	/** load a patch from a file into @see rootContainer,
		if @param theContainer == 0 loads patch into this container instead */
	void loadPatch(const char *fn, CSmodContainer *theContainer=0);

	/** save @see rootContainer as patch to a file,
		if @param theContainer == 0 save this container instead */
	void savePatch(const char *fn, CSmodContainer *theContainer=0);


	/** ask container to add module, position it centric */
	CSmodule *addModule(CSmodule *mod);

	// ------------------ FLTK / GUI stuff -------------------------------------------
	/** force redraw (if not running) */
	void refresh();

	public:

	/** get all events here.
		this function simply passes all events further to the active CSmodContainer */
	int handle(int event);

	/** call CSmodContainer->draw() to draw the surface */
	void draw();

	/** open a window to display a text */
	void showText(const char *title, const char *text);

	/** open a file browser and select a file, returns "" if not ok.
		set save to TRUE to open a save file browser.
		if @see dlgLabel != 0 use this as text for label */
	char *selectFile(bool save=false, const char *dlgLabel=0);

	/** set the @see showExternalWindows flag to FALSE (TODO: hide any open windows!) */
	void disableExternalWindows();

	/** set the @see showExternalWindows flag to TRUE. */
	void enableExternalWindows();

	// ------------------------- midi ------------------------------------------------
#ifdef CSMOD_USE_MIDI
	void setMidiInDevice(int deviceId);

	void onMidiIn(DWORD data1, DWORD data2);
#endif
	// ------------------------ audio ------------------------------------------------

	/** setup/initialize the audio devices and streams,
        deviceIdx as returned by portaudio.
		use -1 to ignore. */
    void setAudioDevice(int deviceIdx, int rate);

	/** disable the audio */
	void closeAudioDevice();

	/** when streams are set up, start audio processing */
	void start();

	/** stop all audio processing */
	void stop();

	/** calculate the engine for @param frameCount steps, and write to @param output */
	void step(void *output, unsigned long frameCount);

	/** pass the @param input buffer to the rootContainer's @see inputSamples */
	void passInput(const void *input, unsigned long frameCount);

	/** low priority module update stuff */
	void stepLow();

	/** render everything (wav audio/external windows) into the directory defined by
		@param pathname (like "c:/yourpath/")
		duration as seconds in @param sec,
		external window images will be stored at @param fps frames per seconds
		an audiostart will be issued on start */
	void render(const char *pathname, float sec, int nrChan, int sr, int fps,
					bool renderAudio=true, bool renderGraphics=true, bool multiChannel=false );

};

/** the default csmod class */
extern CSmod * theCSmod;

#ifdef CSMOD_USE_MIDI
/** callback for midi in messages */
void CALLBACK CSmidiInCallback(HMIDIIN handle, UINT msg, DWORD data, DWORD par1, DWORD par2);
#endif

/** callback from installedModulesPopUp */
void CSonAddModule(void *userData1, void *userData2);

/** the default callback function for screen updates,
	the userData will be a pointer to the CSmod class.
	the callback is used by FLTK's add_timer() */
void CSscreenCallback(void *userData);

/** the default callback function for audio out streams,
	the userData will be a pointer to the CSmod class.
	the callback is used by portaudio's stream functions. */
int CSaudioOutCallback (
    const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData );

/** the default callback function for audio input streams,
	the userData will be a pointer to the CSmod class.
	the callback is used by portaudio's stream functions. */
int CSaudioInCallback (
    const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData );


/** the default callback function for ASIO input/output streams,
	the userData will be a pointer to the CSmod class.
	the callback is used by portaudio's stream functions. */
int CSaudioInOutCallback (
    const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData );


#endif // CSMOD_H_INCLUDED
