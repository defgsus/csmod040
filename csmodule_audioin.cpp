#include "csmodule_audioin.h"
#include "csmodcontainer.h"

char *CSmodule_AudioIn::docString()
{
	const char *r = "\
this module provides the audio in interface to the system audio device.\n\
the device is choosen in the audio preferences (F10). each audio in module \
regardless of where it is placed within the patch will represent the one input buffer \
as received from the system. you can add as many audio in modules as you may need \
they all reflect the same.\n\n\
<amplitude> will scale the volume of the output of the module. however, this will not \
resolve problems of distorted audio signals if the clipping happens at the AD-stage of the \
audio hardware.\n\
<mono out> will mix all channels of the audio in.\n\
each channel has an <out x> output to access them separately.\n\
you can set the number of channels for each audio in device in it's \
property window ('p' or double-click). however, only the number of channels \
supported by the selected audio device are used.\
";
	return const_cast<char*>(r);
}


CSmodule_AudioIn* CSmodule_AudioIn::newOne()
{
	return new CSmodule_AudioIn();
}


void CSmodule_AudioIn::init()
{
	gname = copyString("IO");
	bname = copyString("audio in");
	name = copyString(bname);
	uname = copyString(bname);

	/*icon =
"................."
"................."
"................."
"................."
"...............x."
"...............x."
"...........x...x."
"...........xx..x."
"..xxxxxxxxxxxx.x."
"...........xx..x."
"...........x...x."
"...............x."
"...............x."
"................."
"................."
"................."
".................";
*/

	// --- inputs outputs ----

	_amp = &(createInput("a","amplitude",1.0)->value);
	_outmono = &(createOutput("outm","mono out")->value);

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("out",i);
		__out[i] = createOutput(n,"out");
		free(n);
		_out[i] = &(__out[i]->value);
	}

	oldNrOut = 0;
	setNrOut(1);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrchan", "nr of channels", nrOut, 1,CSMOD_MAX_CHAN);

	// --- look ----
	/*
	minWidth = 80;
	width = 80;
	resize();
	*/

	CSsetColor(headerColor, 90,50,50);
	CSsetColor(headerFocusColor, 160,110,110);
	CSsetColor(bodyColor, 120,80,80);
	updateColors();

	// -- internals --


}


void CSmodule_AudioIn::setNrOut(int newNrOut)
{
	if (newNrOut==oldNrOut) return;
	nrOut = newNrOut;
	oldNrOut = nrOut;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__out[i], (i<nrOut));
	}

	arrangeConnectors();

	/* also set the property here, because the container can call
	   this function to set the appropiate number of channels for
	   the audio out/in module */
	for (int i=0;i<nrProp;i++)
		if (!strcmp(property[i]->name, "nrchan"))
			property[i]->ivalue = nrOut;
}


void CSmodule_AudioIn::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrchan")==0)
		setNrOut(prop->ivalue);

	else CSmodule::propertyCallback(prop);
}


void CSmodule_AudioIn::step()
{
	if (parentRoot)
	{
		if (!parentRoot->inputSamples) return;

		// read from input sample buffer

		*_outmono = 0.0;
		for (int i=0;i<nrOut;i++)
		{
			csfloat sam = parentRoot->inputSamples[parentRoot->inputSampleReadPos++];

			*_outmono += sam;
			*_out[i] = *_amp * sam;

			// wrap read pointer
			if (parentRoot->inputSampleReadPos >= parentRoot->inputSampleSize)
				parentRoot->inputSampleReadPos = 0;
		}

		// scale mono input
		*_outmono *= *_amp / nrOut;
	}

}
