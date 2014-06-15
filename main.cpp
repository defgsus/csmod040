#if (0)

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>

class MyWindow : public Fl_Double_Window
{
public:
    MyWindow(int w, int h, const char * l)
        : Fl_Double_Window(w,h,l)
    {
        //make_current();
    }
    /*
    void draw()
    {
        make_current();
    }*/
};


int main(int argc, char **argv)
{
    //Fl_Window *window = new Fl_Window(340,180);
    Fl_Window *window = new MyWindow(340,180,"hello window");
    Fl_Box *box = new Fl_Box(20,40,300,100,"Hello, World!");
    box->box(FL_UP_BOX);
    box->labelfont(FL_BOLD+FL_ITALIC);
    box->labelsize(36);
    box->labeltype(FL_SHADOW_LABEL);
    window->end();
    window->show(argc, argv);
    return Fl::run();
}

#else

#include <stdio.h>
#include <math.h>
#include "csmod.h"

//#include "csmod_midi.h"
#include "csmod_wavereader.h"

using namespace std;


CSmod *csmod;


int main( int /*argc*/, char** /*argv*/ )
{
    //printf("%d\n",(int)((float)4.0/3)); return 0;
    /* // wave reader test

    CSwaveReader *r = new CSwaveReader();
    r->open("d:/samples/DieSendung.wav");
    printf("-> %g\n",r->getSample(100));
    r->updateBuffer();
    r->debugInfo();
    printf("-> %g\n",r->getSample(10000));
    delete r;

    return 0;
    */

    /* // midi test

    CSmidiDevices *md = CSmidi_newDeviceList();
    CSmidi_getDevices(md);
    for (int i=0;i<md->nr;i++)
        printf("%d %s\n", i, md->name[i]);
    return 0;
    */

    /*
    for (float i = 0; i<20; i++)
    {
        float j = i/19;
        printf("%8g %8f\n", j, asinf(j));
    }
    return 0;
    */

    // init installed modules / audio / other stuff
    CSmodInit();

    // create a cymasonics modular editor workbench
    csmod = new CSmod();

    // ... and display main edit window
    csmod->show();

    //csmod->loadPatch("./patches/tests/OSC.csmod");
    //csmod->loadPatch("./patches/tests/OSC_TABLE.csmod");
    //csmod->loadPatch("./patches/LICHT/cs.csmod");
    //csmod->loadPatch("./patches/tests/SCOPE3D_interpolateRGB.csmod");
    //csmod->loadPatch("./patches/tests/CAMERA.csmod");
    //csmod->loadPatch("./patches/tests/DELAY.csmod");
    //csmod->loadPatch("./patches/tests/DELAY_SPREAD.csmod");
    //csmod->loadPatch("./patches/tests/FILTER24.csmod");
    //csmod->loadPatch("./patches/tests/SEQUENCER.csmod");
    //csmod->loadPatch("./patches/tests/TABLE.csmod");
    //csmod->loadPatch("./patches/tests/NN.csmod");
    //csmod->loadPatch("./patches/tests/PLAYWAVE.csmod");
    //csmod->loadPatch("./patches/LICHT/CSFORM.csmod");
    //csmod->loadPatch("./patches/LICHT/ROTATE_HELIX_HELIX_DOME2.csmod");
    //csmod->loadPatch("./patches/LICHT/DOME.csmod");
    //csmod->loadPatch("./patches/tests/TIMELINE.csmod");
    //csmod->loadPatch("./patches/KLANG/pluckString_C6.csmod");
    //csmod->loadPatch("./patches/tests/QUADCIRCLEMODMIX.csmod");
    //csmod->loadPatch("./patches/tests/LSM.csmod");
    //csmod->loadPatch("./patches/tests/MIDIFILE.csmod");
    //csmod->loadPatch("./patches/liszt/ballade2_5.csmod");
    //csmod->loadPatch("./patches/tests/EMPTYCONTAINER.csmod");
    //csmod->loadPatch("./patches/tests/SCOPEGL.csmod");
    //csmod->loadPatch("./patches/tests/REVERB.csmod");
    //csmod->loadPatch("./patches/tests/SYNTH.csmod");
    //csmod->loadPatch("./patches/tests/OSC3.csmod");
    //csmod->loadPatch("./patches/tests/SCOPE3d_opacity.csmod");
    //csmod->loadPatch("./patches/tests/SYNTH3D.csmod");
    //csmod->loadPatch("./patches/tests/SCOPE3d_subsamples.csmod");
    //csmod->loadPatch("./patches/tests/SCOPE3d_client4.csmod");
    //csmod->loadPatch("./patches/tests/SAMPLER_POLY.csmod");
    //csmod->loadPatch("./patches/tests/LSM3.csmod");
    //csmod->loadPatch("./patches/tests/SOM.csmod");

    // right away ask for audio device selection
    csmod->audioWindow->show();

    int ret = 0;
    // start event loop
    try
    {
        ret = Fl::run();
    }
    catch (...)
    {
        printf("EXCEPTION!!\n");
    }

    // finalize stuff

    csmod->closeAudioDevice();
    CSmodDestroy();
    delete csmod;


    // quit with FLTK's event loop return code
    return ret;
}

#endif
