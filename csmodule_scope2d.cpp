#include <algorithm>
#include "csmodule_scope2d.h"

char *CSmodule_Scope2d::docString()
{
	const char *r = "\
this module is used to visualize a number of inputs in two dimensions, time and amplitude.\n\n\
the main amplitude for all input channels is set with <amplitude>. the y-axis of the display \
is always clipped to -1.0 and 1.0, so f.e. to view a signal which ranges between -10 and 10 set \
amplitude to 0.1.\n\
the timespan of the displayed curve (the time displayed on screen) is set with \
the <second> input, and is given of course in seconds. the resolution can be set down to \
about 0.01 sec.\n\
each channel has an <in x> input. the number of channels is set in the \
property window ('p' or double-click). the color for each channel is randomized at module creation time.\n\n\
if the <sync> input is connected, the buffer will be displayed from left to right, and it's write \
position will be reset with a positive edge in <sync>. that way you can view f.e. a single period of \
a waveform.\
";
	return const_cast<char*>(r);
}


CSmodule_Scope2d::~CSmodule_Scope2d()
{
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
		releaseTimeline(scope[i]);
	free(scope);
}

CSmodule_Scope2d* CSmodule_Scope2d::newOne()
{
	return new CSmodule_Scope2d();
}


void CSmodule_Scope2d::init()
{
	gname = copyString("analyze");
	bname = copyString("scope2d");
	name = copyString("scope 2d");
	uname = copyString(name);

	// ----- look ----

	minWidth = 400; width = minWidth;
	minHeight = 138; height = minHeight;
	sx = 60;
	sy = headerHeight+5;
	sw = width-sx-5;
	sh = height-sy-5;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
		scopeColor[i] =
			((rand()%0x7f) << 12) |
			((rand()%0x7f) << 18) |
			((rand()%0x7f) << 24);

	// --- inputs outputs ----

	_amp = &(createInput("amp","amplitude",1.0)->value);
	_offset = &(createInput("off","offset",0.0)->value);
	_width = &(createInput("width","seconds",10.0)->value);
	__rst = createInput("rst","sync");
	_rst = &(__rst->value);

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("in",i);
		char *n1 = int2char("in ",i+1);
		__in[i] = createInput(n,n1);
		free(n); free(n1);
		_in[i] = &(__in[i]->value);
	}

	oldNrIn = 0;
	setNrIn(2);

	// ---- properties ----

	createNameProperty();

	createPropertyInt("nrin", "nr of inputs", nrIn, 1,CSMOD_MAX_CHAN);

	// ---- internals -----

	scope = (CStimeline**) calloc(CSMOD_MAX_CHAN, sizeof(int));
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
		scope[i] = createTimeline(sw*2);
	dpos = 0.0;

	lrst = 0.0;
	offset = 0;
}


void CSmodule_Scope2d::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);

	CSmodule::propertyCallback(prop);
}

void CSmodule_Scope2d::setNrIn(int newNrIn)
{
	if (newNrIn==oldNrIn) return;
	nrIn = newNrIn;
	oldNrIn = nrIn;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__in[i], (i<nrIn));
	}

	arrangeConnectors();
}


void CSmodule_Scope2d::step()
{
	// increase buf pos
	dpos += max(0.00000001f,min(1.0f, scope[0]->length/( sampleRate * max(0.000001f, *_width)) ));
	if (dpos>=scope[0]->length) dpos-=scope[0]->length;

	// reset buf pos
	if ((lrst<=0.0)&&(*_rst>0.0)) { dpos = 0; }
	lrst = *_rst;

	for (int i=0;i<nrIn;i++)
	{
		scope[i]->pos = (int)dpos;
		scope[i]->data[scope[i]->pos] = *_offset + *_amp * *_in[i];
	}
}


void CSmodule_Scope2d::draw(int offx, int offy, float zoom)
{
	CSmodule::draw(offx,offy,zoom);

	int X=offx+zoom*(sx+x);
	int Y=offy+zoom*(sy+y);
	int W=zoom*sw;
	int H=zoom*sh;
	int a=(__rst->nrCon != 0);
	for (int i=0;i<nrIn;i++)
	{
		fl_color(scopeColor[i]);
		drawTimeline(scope[i], X,Y,W,H, (i==0), 0x20202000, -scope[i]->pos*a);
	}
}
