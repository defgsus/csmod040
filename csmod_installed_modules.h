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
static CSmodule
	*installedModules[] =
	{
		new CSmodule_Abs(),
		new CSmodule_AudioIn(),
		new CSmodule_AudioOut(),
		new CSmodule_Band(),
		new CSmodule_Beta(),
		new CSmodule_BitIn(),
		new CSmodule_BitOut(),
		new CSmodule_CA(),
		new CSmodule_Camera(),
		new CSmodule_ClipHard(),
		new CSmodule_ClipSoft(),
		new CSmodule_Clock(),
		new CSmodule_ClockDiv(),
		new CSmodule_Const(),
		new CSmodule_Container(),
		new CSmodule_ContainerInput(),
		new CSmodule_ContainerOutput(),
    #ifdef CSMOD_USE_MIDI
		new CSmodule_ControllerIn(),
    #endif
		new CSmodule_Comp(),
		new CSmodule_Counter(),
		new CSmodule_CounterFloat(),
		new CSmodule_SampleCounter(),
		new CSmodule_CounterSec(),
		new CSmodule_Cube(),
		new CSmodule_Delay(),
		new CSmodule_DelayOneSample(),
		new CSmodule_DelaySam(),
		new CSmodule_DelaySpread(),
		new CSmodule_Dist(),
		new CSmodule_DistFactor(),
		new CSmodule_EnvD(),
		new CSmodule_EnvAD(),
		new CSmodule_EnvADMulti(),
		new CSmodule_Eq(),
		new CSmodule_Fade(),
		new CSmodule_FadeFF(),
		new CSmodule_FadeMulti(),
		new CSmodule_FadeMultiFF(),
		new CSmodule_Filter(),
		new CSmodule_Filter24(),
		new CSmodule_Fisheye(),
		new CSmodule_FFT(),
		new CSmodule_Follow(),
		new CSmodule_FollowMulti(),
		new CSmodule_GateDelay(),
		new CSmodule_GateHold(),
		new CSmodule_GateRepeat(),
		new CSmodule_GHM(),
		new CSmodule_Hsb2Rgb(),
		new CSmodule_Logic(),
		new CSmodule_LogicOR(),
		new CSmodule_LogicXOR(),
		new CSmodule_Mag(),
		new CSmodule_Math(),
		new CSmodule_MathSub(),
		new CSmodule_MathDiv(),
		new CSmodule_MathMul(),
		new CSmodule_Merge(),
#ifdef CSMOD_USE_MIDI
        new CSmodule_MidiMerge(),
		new CSmodule_MidiFile(),
#endif
		new CSmodule_Mixer(),
		new CSmodule_Modulo(),
		new CSmodule_MultiSin(),
		new CSmodule_NN(),
		new CSmodule_NNLSM(),
#ifdef CSMOD_USE_MIDI
		new CSmodule_NoteIn(),
		new CSmodule_NoteInPoly(),
#endif
		new CSmodule_NoiseWhite(),
		new CSmodule_NoisePitch(),
		new CSmodule_OnChange(),
		new CSmodule_OnRender(),
		new CSmodule_Osc(),
		new CSmodule_OscPOut(),
		new CSmodule_OscLU(),
		new CSmodule_OscP(),
		new CSmodule_Osc3(),
		new CSmodule_OscSpec(),
		new CSmodule_QuadPos(),
		new CSmodule_QuadMix(),
		new CSmodule_QuadMixMod(),
		new CSmodule_Quant(),
		new CSmodule_P2F(),
		new CSmodule_PitchDetect(),
		new CSmodule_PlayWave(),
		new CSmodule_PointMatrix(),
		new CSmodule_PolyOsc(),
		new CSmodule_Power(),
		new CSmodule_Reverb(),
		new CSmodule_Rnd(),
		new CSmodule_RndTrig(),
		new CSmodule_Rotate(),
		new CSmodule_RotateAxis(),
		new CSmodule_SH(),
		new CSmodule_SHfollow(),
		new CSmodule_SHmulti(),
		new CSmodule_SHpoly(),
		new CSmodule_SampleGate(),
		new CSmodule_SampleRate(),
		new CSmodule_SamplerPoly(),
		new CSmodule_Scope2d(),
		new CSmodule_Scope3d(),
#ifdef CSMOD_USE_GL
        new CSmodule_ScopeGl(),
#endif
        new CSmodule_Select(),
		new CSmodule_SelectInNN(),
		new CSmodule_SelectOut(),
		new CSmodule_Seq(),
		new CSmodule_Sin(),
		new CSmodule_Sin1(),
		new CSmodule_SOM(),
		//new CSmodule_Sin2Cos(),
		new CSmodule_StackFILO(),
		new CSmodule_StarSphere(),
		new CSmodule_StartGate(),
		new CSmodule_Sum(),
		new CSmodule_SwitchGate(),
		new CSmodule_Switch(),
		new CSmodule_SwitchMatrix(),
		new CSmodule_Synth(),
		new CSmodule_SynthMulti(),
		new CSmodule_SynthMultiQuadPos(),
		new CSmodule_SynthMulti3D(),
		new CSmodule_SynthMultiFilter3D(),
		new CSmodule_Table(),
		new CSmodule_Timeline(),
		new CSmodule_ValueMeter(),
		new CSmodule_View7(),
		new CSmodule_Pos2D(),
		new CSmodule_Pos3D()
	};



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
