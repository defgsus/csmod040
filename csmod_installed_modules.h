/**
 *
 * CS MODULAR EDITOR
 *
 * collection of all possible module classes
 * + a choose window
 *
 * DEFINES:
 *	CSmodule *installedModules[] - instantiated classes of all available modules
 *  int nrInstalledModules
 *
 * NOTE:
 *   - add new modules in csmod_installed_modules.cpp
 *   - call releaseInstalledModules() at end of program
 *     (normally this is done by CSmodDestroy() !)
 *
 * @version 2010/09/26 v0.1
 * @author def.gsus-
 *
 */

#ifndef CSMOD_INSTALLED_MODULES_H_INCLUDED
#define CSMOD_INSTALLED_MODULES_H_INCLUDED

#include "FL/Fl.H"
#ifdef CSMOD_USE_DOUBLE_WINDOW
#   include "FL/Fl_Double_Window.H"
#else
#   include "FL/Fl_Window.H"
#endif
#include "FL/Fl_Scroll.H"
#include "FL/Fl_Button.H"

#include "csmodule.h"
#include "csmod.h"


// list of module headers (add here...)
#include "csmodule_analyze.h"
#include "csmodule_audioin.h"
#include "csmodule_audioout.h"
#include "csmodule_audiofile.h"
#include "csmodule_beta.h"
#include "csmodule_band.h"
#include "csmodule_bit.h"
#include "csmodule_ca.h"
#include "csmodule_camera.h"
#include "csmodule_clip.h"
#include "csmodule_clockdiv.h"
#include "csmodule_const.h"
#include "csmodule_container.h"
#include "csmodule_control.h"
#include "csmodule_comp.h"
#include "csmodule_delay.h"
#include "csmodule_env.h"
#include "csmodule_filter.h"
#include "csmodule_fft.h"
#include "csmodule_logic.h"
#include "csmodule_math.h"
#include "csmodule_mixer.h"
#include "csmodule_midi.h"
#include "csmodule_nn.h"
#include "csmodule_objects.h"
#include "csmodule_osc.h"
#include "csmodule_p2f.h"
#include "csmodule_panning.h"
#include "csmodule_rnd.h"
#include "csmodule_rotate.h"
#include "csmodule_select.h"
#include "csmodule_sh.h"
#include "csmodule_scope2d.h"
#include "csmodule_scope3d.h"
#include "csmodule_scopegl.h"
#include "csmodule_seq.h"
#include "csmodule_system.h"
#include "csmodule_sum.h"
#include "csmodule_sampler.h"
#include "csmodule_synth.h"
#include "csmodule_table.h"

/** list of all modules availabe (add here...) */
extern CSmodule
    *installedModules[];


/** maximum of possible module groups */
#define CSMOD_MAX_GROUP 1024
/** maximum modules in one group */
#define CSMOD_MAX_MODULE 1024

/** initialize each module in list */
void initInstalledModules();

/** call this before program termination */
void releaseInstalledModules();

extern int
	/** the number of different modules in @see installedModules[] */
    nrInstalledModules,

	csNrGroups,
	csNrModulesGroup[CSMOD_MAX_GROUP];

extern const char
	*csGroupName[CSMOD_MAX_GROUP],
    *csUngroupedString;

extern CSmodule
	*csGroup[CSMOD_MAX_GROUP][CSMOD_MAX_MODULE];


extern CSmodPopUp
	/** the main popup menu for holding modules */
	*CSmodulePopUp,
	/** popups for each group */
	*CSmoduleSubPopUp[1024];


/** return NULL or a link to apropiate intalledModules[] matching bname.
	you MUST NOT delete the received module! */
CSmodule *getInstalledModule(const char *bname);

/** return the number of different module groups in @see csGroup[] */
int getNrInstalledModuleGroups();


/** a window displaying the installed modules */
class CSmoduleWindow : public
#ifdef CSMOD_USE_DOUBLE_WINDOW
        Fl_Double_Window
#else
        Fl_Window
#endif
{
	/** parent CSmod handler */
	CSmod *parent;

	public:
	/** constructor */
	CSmoduleWindow(CSmod *parent);

	/** place the module into container */
	void chooseModule(CSmodule *mod);

};

/** a button to choose a module */
class CSmoduleWindowButton : public Fl_Button
{
	public:
	/** parent CSmoduleWindow */
	CSmoduleWindow *modwin;
	/** assigned module */
	CSmodule *mod;
	/** constructor */
	CSmoduleWindowButton(int xx,int yy,int ww,int hh, CSmoduleWindow *modwin, CSmodule *mod);
	/** event handler (calls modwin->chooseModule) */
	int handle(int event);
};


#endif // CSMOD_INSTALLED_MODULES_H_INCLUDED
