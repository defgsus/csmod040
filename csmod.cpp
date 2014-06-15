#include "csmod.h"
#include "csmod_installed_modules.h"

#include <math.h> // only debug

CSmod * theCSmod = 0;
#ifdef CSMOD_USE_MIDI
CSmidiDevices * CSmidiInDevices = 0;
CSmidiDevices * CSmidiOutDevices = 0;
#endif

char *CSmod::docString()
{
	size_t len = 1000000;
	char *r = (char*) calloc(len,1);
	snprintf(r, len, "\
                      _            _   _   _          _  _\n\
                     |  | | |\\ /| | \\ /   | \\ \\  | | |  /\n\
                     |  \\_/ | | | |_| \\_  | | |\\ | | |  \\_\n\
                     |   |  |   | | |   \\ | | | \\| | |    \\\n\
                     \\_  |  |   | | | \\_/ \\_| |  \\ | \\_ \\_/\n\
\n\
                      MODULAR EDITOR - (c) 0x7db def.gsus-\n\
                     ver %d.%d.%d (built %s %s)\n\
\n\
 developement: cdefsus@gmail.com || jan.ten@cymatrix.org || zeichlicht@gmail.com\n\
 project contact: http://cymatrix.org\n\
\n\
--------------------------------------------------------------------------------\n\
this doc: * introduction\n\
          * known probs\n\
          * what's new?\n\
          * general help on handling\n\
          * how to build big patches\n\
          * disk rendering\n\
--------------------------------------------------------------------------------\n\
it's an almost common modular digital signal processing framework,\n\
  in it's still EXPERIMENTAL stage,\n\
to solve several image and audio creation and manipulation tasks in realtime.\n\
\n\
a variety of modules (currently %d in %d groups) can be added and connected \n\
to each other. currently provided hardware interfaces are:\n\
  - audio in, audio out\n\
    the software uses the portaudio dynamic link library, so it should be\n\
    possible, to compile an individual version for your hardware and use it\n\
    with this software(?). however, the delivered precompiled dll supports\n\
    WMME, DX and ASIO (currently asio input won't work).\n\
  - midi in\n\
    currently note on/offs, control change and nrpn control change\n\
\n\
a module generally has it's inputs on the left and outputs on the right side. \
each input, in fact, represents a parameter of the module. they can be altered \
using the mouse or keyboard and can be modulated through cable connections. \
there can be multiple input or output connections at one connector. multiple \
signals connected to one input will be added together.\n\
\n\
the numerical value on the module input represents an offset which is always added \
to the incoming signal.\n\
\n\
this whole software is written to get in contact with the numbers. please contact me \
(cdefsus) if anything about a modules internal behaviour is not clear. unless otherwise \
mentioned, all modules calculate anything 'the hard way', f.e. the oscillator uses a \
FPU sin() function. all significant numbers are always visible on the modules. \
ranges of the input values are given in the individual module help, \
generally they are between 0.0 and 1.0 where possible or meaningful.\n\
\n\
each module introduces a delay of ONE audiosample. normally you can ignore that. but for \
certain situations you may have to delay other signals in order to make all signals that \
belong together reach a module at the exact same time. use the 'delay (1 sample)' or \
'delay (sample)' module for that.\n\
\n\
check out the gui-help below but before make yourself comfortable with the\n\
--------------------------------------------------------------------------------\n\
\n\
known issues!\n\
\n\
at this point it is not clear wether or not the source is going to be published and \
under which conditions. certainly the cymasonic modular image and audio synthesis \
project relies on collaboration in any healthy way. the source is completely cross-platform \
(except for the midi-part right now), but  has not been compiled on something different than \
XP so far.\n\
\n\
0.1.6 >>\n\
still the same prob of audio callback interference on multi-core machines. \
if any action you try to do does not cause any reaction on side of the program, \
please CHECK THE DEBUG-CONSOLE if a callback error occured. if so, try again... \
and maybe stop audio engine for a moment.\n\
\n\
save your work often, the program crashes fairly regular. you will learn to know \
before it happens :)\n\
\n\
i know this is terrible but *sometimes* the program crashes when saving a patch. \
of course it crashes after clearing the file, so be sure to save to a new file once and then \
(this does also happen when audio is off!!) BUT! it only happens the FIRST time you try to \
save a patch, so it's adviseable to save early in the creation process...\n\
\n\
yes that's tough! however, i suspect that lot of the crashes stem from my (intuitive) \
use of the FLTK library (don't worry guys, you lib is great, eah..., really). at some point i'd \
like to migrate to another, simpler window manager to avoid those conflicts.\n\
\n\
--------------------------------------------------------------------------------\n\
\n\
what's new?\n\
\n\
0.2.3 \n\
      general:\n\
        container connectors fixed (no mismatch anymore / \
          input values saved in patch / can be moved (ctrl-up/down) )\n\
0.2.2 \n\
      general:\n\
        fine and course mouse value editing (alt/ctrl)\n\
        cable value display, cables behind modules (alt+b)\n\
      modules:\n\
        gate delay, gate repeat, oscillator set 3, oscillator (period),\n\
        cellular automata, switch, switch matrix!!, various fades,\n\
        different polyphonic synthesizers, value meter, view 7,\n\
        objects (stars,points,ghm), some logics,\n\
        (scope3d client/server drawing and image buffer send)\n\
\n\
0.2.0 \n\
      general:\n\
        patch and sub-containers MUCH faster!!\n\
        MIDI IN (notes, cc, nrpn)\n\
\n\
0.1.9 \n\
      general:\n\
        off-line messurement of module's calculation time (ctrl-m)\n\
      modules:\n\
        lot's of new modules...\n\
        (scope3d - dome perspective / auto-subsamples / client-server mode ...)\n\
        microphone 2d/3d\n\
        bit <-> integer\n\
        backprob. net and liquid state machine\n\
        midi file player\n\
\n\
0.1.8 general:\n\
        non-realtime, non-interactive disk rendering of audio and video.\n\
        sampling rate adjustable\n\
        connector active flags (internal feature - better dynamic con. handling)\n\
      gui:\n\
        groups of available modules\n\
        area select\n\
        ctrl-alt-s and ctrl-alt-l to store/load CURRENT container\n\
        ctrl-c to build a container from selected modules (to be continued)\n\
        alt-rightclick to copy cables\n\
      modules:\n\
        (multiple banks for sequencer and table 2d module)\n\
        (oscillators use 64 bit internal phase, frequency can be >= ~0.007\n\
        (1-3 dimensional beta term)\n\
        start gate module to syncronize patches on audio start or render start\n\
        wave file player module (first buggy implementation)\n\
        quad amp / quad mix / timeline / lots of other tiny stuff\n\
\n\
0.1.6 gui: \n\
        multi-module-selection with multi-edit/duplicate...\n\
        more bugs in the list\n\
      modules:\n\
        (scope3d - draw subsamples) / selector / clock / delay spread\n\
        camera (first bogus calculation) / oscillator table / \n\
        sequencer / table 2d\n\
\n\
--------------------------------------------------------------------------------\n\
\n\
  ----- general keys -----\n\
\n\
  F1     - module help when module is selected, or this help\n\
  F7     - start audio engine\n\
  F8     - stop audio engine\n\
  F10    - audio device settings (select device and close window to apply)\n\
  alt-a  - view all modules (set view port accordingly)\n\
  \n\
  ctrl-n - new patch (wipe out everything)\n\
  ctrl-l - load a patch (opens file browser)\n\
  ctrl-s - save a patch (opens file browser)\n\
  ctrl-alt-l - load a patch into the current container, which can also be\n\
			   a sub-container\n\
  ctrl-alt-s - store the current (sub-)container\n\
  \n\
  ctrl-c - create a container with the selected modules\n\
           the modules in the container will be connected as the original\n\
           modules. all connections to modules which are not selected,\n\
           will create container inputs/outputs.\n\
\n\
  ctrl-r - disk render dialog\n\
\n\
  ctrl-m - messure all module's calculation times and print in debug window\n\
           (audio needs to be off)\n\
\n\
  ----- module handling -----\n\
\n\
  ins | right-click in free space\n\
           - show module chooser\n\
             then click on a button to create the module\n\
\n\
  left-click in free space\n\
           - move the screen display offset\n\
		   - if shift is pressed select area (modules)\n\
		   - if also alt is pressed then multiselect area\n\
\n\
  right-click in free space\n\
           - zoom in/out (hold down right mousekey and move up/down)\n\
\n\
  left-click on module header\n\
           - select the module / start moving the module(s)\n\
           - select multiple modules together with the alt key\n\
\n\
  ctrl-p | double-click on module header\n\
           - open the property window for the selected module.\n\
\n\
  ctrl-a   - select all modules\n\
  ctrl-del - delete the selected module(s) (no undo!)\n\
  ctrl-d   - duplicate the selected module(s)\n\
\n\
  alt-b    - move cables behind modules\n\
\n\
  ----- module values / parameters -----\n\
\n\
  cursors  - move the edit focus for value editing\n\
\n\
  ctrl-up/down\n\
           - for container modules: move container value up/down\n\
             (when module AND input value is selected)\n\
\n\
  return | double-click on value\n\
           - start keyboard value edit\n\
             (quit with return or escape, type with 0-9 - . or ,\n\
             move with cursors or home and end)\n\
\n\
  left-click on value\n\
           - change value in 1/10th (hold down mousekey and move up/down)\n\
             when alt is pressed, value is changed in 1/1000th steps,\n\
             with ctrl in 50th steps\n\
\n\
  ----- connections -----\n\
\n\
  left-click on input/output connector\n\
           - start connecting modules (hold down mousekey and drag the cable to\n\
             the desired output/input\n\
\n\
  right-click on cable\n\
           - delete the cable\n\
\n\
  right-click on input/output connector\n\
           - move all connected cables to another input/output\n\
           - hold alt-key to copy the connections instead\n\
\n\
--------------------------------------------------------------------------------\n\
\n\
:building big patches:\n\
\n\
it is possible to contain a bunch of modules in a container (see system::container). \
there are currently three way to do that:\n\
1. create an empty container and place modules inside,\n\
2. create an empty container and load a patch into that container (ctrl-alt-L) or\n\
3. select a bunch of modules and auto-contain them (ctrl-C).\n\
each container module has an <active> input. if this is > 0, the container within the module \
will be processed normally. if it's <= 0, the container will not be calculated and the system \
resources are free for other things. that way you can combine several patches into one patch \
and blend between different scenes without having to calculate all scenes all the time.\n\
\n\
\n\
--------------------------------------------------------------------------------\n\
\n\
:the disk rendering:\n\
\n\
it is possible to render the output (audio out tracks and graphic windows) to files \
(no audio in and stuff can be used). \
in case of audio this is currently done in 1-channel RIFF WAVE files, and graphic windows are \
rendered into sequential PNG files. press ctrl-r to show the render dialog.\n\
\n\
select the duration of the whole rendering in seconds and the frames per seconds of \
the graphics, and wether you want to render audio or graphics or both. the output path \
HAS TO EXIST!! press GO and wait until finished. progress is shown in the debug window.\n\
\n\
the nr of channels can be up to %d. either individual mono waves will be rendered for them \
(their order is the same as on any audio out module) or all channels will be stored in \
one multi-channel wave file (most players only support up to two channels, however the \
wave file module will output all channels).\n\
\n\
if you want to synchronize your patch to the start of the render process use the <start gate> \
module. it will issue a small gate on every start of the audio engine or start of rendering \
process.\n\
\n\
to make certain settings that are only applied when disk rendering, you can use the 'on rendering' \
module. it will output a 1.0 if the calculation is done for disk rendering.\n\
\n\
the external graphic windows, like of the scope3d, are not visually updated during disk rendering. \
only scope3d modules that are NOT clients are saved to disk.\n\
\
",
	// title bar stuff
	CSMOD_VERSION_MAJ,CSMOD_VERSION_MIN,CSMOD_VERSION_TINY, __DATE__, __TIME__,
	// desc text
	nrInstalledModules,
	getNrInstalledModuleGroups(),
	// diskrendering
	CSMOD_MAX_CHAN
	);
	return r;
}






void CSmodInit()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSmodInit()\n");
	#endif

	printf("hello seeker!\n");
	printf("this is\n");
	printf("                  ******************************************\n");
	printf("                  *    CYMASONICS MODULAR EDITOR v%d.%d.%d    *\n",CSMOD_VERSION_MAJ,CSMOD_VERSION_MIN,CSMOD_VERSION_TINY);
	printf("                  *      (built %s %s)      *\n", __DATE__, __TIME__);
	printf("                  * (c)0x7db def.gsus- (cdefsus@gmail.com) *\n");
	printf("                  ******************************************\n");
	printf("\n\n");

	#ifdef CSMOD_GLGUI
	// enable opengl drawing within Fl_Window
	printf("-> opengl GUI\n");
	Fl::gl_visual(FL_RGB);
	#else
	Fl::visual(FL_RGB|FL_DOUBLE|FL_INDEX);
	#endif

	printf("checking modules...\n");
	initInstalledModules();

#ifdef CSMOD_USE_MIDI
	printf("checking midi... ");
	CSmidiInDevices = CSmidi_newDeviceList();
	CSmidi_getDevices(CSmidiInDevices, true);
	CSmidiOutDevices = CSmidi_newDeviceList();
	CSmidi_getDevices(CSmidiInDevices, false);
	printf("%d ins, %d outs\n", CSmidiInDevices->nr, CSmidiOutDevices->nr);
#endif

	printf("initializing audio engine...\n");
	Pa_Initialize();


	printf("\n");
	printf("F1 for HELP\n\n");
}

void CSmodDestroy()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSmodDestroy()\n");
	#endif

	printf("closing audio...\n");
	Pa_Terminate();

#ifdef CSMOD_USE_MIDI
	CSmidi_deleteDeviceList(CSmidiInDevices);
	CSmidi_deleteDeviceList(CSmidiOutDevices);
#endif

	releaseInstalledModules();
}





//////////////////////////// CSaudioWindow ////////////////////////////////////////////////

CSaudioWindow::CSaudioWindow(CSmod *parent)
	: Fl_Window(820,330,"select audio device(s). close window to apply.")
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSaudioWindow::CSaudioWindow(0x%p)\n",parent);
	#endif
	this->parent = parent;

	make_current();
	browserIn = new Fl_Hold_Browser(10,10,400,150, "input devices");
	browserOut = new Fl_Hold_Browser(410,10,400,150, "output devices");
	browserMidiIn = new Fl_Hold_Browser(10,175, 400,100, "midi input devices");
	sampleRate = new Fl_Value_Input(370,h()-30,100,25, "sample rate");
	sampleRate->value(44100);

	checkAudioDevices();
}


CSaudioWindow::~CSaudioWindow()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSaudioWindow::~CSaudioWindow()\n");
	#endif

}



void CSaudioWindow::checkAudioDevices()
{
	#ifdef CSMOD_DEBUGE
	printf("CSaudioWindow::checkAudioDevices()\n");
	#endif

	browserIn->clear();
	browserOut->clear();

	int num = Pa_GetDeviceCount();

	// get device infos
    for (ptrdiff_t i=0;i<num;i++)
	{
		// get each device info (has to be const)
		const PaDeviceInfo *inf = Pa_GetDeviceInfo(i);
		const PaHostApiInfo *apiinf = Pa_GetHostApiInfo(inf->hostApi);

		char dname[1024];

		// store the device name and the id as browser items
		if (inf->maxInputChannels>0)
		{
			// is input device
			sprintf(dname,"%s: %d ch. %s\n",apiinf->name,inf->maxInputChannels,inf->name);
			browserIn->add(dname, (void*)i);
		}
		if (inf->maxOutputChannels>0)
		{
			// is (also) output device
			sprintf(dname, "%s: %d ch. %s\n",apiinf->name,inf->maxOutputChannels,inf->name);
			browserOut->add(dname, (void*)i);
		}
	}

#ifdef CSMOD_USE_MIDI
	// midi devices
	CSmidiDevices *md = CSmidi_newDeviceList();
	CSmidi_getDevices(md);
    for (ptrdiff_t i=0;i<md->nr;i++)
	{
		browserMidiIn->add(md->name[i], (void*)i);
	}
#endif

}


int CSaudioWindow::handle(int event)
{
	if (event==FL_HIDE)
	{
		// get the device indexes

		int inDevice = -1;
		if ( (browserIn->value()>0) && (browserIn->value()<=browserIn->size()) )
            inDevice = (ptrdiff_t)browserIn->data(browserIn->value());

		int outDevice = -1;
		if ( (browserOut->value()>0) && (browserOut->value()<=browserOut->size()) )
            outDevice = (ptrdiff_t)browserOut->data(browserOut->value());

		parent->setAudioDevice(inDevice, outDevice, sampleRate->value());

#ifdef CSMOD_USE_MIDI
		int midiInDevice = -1;
		if ( (browserMidiIn->value()>0) && (browserMidiIn->value()<=browserMidiIn->size()) )
		{
			midiInDevice = browserMidiIn->value()-1;
			parent->setMidiInDevice(midiInDevice);
		}
#endif

	}
	return Fl_Window::handle(event);
}










//////////////////////////// CSpropertyWindow ////////////////////////////////////////////////

CSpropertyWindow::CSpropertyWindow()
	: Fl_Window(480,320,"module properties")
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSpropertyWindow::CSpropertyWindow()\n");
	#endif

	clear();

	// make window resizable from minimum to screen size
	size_range(w(),h());

}

CSpropertyWindow::~CSpropertyWindow()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSpropertyWindow::~CSpropertyWindow()\n");
	#endif

	release();
}



void CSpropertyWindow::clear()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSpropertyWindow::clear() \n");
	#endif

	group = 0;
}

void CSpropertyWindow::release()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSpropertyWindow::release() \n");
	#endif

	releaseWidgets();
}




	//--------------------------- gui --------------------------------



void CSpropertyWindow::releaseWidgets()
{
	#ifdef CSMOD_DEBUGE
	printf("CSpropertyWindow::releaseWidgets() \n");
	#endif

	if (group) delete group;
	group = 0;
}


void CSpropertyWindow::assignModule(CSmodule *mod)
{
	#ifdef CSMOD_DEBUGE
	printf("CSpropertyWindow::assignModule(CSmodule *mod) \n");
	#endif

	releaseWidgets();

	// be sure, this window is active
	make_current();

	begin();

	// main group container
	group = new Fl_Group(0,0,w(),h(),"");

	// ----- start building widgets ----

	int xo=10;
	int yo=10;
	int wi=190, wi1=80;
	int he=30;

	// display name of module
	Fl_Box *box = new Fl_Box(xo,yo,wi*2+wi1-10,he,mod->name);
	box->color(fl_rgb_color(180,200,220));
	box->box(FL_ENGRAVED_BOX);

	yo+=he+10;

	// go through all properties
	for (int i=0;i<mod->nrProp;i++)
	{
		// name display
		Fl_Box *box = new Fl_Box(xo,yo,wi,he,mod->property[i]->uname);
		box->color(fl_rgb_color(180,200,180));
		box->box(FL_EMBOSSED_BOX);

		// appropiate input/edit
        //CSPinteger *inpi;
        //CSPfloat *inpf;
        //CSPstring *inps;
		switch (mod->property[i]->typ)
		{
			case CS_PROP_INT:
                /*inpi =*/ new CSPinteger(wi+wi1, yo, wi,he, "integer", mod->property[i]); break;
			case CS_PROP_FLOAT:
                /*inpf =*/ new CSPfloat(wi+wi1, yo, wi,he, "float", mod->property[i]); break;
			case CS_PROP_STRING:
                /*inps =*/ new CSPstring(wi+wi1, yo, wi,he, "string", mod->property[i]); break;
		}

		yo += he+5;
	}

	// stop adding widgets
	end();

	// show property window
	h(yo);
	set_modal();
	show();

}













///////////////////////// render window ///////////////////////////////////////////

/** a render dialog */
CSrenderWindow::CSrenderWindow(CSmod *csmod) : Fl_Window(480,320,"render audio/gfx")
{
	this->csmod = csmod;

	//set_modal();
	int y=10;
	rlen = new Fl_Value_Input(200,y,200,30,"length in seconds");
	rlen->value(10);
	y+=30;
	rsr = new Fl_Value_Input(200,y,200,30,"sample rate (audio)");
	rsr->value(44100);
	y+=30;
	rchan = new Fl_Value_Input(200,y,200,30,"nr of channels (audio)");
	rchan->value(2);
	y+=30;
	rfps = new Fl_Value_Input(200,y,200,30,"frames per seconds (gfx)");
	rfps->value(30);
	y+=40;
	raudio = new Fl_Check_Button(10,y,20,20,"render audio tracks?");
	raudio->value(1);
	y+=30;
	rmultichan = new Fl_Check_Button(10,y,20,20,"create multi channel wave (all in one file)?");
	rmultichan->value(0);
	y+=30;
	rgfx = new Fl_Check_Button(10,y,20,20,"render graphics?");
	rgfx->value(1);
	y+=30;
	rpath = new Fl_Input(100,y,w()-110,20,"output path");
	rpath->value("./output/");

	//info = new Fl_Label(10,h()-50,w()-20,30, "");

	ok = new Fl_Button(w()-100,h()-50, 70,30, "GO");
}

void CSrenderWindow::show()
{
	char text[2048];
	sprintf(text, "rendering audio/video (%d output channel%s)", csmod->nrChannelsOut,
		(csmod->nrChannelsOut!=1)?"s":"");
	label(text);
	Fl_Window::show();
}

int CSrenderWindow::handle(int event)
{
	Fl_Widget *w = Fl::readqueue();
	if (w == ok)
	{
		csmod->render(
			rpath->value(), rlen->value(), rchan->value(), rsr->value(), rfps->value(),
			(raudio->value()==1), (rgfx->value()==1), (rmultichan->value()==1));
		hide();
		return 1;
	}

	return Fl_Window::handle(event);
}















//////////////////////////// CSmod ////////////////////////////////////////////////






////// LETS GO !!! ///////

CSmod::CSmod() :
	#ifdef CSMOD_GLGUI
	Fl_Window
	#else
	Fl_Double_Window
	#endif
	(640, 480, CSMOD_TITLE)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSmod::CSmod()\n");
	#endif

	theCSmod = this;

	clear();

	// make window resizable from minimum to screen size
	size_range(320,200);

	// create property window
	propertyWindow = new CSpropertyWindow();
	// create audio properties
	audioWindow = new CSaudioWindow(this);
	// create module window
	moduleWindow = new CSmoduleWindow(this);
	// render window
	renderWindow = new CSrenderWindow(this);

	// create the top-most container and assign
	rootContainer = new CSmodContainer();
	doAssignContainer(rootContainer);

	// create the text window
	textWindow = new Fl_Window(674,480,"text display");
	textBox = new Fl_Text_Display(1,20,textWindow->w()-2,textWindow->h()-25, "a text");
	textBox->wrap_mode(1,80);
	textBox->textfont(FL_COURIER);
	textBox->color(0);
	textBox->textcolor(FL_WHITE);
	textBuffer = new Fl_Text_Buffer();
	// assign buffer to display
	textBox->buffer(textBuffer);

	browser = new Fl_File_Chooser(".", "CS Patches (*.csmod)", FL_SINGLE, "select patch");

	// switch back to main window (wont work on _gl or _double window anyway)
	make_current();

	ready = true;
}

CSmod::~CSmod()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSmod::~CSmod()\n");
	#endif
	closeAudioDevice();
	release();
}

void CSmod::clear()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSmod::clear()\n");
	#endif

	running = false;
	ready = false;
	isStep = false;
	propertyWindow = 0;
	container = 0;
	rootContainer = 0;
	containerStack = 0;
	nrStack = 0;

	lastMouseButton = 0;

	screenUpdateTime = 0.04; // seconds
	showExternalWindows = true;

	// -audio stuff-
	nrChannelsIn = 1;
	nrChannelsOut = 1;
	sampleRate = 44100;
	streamIn = 0;
	streamOut = 0;
	bufferSizeIn = 64*16;
	bufferSizeOut = 64;

}


void CSmod::release()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSmod::release()\n");
	#endif

	if (propertyWindow) delete propertyWindow;
	if (renderWindow) delete renderWindow;
	if (audioWindow) delete audioWindow;

	if (container) delete container;
	if (containerStack)
	{
		for (int i=0;i<nrStack;i++) delete containerStack[i];
		free(containerStack);
	}

	delete browser;

	clear();
}









	// ------------------------ containers -------------------------------------------

void CSmod::pushContainer()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmod::pushContainer()\n");
	#endif
	if (!containerStack)
	{
		containerStack = (CSmodContainer**) calloc(1, sizeof(int));
		nrStack = 1;
	} else {
		nrStack++;
		containerStack =
			(CSmodContainer**) realloc(containerStack, nrStack*sizeof(int));
	}
	// check for duplicate
	if ((nrStack>1)&&(containerStack[nrStack-2]==container))
		{ nrStack--; return; }

	containerStack[nrStack-1] = container;
}

bool CSmod::popContainer()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmod::popContainer()\n");
	#endif

	if ((!containerStack)||(nrStack<1)) return false;

	doAssignContainer(containerStack[nrStack-1]);
	nrStack--;
	return true;
}


void CSmod::assignContainer(CSmodContainer *con)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmod::assignContainer(0x%p)\n",con);
	#endif

	pushContainer();
	doAssignContainer(con);
}

void CSmod::doAssignContainer(CSmodContainer *con)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmod::doAssignContainer(0x%p)\n",con);
	#endif

	container = con;
	container->parent = this;
	container->rootContainer = rootContainer;
	refresh();
}

void CSmod::updateSubContainers()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmod::updateSubContainers()\n");
	#endif
	updateSubContainers(rootContainer);
}

void CSmod::updateSubContainers(CSmodContainer *conn)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmod::updateSubContainers(0x%p)\n",conn);
	#endif

	// set container's rootContainer
	conn->rootContainer = rootContainer;
	// check for container modules and repeat
	for (int i=0;i<conn->nr;i++)
	{
		CSmodule_Container *cm = dynamic_cast<CSmodule_Container*>(conn->module[i]);
		if (cm) updateSubContainers(cm->container);
	}
}


	// ---------------------- modules ------------------------------------------------

void CSmod::newPatch()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmod::newPatch()\n");
	#endif
	CSwait();

	bool oldReady = ready;
	ready = false;

	// clear stack
	while (popContainer());
	// clear root container
	doAssignContainer(rootContainer);
	rootContainer->deleteAll();

	ready = oldReady;

}


void CSmod::loadPatch(const char *fn, CSmodContainer *theContainer)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmod::loadPatch(\"%s\",0x%p)\n",fn,theContainer);
	#endif

	if (!theContainer) {
		// clear stack
		while (popContainer());
		theContainer = rootContainer;
	}

	CSwait();
	ready = false;

	printf("loading %s\n",fn);
	FILE *f = fopen(fn,"rb");

	theContainer->deleteAll();
	int k = theContainer->restore(f);

	// create sub container if sub-container-patch
	if (k==1)
	{
		//CSmodule_Container *mc = (CSmodule_Container*)
		addModule(getInstalledModule("container"));
		//mc->restore(f);
	}

	fclose(f);

	ready = true;
}


void CSmod::savePatch(const char *fn, CSmodContainer *theContainer)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmod::loadPatch(\"%s\")\n",fn);
	#endif

	if (!theContainer) theContainer = rootContainer;

	CSwait();

	printf("saving %s\n",fn);
	FILE *f = fopen(fn,"w");

	theContainer->store(f);

	fclose(f);
}


CSmodule *CSmod::addModule(CSmodule *mod)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmod::addModule()\n");
	#endif
	if (!container) return 0;

/*
	if (isStep) { \
		printf("waiting for audio callback\n"); \
		for (;isStep;); \
		printf("ok\n"); \
	}
	*/
	if (isStep) {
		printf("CALLBACK CONFLICT!! please repeat your wish..\n");
		return 0;
	}

	CSmodule *mod1 = container->addModule(mod);

	/* -- actually the following is done by container->addModule
	float z = std::max(0.1f, container->zoom);
	mod1->setPos(
		(w()-mod->width)/2/z-container->offx,
		(h()-mod->height)/2/z-container->offy
		);
	container->repositionModule(mod1);
	*/

	refresh();

	return mod1;
}






	// ------------------ FLTK / GUI stuff -------------------------------------------

void CSmod::refresh()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmod::refresh()\n");
	#endif
	if (!running) redraw();
}


int CSmod::handle(int event)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmod::event(%d)\n",event);
	#endif
	if (!container) return 0;

	bool ctrl = ((Fl::event_state()&FL_CTRL) || (Fl::event_state()&FL_COMMAND));
	bool alt = (Fl::event_state()&FL_ALT);
	//bool shift = (Fl::event_state()&FL_SHIFT);

	bool wantEvent=false;

	char *fn = 0;


	// check event
	switch (event)
	{
		// acknowledge that window wants events
		case FL_ENTER: return 1; break;

		//case FL_HIDE: Fl::release();
		/* TODO: delete ALL possible windows when closing this window and quit app. */

		case FL_KEYDOWN:

			wantEvent = true;
			// GLOBAL KEYS
			switch (Fl::event_key())
			{
				case FL_Escape:
					wantEvent = container->keyDown(Fl::event_key());
					refresh();
					// just catch the escape so it wont kill the window
					return 1;
					break;
				case FL_F+1: // on f1 (help)
					if (container->selModule)
						showText(container->selModule->name, container->selModule->docString());
					else
						showText("general help", docString());
					break;
				case FL_F+7: // on f7 (play)
					start();
					break;
				case FL_F+8: // on f8 (stop)
					stop();
					break;
				case FL_F+10: // on f10 (audio preferences)
					audioWindow->show();
					break;

				// new
				case 'n': if (!ctrl) { wantEvent=false; break; }
					newPatch();
					break;
				// load
				case 'l': if (!ctrl) { wantEvent=false; break; }
					fn = selectFile(false, (alt)? "load patch into current container" : 0);
					// into current container on ALT
					if (fn) { loadPatch(fn, (alt)? container : 0); free(fn); }
					break;
				// save
				case 's': if (!ctrl) { wantEvent=false; break; }
					fn = selectFile(true, (alt)? "save current container as patch" : 0);
					if (fn) { savePatch(fn, (alt)? container : 0); free(fn); }
					break;
				// render audio
				case 'r': if (!ctrl) { wantEvent=false; break; }
					renderWindow->show();
					break;

				// else
				default:
					wantEvent = false;
			}
			if (wantEvent) return 1;
			// pass key through to container
			wantEvent = container->keyDown(Fl::event_key());
			break;
		case FL_PUSH:
			lastMouseButton = Fl::event_button();
			wantEvent = container->mouseDown(Fl::event_x(), Fl::event_y(), lastMouseButton);
			break;
		case FL_MOVE:
		case FL_DRAG:
			wantEvent = container->mouseMove(Fl::event_x(), Fl::event_y(), lastMouseButton);
			break;
		case FL_RELEASE:
			lastMouseButton = 0;
			wantEvent = container->mouseUp(Fl::event_x(), Fl::event_y());
			break;
	}

	if (wantEvent) {
		refresh();
		return 1;
	} else
	#ifdef CSMOD_GLGUI
	return Fl_Window::handle(event);
	#else
	return Fl_Double_Window::handle(event);
	#endif
}

void CSmod::draw()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmod::draw()\n");
	#endif

	make_current();

	#ifdef CSMOD_GLGUI
	gl_start();
	#endif

	// draw current container
	container->fadeView((running)? 0.3:1.0);
	container->draw(w(),h());

	// show all external windows
	if ((rootContainer)&&(showExternalWindows))
		rootContainer->drawExternalWindows();

	#ifdef CSMOD_GLGUI
	gl_finish();
	#endif
}


void CSmod::showText(const char *title, const char *text)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmod::showText(\"%s\")\n",text);
	#endif

	textBox->label(title);
	textBuffer->text(text);
	textWindow->show();
}


char *CSmod::selectFile(bool save, const char *dlgLabel)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmod::selectFile(%d)\n",save);
	#endif
/*
	Fl_File_Chooser *browser
		= new Fl_File_Chooser(".", "CS Patches (*.csmod)", FL_SINGLE, "select patch");
*/
	if (save)
	{
		browser->label((dlgLabel)? dlgLabel:"save patch");
		browser->ok_label("save");
		browser->type(2);
	} else {
		browser->label((dlgLabel)? dlgLabel:"load patch");
		browser->ok_label("load");
		browser->type(0);
	}

	browser->preview(0); // turn preview off

	disableExternalWindows();

	// show and loop
	browser->show();
	while (browser->visible()==1) Fl::wait();

	enableExternalWindows();

	// return the correct string
	char *c=0;
	if (browser->value())
	{
		c = copyString(browser->value());
		//char *c = const_cast<char*>(browser->value());
		if (save && (c)) c = fl_filename_setext(c, strlen(c)+7, ".csmod");
	};

	return c;
}

void CSmod::disableExternalWindows()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmod::disableExternalWindows()\n");
	#endif
	showExternalWindows = false;
}

void CSmod::enableExternalWindows()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmod::enableExternalWindows()\n");
	#endif
	showExternalWindows = true;
}





	// ------------------------- midi ------------------------------------------------
#ifdef CSMOD_USE_MIDI
void CSmod::setMidiInDevice(int deviceId)
{
	#ifdef CSMOD_DEBUG
	printf("CSmod::setMidiInDevic(%d)\n", deviceId);
	#endif

	CSmidi_closeInput(midiInHandle);

	if (deviceId<0) return;

	printf("opening midi input...\n");
    midiInHandle = CSmidi_openInput(deviceId, (DWORD)CSmidiInCallback, (DWORD)this);
	if (midiInHandle==0)
	{
		printf("error opening midi device...\n");
	}
}

void CSmod::onMidiIn(DWORD data1, DWORD data2)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmod::onMidiIn(%p, %p)\n",data1,data2);
	#endif
	if (rootContainer) rootContainer->onMidiIn(data1,data2);
}

void CALLBACK CSmidiInCallback(HMIDIIN handle, UINT msg, DWORD data, DWORD par1, DWORD par2)
{
	//printf("midi: handle %p, msg %p, %p, %p %p\n",handle,msg,data,par1,par2);
	CSmod *c = (CSmod*)data;
	if (c) c->onMidiIn(par1, par2);
}

#endif
	// ------------------------ audio ------------------------------------------------


void CSmod::setAudioDevice(int inDevice, int outDevice, int rate)
{
	#ifdef CSMOD_DEBUG
	printf("CSmod::setAudioDevice(%d, %d, %d)\n", inDevice, outDevice, rate);
	#endif

	stop();

	const PaDeviceInfo *dinf;

	// ----- setup output ------

	sampleRate = rate;
	if (rootContainer) rootContainer->setSampleRate(sampleRate);

	// close any open stream
	if (streamOut) Pa_CloseStream(streamOut);
	streamOut = 0;

	// close any open stream
	if (streamIn) Pa_CloseStream(streamIn);
	streamIn = 0;


	// setup output
	if (outDevice>=0)
	{
		dinf = Pa_GetDeviceInfo(outDevice);
		PaStreamParameters param;
		param.device = outDevice;
		param.channelCount = dinf->maxOutputChannels;
		param.sampleFormat = paFloat32;
		param.suggestedLatency = dinf->defaultLowOutputLatency;
		param.hostApiSpecificStreamInfo = 0;

		PaError err = Pa_OpenStream(
			&streamOut,
			0, // no input
			&param,
			sampleRate,
			bufferSizeOut, // buffer size
			paClipOff,
			CSaudioOutCallback,
			// pass the CSmod class as user data
			(void*)this);

		if (err!=paNoError) {
			printf("error setting up output device...\n");
			streamOut = 0;
			return;
		}
		nrChannelsOut = param.channelCount;
	}


	// ----- setup input ------

	if (inDevice>=0)
	{
		dinf = Pa_GetDeviceInfo(inDevice);
		PaStreamParameters param;
		param.device = inDevice;
		param.channelCount = dinf->maxInputChannels;
		param.sampleFormat = paFloat32;
		param.suggestedLatency = dinf->defaultLowInputLatency;
		param.hostApiSpecificStreamInfo = 0;

		PaError err = Pa_OpenStream(
			&streamIn,
			&param,
			0, // no output
			sampleRate,
			bufferSizeIn, // buffer size
			paClipOff,
			CSaudioInCallback,
			// pass the CSmod class as user data
			(void*)this);

		if (err!=paNoError) {
			printf("error setting up input device...\n");
			streamIn = 0;
			return;
		}
		nrChannelsIn = param.channelCount;

		// create root container's input buffer
		if (rootContainer->inputSamples) free(rootContainer->inputSamples);
		rootContainer->inputSampleSize = nrChannelsIn * bufferSizeIn * 16;
		rootContainer->inputSamples = (float*) calloc(rootContainer->inputSampleSize, sizeof(float));
		rootContainer->inputSamplePos = 0;
		rootContainer->inputSampleReadPos = 0;
	}

}

void CSmod::closeAudioDevice()
{
	#ifdef CSMOD_DEBUG
	printf("CSmod::closeAudioDevice()\n");
	#endif

	if (streamIn) Pa_CloseStream(streamIn); streamIn = 0;
	if (streamOut) Pa_CloseStream(streamOut); streamOut = 0;

#ifdef CSMOD_USE_MIDI
    CSmidi_closeInput(midiInHandle);
#endif

}



void CSmod::start()
{
	#ifdef CSMOD_DEBUG
	printf("CSmod::start()\n");
	#endif

	if (streamOut) {

		if (rootContainer) rootContainer->start();

		if (streamIn) {
			Pa_StartStream(streamIn);
			printf("start audio input..\n");
		} else printf("no input device selected...\n");

		Pa_StartStream(streamOut);
		printf("start audio output..\n");

	} else printf("no output device selected..\n");

	if (streamIn||streamOut)
	{
		running = true;
		label(CSMOD_TITLE " running...");
		Fl::add_timeout(screenUpdateTime, CSscreenCallback, (void*)this);
	}
}

void CSmod::stop()
{
	#ifdef CSMOD_DEBUG
	printf("CSmod::stop()\n");
	#endif

	if (streamIn) { printf("stop audio in..\n"); Pa_StopStream(streamIn); }
	if (streamOut) { printf("stop audio out..\n"); Pa_StopStream(streamOut); }
	Fl::remove_timeout(CSscreenCallback);
	running = false;
	label(CSMOD_TITLE);
}


void CSmod::step(void *output, unsigned long frameCount)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmod::step(0x%p, %li)\n",output,frameCount);
	#endif

	// we read from the root container's outputSample* buffer
	if (!rootContainer) return;

	isStep = true;

	// pointer to buffer data
	float
		*out = (float*)output,
		*rout;

	// calculate frameCount samples and fill the *output buffer
	for (unsigned long i=0;i<frameCount;i++)
	{
		// do one calculation step, fill sample buffer
		rootContainer->step();

		// copy output sample(s)
		rout = rootContainer->outputSample;
		for (int j=0;j<nrChannelsOut;j++)
			*out++ = *rout++;
	}

	isStep = false;
}

void CSmod::passInput(const void *input, unsigned long frameCount)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmod::passInput(0x%p, %li)\n",input,frameCount);
	#endif

	if (!streamIn) return;

	// we write to the root container's inputSamples* buffer
	if (!rootContainer) return;
	if (!rootContainer->inputSamples) return;

	// pointer to buffer data
	float *in = (float*)input;

	// fill ring buffer
	int p = rootContainer->inputSamplePos;
	for (unsigned long i=0;i<frameCount;i++)
	{
		for (int j=0;j<nrChannelsIn;j++)
			rootContainer->inputSamples[p++] = *in++;

		if (p>=rootContainer->inputSampleSize) p = 0;
	}
	rootContainer->inputSamplePos = p;
}



void CSmod::stepLow()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmod::stepLow()\n");
	#endif

	if (rootContainer) rootContainer->stepLow();
}




void CSmod::render(const char *pathname, float sec, int nrChan, int sr, int fps,
	bool renderAudio, bool renderGraphics, bool multiChannel)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmod::render(%s, %f, %i)\n", pathname, sec, fps);
	#endif

	if ((!rootContainer) || (!(renderAudio||renderGraphics)))
	{
		printf("nothing to render...\n");
		return;
	}

	stop();

	// keep old settings

	int oldNrChan = nrChannelsOut;
	int oldSampleRate = sampleRate;

	// set new settings for render session

	if (sampleRate != sr) rootContainer->setSampleRate(sr);
	sampleRate = sr;
	nrChannelsOut = nrChan;



	printf("---rendering %g seconds (%s%s%s) @ %dhz / %d fps...\n",
		sec,
		(renderAudio)? "audio":"",
		(renderAudio && renderGraphics)? " and ":"",
		(renderGraphics)? "video":"",
		sampleRate, fps);
	printf("nr audio tracks : %d %s\n", nrChannelsOut,
		(multiChannel)? "(in one file)":"" );


	int
	// samples per frame
		spf = sampleRate / fps,
	// length in samples
		sl = sec * sampleRate;

	// file/path things

	// the audiotracks / wavewriters
	char wavename[4096];
	CSwaveWriter *wavewriter[nrChannelsOut];
	if (renderAudio)
	{
		printf("allocating wave tracks...\n");
		// separate mono waves
		if (!multiChannel)
		{
			for (int i=0;i<nrChannelsOut;i++)
			{
				sprintf(wavename, "%saudio_%02i.wav",pathname,i+1);
				wavewriter[i] = new CSwaveWriter();
				wavewriter[i]->open(wavename,sampleRate, 16, 1);
			}
		} else
		// multi channel wave
		{
			sprintf(wavename, "%saudio_%02i_chan.wav",pathname,nrChannelsOut);
			wavewriter[0] = new CSwaveWriter();
			wavewriter[0]->open(wavename,sampleRate,16,nrChannelsOut);
		}
	}

	// collect external windows
	CSmoduleList *winmod=0;
	if (renderGraphics)
	{
		winmod = rootContainer->getExternalWindowModules();
		printf("found %d graphic window%s\n", winmod->nr, (winmod->nr==1)?"":"s");
	}
	// buffer for filename for graphics
	char winname[4096];

	// audio buffer
	float
		*outbuf = (float*) calloc(nrChannelsOut * spf, sizeof(float));



	// 			--- start ---

	printf("start...\n");
	rootContainer->start();
	rootContainer->isRendering = true;
	int s = 0;

	time_t
		curTime = 0,
		startTime = timeGetTime(),
		frameStartTime = 0,
		frameTime = 0,
		timeRemaining = 0;
	float
		framesPerSecond;

	// calculation loop
	int frame = 0;
	while (s<sl)
	{
		frameStartTime = timeGetTime();

		// calculate machine
		stepLow();
		step(outbuf, spf);

		// write wave tracks
		if (renderAudio)
		{
			/*
				outbuf with 3 channels looks like this:
					123123123123123123...
					= (spf * nrChannelsOut) samples
			*/

			// separate tracks
			if (!multiChannel)
			{
				float *ob = outbuf;
				for (int i=0;i<nrChannelsOut;i++)
				{
					// store each channel
					wavewriter[i]->addFloat(ob, spf, nrChannelsOut);
					ob++; // shift to next channel
				}
			} else
			// single file
			{
				wavewriter[0]->addFloat(outbuf, spf*nrChannelsOut);
			}
		}

		// draw windows
		if (renderGraphics)
		for (int i=0;i<winmod->nr;i++)
		if ((!winmod->m[i]->parent)||
			(winmod->m[i]->parent->active) )
		{
			sprintf(winname, "%sgfx_%02i_%08i.png", pathname, i, frame);
			winmod->m[i]->drawAndSaveWindow(winname);
		}

		s += spf;
		frame++;

		// messure time

		curTime = timeGetTime();
		frameTime = curTime - frameStartTime;
		if (frameTime>0)
			framesPerSecond = (float)((int)(1000.0 / frameTime * 100.0)) / 100.0;
		else
			framesPerSecond = 1000.0;

		timeRemaining = (float)(curTime - startTime) / s * (sl-s) / 1000.0;

		printf("\r%3i%% / %g fps / takes %lih:%02lim:%02lis          ",
			(int)((float)s/sl*100.0), framesPerSecond,
			timeRemaining/60/60, (timeRemaining/60)%60, timeRemaining%60);
	}
	printf("\n");
	rootContainer->isRendering = false;
	free(outbuf);

	// 		--- finished ---



	// close audio files
	if (renderAudio)
	{
		if (!multiChannel)
		for (int i=0;i<nrChannelsOut;i++)
		{
			wavewriter[i]->close();
			delete wavewriter[i];
		}
		else
		{
			wavewriter[0]->close();
			delete wavewriter[0];
		}
	}

	// set the settings back

	if (sampleRate != oldSampleRate) rootContainer->setSampleRate(oldSampleRate);
	sampleRate = oldSampleRate;
	nrChannelsOut = oldNrChan;

	// through...

	printf("OK.\n");
}







void CSonAddModule(void *userData1, void * /* userData2 */)
{
	//printf("yes");
	theCSmod->addModule((CSmodule*)userData1);
}






void CSscreenCallback(void *userData)
{
	CSmod *csmod = static_cast<CSmod*>(userData);
	if (csmod)
	{
		if (!csmod->ready) return;
		csmod->stepLow();
		csmod->redraw();
		Fl::repeat_timeout(csmod->screenUpdateTime, CSscreenCallback, userData);
	}

}


int CSaudioOutCallback (
    const void *,//input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* ,//timeInfo,
    PaStreamCallbackFlags, // statusFlags,
    void *userData )
{

	CSmod *csmod = static_cast<CSmod*>(userData);
	if (csmod&&csmod->ready) csmod->step(output, frameCount);

	return paContinue;
}


int CSaudioInCallback (
    const void *input,
    void *,//output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo*,// timeInfo,
    PaStreamCallbackFlags,// statusFlags,
    void *userData )
{

	CSmod *csmod = static_cast<CSmod*>(userData);
	if (csmod&&csmod->ready) csmod->passInput(input, frameCount);

	return paContinue;
}



int CSaudioInOutCallback (
    const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo*,// timeInfo,
    PaStreamCallbackFlags,// statusFlags,
    void *userData )
{

	CSmod *csmod = static_cast<CSmod*>(userData);

	if (csmod&&csmod->ready) csmod->passInput(input, frameCount);
	if (csmod&&csmod->ready) csmod->step(output, frameCount);

	return paContinue;
}

