#include <math.h>
#include "csmodule_table.h"

char *CSmodule_Table::docString()
{
	const char *r = "\
the module lets you paint a sequence/table of variable length and outputs a requested value.\n\n\
<nr steps> is interpreted as integer and defines the length of the sequence (from 1 - 256).\n\
all values drawn in the sequencer window are between 0.0 and 1.0. the output however will be in the \
range of <offset> to <offset> + <amplitude>.\n\n\
<pos> defines the readout position between 0.0 and 1.0. the value is wrapped around, thus can be greater \
than 1.0 (or smaller 0.0). a 0.0 means output the first table value and 1.0 again is wrapped around to 0.0.\n\n\
<bank (read)> selects the sequence which should be output. there can be up to 256 sequences in the module. \
a non-integer value will mix the values of two banks together. f.e. 2.3 will give 70% of bank #2 and 30% of bank #3.\n\
<bank (edit)> is interpreted as integer and selects the sequence you want to edit/draw. presumably you \
won't modulate this input in the general case.\n\
currently all banks/sequences have the same length as defined by <nr steps>\n\n\
in <output> the current value will be output all the time.\n\n\
the following outputs are only calculated when connected:\n\
in <out linear> a linearly interpolated version will be output.\n\
<out smooth> is also an interpolator between two data points, but with a sigmoid kind of transient.\n\
<out cubic> is a good quality cubic interpolation between 4 data points. NOTE: the output value might have \
a slightly larger/smaller amplitude than the table value at some points.\n\n\
note that the audio engine has to be running in order to make changes of the <nr steps> value be effective.\
";
	return const_cast<char*>(r);
}


CSmodule_Table* CSmodule_Table::newOne()
{
	return new CSmodule_Table();
}



void CSmodule_Table::init()
{
	fileVersion = 2;

	gname = copyString("sequencer");
	bname = copyString("table 2d");
	name = copyString(bname);
	uname = copyString(bname);

	// --- look ----

	sw = 256;
	sh = 136;
	sx = 64;
	sy = headerHeight+12;
	minWidth = sx+sw+64;
	minHeight = sy+sh+12;

	// --- inputs outputs ----

	_steps = &(createInput("st","nr steps",16.0)->value);
	con[0]->value = con[0]->uvalue;
	_pos = &(createInput("p","pos")->value);
	_amp = &(createInput("a","amplitude",1.0)->value);
	_off = &(createInput("o","offset")->value);
	_bank = &(createInput("br","bank (read)")->value);
	_bankedit = &(createInput("be","bank (edit)")->value);

	_out = &(createOutput("ot","output")->value);
	outL = createOutput("ol","out linear"); _outL = &(outL->value);
	outS = createOutput("os","out smooth"); _outS = &(outS->value);
	outC = createOutput("oc","out cubic"); _outC = &(outC->value);

	// ---- properties ----

	createNameProperty();

	// intern

	valueStr = (char*) calloc(1024,sizeof(char));

	drawing = -1;
	lpos = -1;
	bankEdit = 0;
	memset(bankChanged, 0, CSMOD_SEQ_MAX_BANK * sizeof(bool));
	memset(data, 0, CSMOD_SEQ_MAX * CSMOD_SEQ_MAX_BANK * sizeof(csfloat));
}


void CSmodule_Table::getValue(int bank, csfloat *_out, csfloat *_outL, csfloat *_outS, csfloat *_outC)
{

}

void CSmodule_Table::step()
{
	bankEdit = max(0,min(CSMOD_SEQ_MAX_BANK-1, (int)*_bankedit ));

	// output value
	if (lpos != *_pos)
	{
		// bank select
		int ba = max(0,min(CSMOD_SEQ_MAX_BANK-1, (int)*_bank ));
		int ba1 = max(0,min(CSMOD_SEQ_MAX_BANK-1, (int)*_bank + 1 ));
		int bao = ba*CSMOD_SEQ_MAX;
		int bao1 = ba1*CSMOD_SEQ_MAX;
		csfloat bt = *_bank - ba, bt1 = 1.0-bt;

		// nr of steps
		int st = max(1,min(CSMOD_SEQ_MAX, (int)*_steps ));

		// position
		csfloat p = *_pos;
		if (p>0.0) p = MODULO(p, 1.0);
		else
		if (p<0.0) p = MODULO(-p, 1.0);

		p *= st;

		unsigned int pt = (unsigned int)p;

		// stepped out
		*_out = *_off + *_amp * data[bao + pt];
		// mix bank
		if (bt) *_out = *_out * bt1 + bt * (*_off + *_amp * data[bao1 + pt]);

		// fade between 0 and 1
		p = p-pt;

		// linear out
		if (outL->nrCon)
		{
			*_outL = *_off + *_amp * ( data[bao + pt] * (1-p) + p * data[bao + (pt+1)%st] );
			// mix bank
			if (bt) *_outL = *_outL * bt1 + bt * (
				*_off + *_amp * ( data[bao1 + pt] * (1-p) + p * data[bao1 + (pt+1)%st] )
				);
		}

		// smooth out
		if (outS->nrCon)
		{
			csfloat p1 = 5*p*p*(1-p) + powf(p,5);
			*_outS = *_off + *_amp * ( data[bao + pt] * (1-p1) + p1 * data[bao + (pt+1)%st] );
			// mix bank
			if (bt) *_outS = *_outS * bt1 + bt * (
				*_off + *_amp * ( data[bao + pt] * (1-p1) + p1 * data[bao + (pt+1)%st] )
				);
		}

		// cubic out
		if (outC->nrCon)
		{
			int pt0 = pt-1; if (pt0<0) pt0 = st-1;
			int pt2 = pt+1; if (pt2>=st) pt2 = 0;
			int pt3 = pt+2; if (pt3>=st) pt3 = min(st-1, 1);
			csfloat y0 = data[bao + pt0];
			csfloat y1 = data[bao + pt];
			csfloat y2 = data[bao + pt2];
			csfloat y3 = data[bao + pt3];
			// mix bank
			if (bt)
			{
				y0 = y0 * bt1 + bt * data[bao1 + pt0];
				y1 = y1 * bt1 + bt * data[bao1 + pt];
				y2 = y2 * bt1 + bt * data[bao1 + pt2];
				y3 = y3 * bt1 + bt * data[bao1 + pt3];
			}
			// final shape
			csfloat ys1 = (y2-y0) * 0.5;
			csfloat ys2 = (y3-y1) * 0.5;
			csfloat a1 = y2-y1;
			*_outC = *_off + *_amp * (
				y1 + (a1 + ( a1 - ys1 + (ys2-a1)*p) * (p-1) ) * p
				);
		}

	}
	lpos = *_pos;

}








void CSmodule_Table::draw(int offx, int offy, float zoom)
{
	CSmodule::draw(offx,offy,zoom);

	// background
	int X=offx+zoom*(sx+x);
	int Y=offy+zoom*(sy+y);
	int W=zoom*sw;
	int H=zoom*sh;
	fl_color(FL_BLACK);
	fl_rectf(X,Y,W,H);

	// data
	int bao = bankEdit*CSMOD_SEQ_MAX;

	int st = max(1,min(CSMOD_SEQ_MAX, (int)*_steps ));
	int W1 = W/st;
	for (int i=0;i<st;i++)
	{
		int X1 = X + i*W/st;
		int H1 = H-data[bao+i]*H;
		// bar
		fl_color(FL_GREEN);
		fl_rectf(X1,Y+H1,W1,H-H1);

		// separation line
		fl_color(FL_GRAY);
		fl_yxline(X1, Y, Y+H);
	}

	// changed value
	if (drawing>=0)
	{
		drawing = min(st-1, drawing);
		fl_color(FL_WHITE);
		fl_font(FL_HELVETICA, 10);
		sprintf(valueStr, "%g", *_off + *_amp * data[bao+drawing]);
		fl_draw(valueStr, X+2+drawing*W/st, Y+H-data[bao+drawing]*H );
	}

}
