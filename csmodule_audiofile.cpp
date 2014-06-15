#include "csmodule_audiofile.h"


char *CSmodule_PlayWave::docString()
{
	const char *r = "\
this module lets you stream a RIFF WAVE format audio file from disk. the buffer is currently a bit buggy \
so you might experience some hangs.\n\n\
a value greater than zero in <play> will forward the play position and output the sample.\n\
a <pitch> of 1.0 is default, 0.5 will be half speed, 2.0 double, and -1.0 will play reversed.\n\
a positive edge in <reset> will reset the play position to the value in <position (sec)> which is \
in seconds.\n\n\
the wave file can be selected in the property view. the module appears redish when no file is loaded.\n\
currently, the wave is always looped.\
";
	return const_cast<char*>(r);
}


CSmodule_PlayWave::CSmodule_PlayWave()
{
	clear();
	wr = new CSwaveReader();
}

CSmodule_PlayWave::~CSmodule_PlayWave()
{
	if (wr) delete wr;
}

CSmodule_PlayWave* CSmodule_PlayWave::newOne()
{
	return new CSmodule_PlayWave();
}


void CSmodule_PlayWave::init()
{
	gname = copyString("sampler");
	bname = copyString("play wave");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- look ----

	headerHeight = 32;

	// --- inputs outputs ----

	_play = &(createInput("p","play")->value);
	_pitch = &(createInput("pi","pitch",1.0)->value);
	_rst = &(createInput("r","reset")->value);
	_pos = &(createInput("po","position (sec)")->value);

	_outP = &(createOutput("outp","position (sec)")->value);
	_outM = &(createOutput("outm","mono")->value);

	char n[32],n1[32];
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n,"out%d",i);
		sprintf(n1,"ch %d",i+1);
		__out[i] = createOutput(n,n1);
		_out[i] = &(__out[i]->value);
	}

	// ---- properties ----

	createNameProperty();
	createPropertyString("fn", "file name", "-");

	// --- intern ---

	setNrOut(1);

	oldBodyColor = bodyColor;
	CSsetColor(invBodyColor, 120,100,100);

	lrst = 0.0;

	wr = new CSwaveReader();
}


void CSmodule_PlayWave::propertyCallback(CSmodProperty *prop)
{
	if (!strcmp(prop->name, "fn"))
		openFile(prop->svalue);
	else
		CSmodule::propertyCallback(prop);
}

void CSmodule_PlayWave::setNrOut(int newNrOut)
{
	nrOut = newNrOut;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__out[i], (i<nrOut));
	}

	arrangeConnectors();
}


void CSmodule_PlayWave::openFile(const char *filename)
{
	bodyColor = invBodyColor;
	updateColors();
	CSwait();

	//setNrOut(1);

	if (!wr->open(filename)) return;

	setNrOut(wr->nrchan);

	bodyColor = oldBodyColor;
	updateColors();

}



void CSmodule_PlayWave::step()
{
	if (!wr->file) return;

	if ((lrst<=0.0)&&(*_rst)) playpos = *_pos * wr->rate;
	lrst = *_rst;

	if (*_play>0.0)
	{
		*_outP = playpos * isampleRate;

		wr->getSamples(playpos, 2, samples);
		*_outM = 0.0;
		csfloat *s = samples;
		for (int i=0;i<nrOut;i++)
		{
			*_outM += *s;
			*_out[i] = *s;
			s++;
		}
		*_outM /= nrOut;

		// move play position
		playpos	+= *_pitch * wr->rate * isampleRate;
		// wrap around
		if (playpos<0)
			playpos += wr->datalength;
		else
		if (playpos>=wr->datalength)
			playpos -= wr->datalength;
	}



}



void CSmodule_PlayWave::stepLow()
{
	wr->updateBuffer();
}



void CSmodule_PlayWave::draw(int offx, int offy, float zoom)
{
	// draw original module
	CSmodule::draw(offx,offy,zoom);

	if (!wr->file) return;

	int sx = offx+zoom*(x+5);
	int sw = zoom*(width-10);
	int sy = offy+zoom*(y+20);
	int sh = zoom*10;
	// background
	fl_color((wr->requestRead>=0)? FL_RED:FL_BLACK);
	fl_rectf(sx,sy,sw,sh);
	// buffer load
	fl_color(FL_DARK_BLUE);
	fl_rectf(sx+sw*(float)wr->pos/wr->datalength, sy,
			    sw*(float)wr->buflength/wr->datalength, sh);
	// playpos
	fl_color(FL_WHITE);
	fl_yxline(sx+sw*(float)playpos/wr->datalength, sy, sy+sh);
}
