#include "csmod_installed_modules.h"

CSmodule *installedModules[] =
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
    new CSmodule_ControllerIn(),
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
#ifdef CSMOD_USE_FFTW
    new CSmodule_FFT(),
#endif
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
    new CSmodule_MidiMerge(),
    new CSmodule_MidiFile(),
    new CSmodule_Mixer(),
    new CSmodule_Modulo(),
    new CSmodule_MultiSin(),
    new CSmodule_NN(),
    new CSmodule_NNLSM(),
    new CSmodule_NoteIn(),
    new CSmodule_NoteInPoly(),
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

/** the number of different modules in @see installedModules[] */
int nrInstalledModules = sizeof(installedModules)/sizeof(*installedModules);

int csNrGroups = 0;
int csNrModulesGroup[CSMOD_MAX_GROUP];

const char * csGroupName[CSMOD_MAX_GROUP];
const char * csUngroupedString = "---";

CSmodule * csGroup[CSMOD_MAX_GROUP][CSMOD_MAX_MODULE];


CSmodPopUp * CSmodulePopUp = 0;
CSmodPopUp * CSmoduleSubPopUp[1024];




int getNrInstalledModuleGroups()
{
	return csNrGroups;
}

/** return the milliseconds per sample step for the given module */
float getModuleTime(CSmodule *m)
{
	CSmodContainer *con = new CSmodContainer();
	con->addModule(m);

	DWORD t1=0,t2=0;
	int nrSteps = 196000;
	t1 = timeGetTime();
	con->module[0]->stepLow();
	for (int i=0;i<nrSteps;i++)
	{
		con->module[0]->step();
	}
	t2 = timeGetTime();
	delete con;

	return (float)max((DWORD)1, t2-t1) / nrSteps;
}



int getGroupIndex(const char *gname)
{
	//printf("%d %s\n",csNrGroups,gname);
	if (!gname)
	{
		for (int i=0;i<csNrGroups;i++)
			if (!strcmp(csGroupName[i], csUngroupedString)) return i;
		return -1;
	}

	for (int i=0;i<csNrGroups;i++)
	{
		//printf(".%s. .%s.\n", csGroupName[i], gname);
		if (!strcmp(csGroupName[i], gname)) return i;
	}
	return -1;
}


void initInstalledModules()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmoduleWindow::initInstalledModules()\n");
	#endif

	// init groups
	csNrGroups = 0;
	for (int i=0;i<CSMOD_MAX_GROUP;i++)
	{
		csNrModulesGroup[i] = 0;
		for (int j=0;j<CSMOD_MAX_GROUP;j++)
			csGroup[i][j] = 0;
	}

	// for each module

	for (int i=0;i<nrInstalledModules;i++)
	if (installedModules[i]->canAdd)
	{
		// init the modules
		installedModules[i]->init();
		//printf("%s\n",installedModules[i]->name);

		// collect groups
		int j = getGroupIndex(installedModules[i]->gname);
		if (j<0)
		{
			// add this group
			csGroupName[csNrGroups] = (installedModules[i]->gname)?
				installedModules[i]->gname : csUngroupedString;

			csNrGroups++;
		}
	}

	// put modules into groups

	for (int i=0;i<nrInstalledModules;i++)
	if (installedModules[i]->canAdd)
	{
		int j = getGroupIndex(installedModules[i]->gname);
		if (j<0) continue;

		csGroup[j][csNrModulesGroup[j]] = installedModules[i];
		csNrModulesGroup[j]++;
	}

	// create popUps
	/*
	CSmodulePopUp = new CSmodPopUp();
	for (int i=0;i<csNrGroups;i++)
	{
		CSmoduleSubPopUp[i] = new CSmodPopUp();
		for (int j=0;j<csNrModulesGroup[i];j++)
		{
			CSmoduleSubPopUp[i]->addItem(
				csGroup[i][j]->name, (void*)CSonAddModule, (void*)csGroup[i][j]);
		}
		CSmodulePopUp->addItem(csGroupName[i], (void*)CSmoduleSubPopUp[i]);
	}

	*/

	/*
	return;
	// get calculation time
	for (int i=0;i<nrInstalledModules;i++)
	{
		printf("%12f %s\n", getModuleTime(installedModules[i]), installedModules[i]->name);
	}
	*/

}


void releaseInstalledModules()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmoduleWindow::releaseInstalledModules()\n");
	#endif

	for (int i=0;i<nrInstalledModules;i++)
    {
        //printf("deleting %s\n", installedModules[i]->name);
		delete installedModules[i];
    }

}


CSmodule *getInstalledModule(const char *bname)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmoduleWindow::getInstalledModule(\"%s\")\n", bname);
	#endif
	for (int i=0;i<nrInstalledModules;i++)
	{
		if (strcmp(installedModules[i]->bname, bname)==0)
			return installedModules[i];
	}
	return NULL;
}


CSmoduleWindowButton::CSmoduleWindowButton(int xx,int yy,int ww,int hh, CSmoduleWindow *modwin, CSmodule *mod)
	: Fl_Button(xx,yy,ww,hh,mod->name)
{
	this->modwin = modwin;
	this->mod = mod;
}

int CSmoduleWindowButton::handle(int event)
{
	if (event==FL_RELEASE) {
		modwin->chooseModule(mod);
		modwin->hide();
	}
	return Fl_Button::handle(event);
}





CSmoduleWindow::CSmoduleWindow(CSmod *parent)
    :
    #ifdef CSMOD_USE_DOUBLE_WINDOW
      Fl_Double_Window
    #else
      Fl_Window
    #endif
      (200,480,"installed modules")

{
	#ifdef CSMOD_DEBUGE
	printf("CSmoduleWindow::CSmoduleWindow(0x%p)\n", parent);
	#endif

	this->parent = parent;

	// a scrollbar if too many modules
	Fl_Scroll *scr = new Fl_Scroll(0,0,w(),h(),"");
	scr->w(); // just to shut up compiler warning about unused symbol

	// ---- create list of modules ----

	int k=2;
	for (int i=0;i<csNrGroups;i++)
	if (strcmp("---", csGroupName[i]))
	{
		k+=5;
		// group label
		Fl_Button *l = new Fl_Button(5,k,w()-25,18, csGroupName[i]);
		l->box(FL_FLAT_BOX);
		l->labelsize(l->labelsize()+4);
		l->w(); // shut-up about unused symbol
		k+=20;
		for (int j=0;j<csNrModulesGroup[i];j++)
		{
			CSmoduleWindowButton *b = new CSmoduleWindowButton(
				5,k,w()-25,18, this, csGroup[i][j]);
			b->w();
			k+=20;
		}
	}

	/*
	int k=0;
	for (int i=0;i<nrInstalledModules;i++)
	if (installedModules[i]->canAdd)
	{
		CSmoduleWindowButton *b = new CSmoduleWindowButton(
			5,2+k*20,w()-25,18, this, installedModules[i]);
		b->w(); // shut-up about unused symbol
		k++;
	}
	*/
}



void CSmoduleWindow::chooseModule(CSmodule *mod)
{
	parent->addModule(mod);
}
