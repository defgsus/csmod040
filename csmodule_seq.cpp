#include <math.h>
#include "csmodule_seq.h"

char *CSmodule_Seq::docString()
{
	const char *r = "\
the module lets you paint a sequence of variable length and outputs that sequence according to a gate signal.\n\n\
<nr steps> is interpreted as integer and defines the length of the sequence (from 1 - 256).\n\
a positive edge in <reset> will reset the play position to the beginning of the sequence.\n\
a positive edge in <step> will forward the play position one step. it will automatically start at the \
beginning when the end of the sequence is reached.\n\n\
all values drawn in the sequencer window are in the range of <offset> to <offset> + <amplitude>. to have a \
unipolar sequence between -1.0 and 1.0, simply set <amplitude> = 2.0; <offset> = -1.0.\n\n\
in <output> the current sequence value will be output all the time.\n\
in <gate output> the sequence value will be output for the duration of one sample whenever the \
sequencer position changes and will fall back to the value of <offset> hereafter. so this output would be \
the choice to drive f.e. an envelope generator.\n\
<bank (read)> is interpreted as integer and selects the sequence which should be output. there can be \
up to 256 sequences in the module.\n\
<bank (edit)> is interpreted as integer and selects the sequence you want to edit/draw. presumably you \
won't modulate this input in the general case.\n\
currently all banks/sequences have the same length as defined by <nr steps>\n\n\
note that the audio engine has to be running in order to make changes of the inputs to be affective.\n\n\
to draw a sequence simply leftclick and draw on the module's sequencer view. right-click will reset the bar \
to the default (the value in <offset>). when pressing SHIFT when drawing, all sequencer bars of that bank are \
set simultanously. to get a finer drawing resolution, currently you have to zoom in.\
";
	return const_cast<char*>(r);
}

CSmodule_Seq::~CSmodule_Seq()
{
	if (valueStr) free(valueStr);
}

CSmodule_Seq* CSmodule_Seq::newOne()
{
	return new CSmodule_Seq();
}

void CSmodule_Seq::copyFrom(CSmodule *mod)
{
	CSmodule::copyFrom(mod);
	CSmodule_Seq *m = dynamic_cast<CSmodule_Seq*>(mod);
	if (!m) return;
	for (int i=0;i<CSMOD_SEQ_MAX_BANK;i++)
		bankChanged[i] = m->bankChanged[i];
	for (int i=0;i<CSMOD_SEQ_MAX;i++)
		data[i] = m->data[i];
}

void CSmodule_Seq::storeAdd(FILE *buf)
{
	fprintf(buf, "banks { ");
	// write nr banks
	int n=0; for (int i=0;i<CSMOD_SEQ_MAX_BANK;i++) if (bankChanged[i]) n++;
	fprintf(buf, "%d ", n);

	for (int b=0;b<CSMOD_SEQ_MAX_BANK;b++) if (bankChanged[b])
	{
		fprintf(buf, "seq { %d %d ", b, CSMOD_SEQ_MAX);
		for (int i=0;i<CSMOD_SEQ_MAX;i++)
			fprintf(buf, "%g ",data[b*CSMOD_SEQ_MAX + i]);
		fprintf(buf, "} ");
	}
	fprintf(buf, "} ");
}

void CSmodule_Seq::restoreAdd(FILE *buf)
{
	if (readFileVersion==1)
	{
		fscanf(buf, " seq { ");

		int nr;
		int e = fscanf(buf, "%d", &nr);
		if (e!=1) CSERROR("CSmodule_Seq::restoreAdd (nr of data points)");

		for (int i=0;i<nr;i++)
		{
			csfloat f;
			e = fscanf(buf, "%f", &f);
			if (e!=1) CSERROR("CSmodule_Seq::restoreAdd (data point)");
			if (i<CSMOD_SEQ_MAX) data[i] = f;
		}
		bankChanged[0] = true;
		fscanf(buf, " } ");
	}
	else
	if (readFileVersion==2)
	{
		fscanf(buf, " banks { ");
		int n = 0;
		int e = fscanf(buf, "%d", &n);
		if (e!=1) CSERROR("CSmodule_Seq::restoreAdd (nr of banks)");

		for (int j=0;j<n;j++)
		{
			fscanf(buf, " seq { ");
			int b=0,nr=0;
			e = fscanf(buf, "%d %d", &b, &nr);
			if (e!=2) CSERROR("CSmodule_Seq::restoreAdd (bank nr / nr of data points)");

			for (int i=0;i<nr;i++)
			{
				csfloat f=0.0;
				e = fscanf(buf, "%f", &f);
				if (e!=1) CSERROR("CSmodule_Seq::restoreAdd (data point)");
				if (i<CSMOD_SEQ_MAX) data[b*CSMOD_SEQ_MAX + i] = f;
			}
			bankChanged[b] = true;
			fscanf(buf, " } ");
		}
		fscanf(buf, " } ");
	}
}


void CSmodule_Seq::init()
{
	fileVersion = 2; // banks

	gname = copyString("sequencer");
	bname = copyString("sequencer");
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
	_rst = &(createInput("r","reset")->value);
	_step = &(createInput("s","step")->value);
	_amp = &(createInput("a","amplitude",1.0)->value);
	_off = &(createInput("o","offset")->value);
	_bank = &(createInput("br","bank (read)")->value);
	_bankedit = &(createInput("be","bank (edit)")->value);

	_out = &(createOutput("out","output")->value);
	_outG = &(createOutput("oug","gate output")->value);

	// ---- properties ----

	createNameProperty();

	// intern

	valueStr = (char*) calloc(1024,sizeof(char));

	sentGate = false;
	drawing = -1;
	lrst = lstep = 0.0;
	pos = 0;
	memset(bankChanged, 0, CSMOD_SEQ_MAX_BANK * sizeof(bool));
	memset(data, 0, CSMOD_SEQ_MAX * CSMOD_SEQ_MAX_BANK * sizeof(csfloat));
}




void CSmodule_Seq::step()
{
	// selected bank
	int ba = max(0,min(CSMOD_SEQ_MAX_BANK-1, (int)*_bank ));
	// bank data offset
	int bao = ba*CSMOD_SEQ_MAX;

	// nr of steps
	int st = max(1,min(CSMOD_SEQ_MAX, (int)*_steps ));

	// forward sequencer
	if ((lstep<=0.0)&&(*_step>0.0))
		{ pos = (pos+1) % st; sentGate = false; }
	lstep = *_step;

	// reset sequencer
	if ((lrst<=0.0)&&(*_rst>0.0))
		{ pos = 0; sentGate = false; }
	lrst = *_rst;

	// be sure when steps has changed outside a gate...
	if (pos>=st) pos = pos % st;

	// feed output
	*_out = *_off + *_amp * data[bao+pos];
	// feed gate output
	if (!sentGate)
		{ *_outG = *_out; sentGate = true; }
	else *_outG = *_off;
}





bool CSmodule_Seq::mouseDown(int mx, int my, int mk)
{
	if ((mx<x+sx)||(my<y+sy)||(mx>x+sx+sw)||(my>y+sy+sh))
	{
		drawing = -1;
		return false;
	}

	drawing = 0;
	return mouseMove(mx,my,mk);
}

bool CSmodule_Seq::mouseMove(int mx, int my, int mk)
{
	if (drawing==-1) return false;

	mx -= x+sx;
	my -= y+sy;
	mx = max(0, min(sw, mx ));
	my = max(0, min(sh, my ));

	int ba = max(0,min(CSMOD_SEQ_MAX_BANK-1, (int)*_bankedit ));
	int bao = ba*CSMOD_SEQ_MAX;

	int st = max(1,min(CSMOD_SEQ_MAX, (int)*_steps ));
	int p = min(st-1, mx*st/sw);

	drawing = p;

	if (mk==FL_LEFT_MOUSE)
	{
		data[bao+p] = 1.0 - (csfloat)my / sh;
		if (Fl::event_state() & FL_SHIFT)
			for (int i=0;i<st;i++)
				data[bao+i] = data[bao+p];
		bankChanged[ba] = true;
	}
	else
	if (mk==FL_RIGHT_MOUSE)
	{
		data[bao+p] = 0.0;
		if (Fl::event_state() & FL_SHIFT)
			for (int i=0;i<st;i++)
				data[bao+i] = 0.0;
		bankChanged[ba] = true;
	}
	else { drawing = -1; return false; }

	return true;
}



void CSmodule_Seq::draw(int offx, int offy, float zoom)
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
	int ba = max(0,min(CSMOD_SEQ_MAX_BANK-1, (int)*_bankedit ));
	int bao = ba*CSMOD_SEQ_MAX;

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

	// seq position
	fl_color(FL_RED);
	fl_rectf(X + pos*W/st, Y+H+2, W1,4);

}





























char *CSmodule_Timeline::docString()
{
	const char *r = "\
";
	return const_cast<char*>(r);
}

CSmodule_Timeline::~CSmodule_Timeline()
{
	if (X) free(X);
	if (Y) free(Y);
}

CSmodule_Timeline* CSmodule_Timeline::newOne()
{
	return new CSmodule_Timeline();
}

void CSmodule_Timeline::copyFrom(CSmodule *mod)
{
	CSmodule::copyFrom(mod);
	CSmodule_Timeline *m = dynamic_cast<CSmodule_Timeline*>(mod);
	if (!m) return;

	// copy data
	for (int i=0;i<m->nrp;i++)
		addPoint(m->X[i], m->Y[i]);

	// copy view
	m->setWindow(m->selWindow);
	for (int i=0;i<CS_TL_MAXWINDOW;i++)
	{
		voffx_[i] = m->voffx_[i];
		voffy_[i] = m->voffy_[i];
		vzoomx_[i] = m->vzoomx_[i];
		vzoomy_[i] = m->vzoomy_[i];
	}
	setWindow(m->selWindow,false);
	viewMode = m->viewMode;
}

void CSmodule_Timeline::storeAdd(FILE *buf)
{
	fprintf(buf, "points { %d ", nrp);
	for (int i=0;i<nrp;i++)
		fprintf(buf, "%g %g ", X[i], Y[i]);
	fprintf(buf, "} ");
	if (fileVersion>1)
	{
		setWindow(selWindow); // store current window view
		fprintf(buf, "view { ");
		if (fileVersion>2)
			fprintf(buf, "%d %d ", selWindow, viewMode);
		fprintf(buf, "%d ", CS_TL_MAXWINDOW);
		for (int i=0;i<CS_TL_MAXWINDOW;i++)
			fprintf(buf, "%g %g %g %g ",
				voffx_[i], voffy_[i], vzoomx_[i], vzoomy_[i]);
		fprintf(buf, "} ");
	}
}

void CSmodule_Timeline::restoreAdd(FILE *buf)
{
	fscanf(buf, " points { ");
	int Nrp;
	int e = fscanf(buf, "%d", &Nrp);
	if (e!=1) CSERROR("CSmodule_Timeline::restoreAdd() (nr points)");

	for (int i=0;i<Nrp;i++)
	{
		csfloat x,y;
		e = fscanf(buf, "%g %g", &x, &y);
		if (e!=2) CSERROR("CSmodule_Timeline::restoreAdd() (data points)");
		addPoint(x,y);
	}
	fscanf(buf, " } ");

	if (readFileVersion>1)
	{
		fscanf(buf, " view { ");
		if (readFileVersion>2)
		{
			e = fscanf(buf, "%d", &selWindow);
			if (e!=1) CSERROR("CSmodule_Timeline::restoreAdd() (selWindow)");
			e = fscanf(buf, "%d", &viewMode);
			if (e!=1) CSERROR("CSmodule_Timeline::restoreAdd() (view mode)");
		}
		e = fscanf(buf, "%d", &Nrp);
		if (e!=1) CSERROR("CSmodule_Timeline::restoreAdd() (nr view windows)");

		for (int i=0;i<Nrp;i++)
		{
			float ox,oy,zx,zy;
			e = fscanf(buf, "%g %g %g %g", &ox,&oy,&zx,&zy);
			if (i<CS_TL_MAXWINDOW)
			{
				voffx_[i] = ox;
				voffy_[i] = oy;
				vzoomx_[i] = zx;
				vzoomy_[i] = zy;
			}
		}
		fscanf(buf, " } ");
		setWindow(selWindow,false);
	}
}

void CSmodule_Timeline::onResize()
{
	sw = width - sx-64;
	sh = height -sy-12;
}

void CSmodule_Timeline::init()
{
	gname = copyString("sequencer");
	bname = copyString("timeline");
	name = copyString(bname);
	uname = copyString(bname);
	fileVersion = 3;

	// --- look ----

	sw = 320;
	sh = 160;
	sx = 64;
	sy = headerHeight+12;
	minWidth = sx+sw+64;
	minHeight = sy+sh+12;
	width = minWidth;
	height = minHeight;

	// --- inputs outputs ----

	_pos = &(createInput("po","position",0.0)->value);
	_amp = &(createInput("a","amplitude",1.0)->value); *_amp = 1.0;
	_off = &(createInput("o","offset")->value);
	_smth= &(createInput("sm","steepness",5.0)->value);

	_out = &(createOutput("os","output")->value);
	outL = createOutput("ol","linear"); _outL = &outL->value;
	outS = createOutput("os","smooth"); _outS = &outS->value;
	outC = createOutput("oc","cubic"); _outC = &outC->value;

	_outFirst = &(createOutput("o1","first X")->value);
	_outLast = &(createOutput("o2","last X")->value);

	// ---- properties ----

	canResize = true;

	createNameProperty();
	createPropertyInt("lop","loop timeline (0 or 1)", 0, 0,1);

	// intern

	editMode = 0;
	focusPoint = -1;
	voffx = 0; vzoomx = sw;
	voffy = 0; vzoomy = sh/2;
	selWindow = 0;
	for (int i=0;i<CS_TL_MAXWINDOW;i++)
	{
		voffx_[i] = voffx;
		voffy_[i] = voffy;
		vzoomx_[i] = vzoomx;
		vzoomy_[i] = vzoomy;
	}

	viewMode = 1;
	doLoop = false;

	nrp = 0;
	nrpa = CSMOD_TIMELINE_MIN;
	X = (csfloat*) calloc(nrpa, sizeof(csfloat));
	Y = (csfloat*) calloc(nrpa, sizeof(csfloat));

}


void CSmodule_Timeline::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"lop")==0)
		doLoop = (prop->ivalue==1);
		if ((nrp>1)&&(doLoop))
			Y[nrp-1] = Y[0];
	else
	CSmodule::propertyCallback(prop);
}


void CSmodule_Timeline::step()
{

	// find correct readout point
	int k = getClosestPoint(*_pos);

	// no points?
	if (k<0)
	{
		*_out = *_off;
		*_outS = *_off;
		*_outL = *_off;
		*_outC = *_off;
		*_outFirst = 0;
		*_outLast = 0;
		return;
	}

	// output first and last point
	*_outFirst = X[0];
	*_outLast = X[nrp-1];

	// after last point
	if ((k>=nrp-1)&&(*_pos>X[nrp-1])) {
		*_out = *_off + *_amp * Y[nrp-1];
		*_outS = *_out;
		*_outL = *_out;
		*_outC = *_out;
		return;
	}

	// decrease index if lower than X[k]
	if ( (k>0) && (*_pos < X[k])) k--;

	// before first point
	if ((k==0)&&(*_pos<X[0])) {
		*_out = *_off + *_amp * Y[0];
		*_outS = *_out;
		*_outL = *_out;
		*_outC = *_out;
		return;
	}

	// stepped out
	*_out = *_off + *_amp * Y[k];

	// other outs
	if ((outL->nrCon)||(outS->nrCon)||(outC->nrCon))
	{
		*_outL = *_off;
		*_outS = *_off;
		*_outC = *_off;
		if (k>=nrp-1) return;

		// linear
		csfloat t = ( *_pos - X[k] ) / max(0.0001, X[k+1] - X[k]);
		*_outL = *_out * (1.0-t) + t * (*_off + *_amp * Y[k+1]);

		// cubic out
		if (outC->nrCon)
		{
			csfloat y0,y1,y2,y3,ys1,ys2,a1;
			int pt0,pt2,pt3;
			if (nrp<4) { *_outC = *_outL; return; }
			if (doLoop)
			{
				pt0 = k-1; if (pt0<0) pt0 = nrp-2;
				pt2 = k+1; if (pt2>=nrp-1) pt2 = 0;
				pt3 = k+2; if (pt3>=nrp-1) pt3 = 1;
			} else {
				pt0 = k-1; if (pt0<0) pt0 = 0;
				pt2 = k+1; if (pt2>=nrp) pt2 = nrp-1;
				pt3 = k+2; if (pt3>=nrp) pt3 = nrp-1;
			}
			y0 = Y[pt0];
			y1 = Y[k];
			y2 = Y[pt2];
			y3 = Y[pt3];
			// final shape
			ys1 = (y2-y0) * 0.5;
			ys2 = (y3-y1) * 0.5;
			a1 = y2-y1;
			*_outC = *_off + *_amp * (
				y1 + (a1 + ( a1 - ys1 + (ys2-a1)*t) * (t-1) ) * t
				);

		}

		// smooth out
		if (outS->nrCon)
		{
			t = *_smth*t*t*(1-t) + pow(t, *_smth);
			*_outS = *_out * (1.0-t) + t * (*_off + *_amp * Y[k+1]);
		}

	}

}



void CSmodule_Timeline::setWindow(int newWindow, bool doStore)
{
	// store current window
	if (doStore)
	{
		voffx_[selWindow] = voffx;
		voffy_[selWindow] = voffy;
		vzoomx_[selWindow] = vzoomx;
		vzoomy_[selWindow] = vzoomy;
	}
	// set other window
	selWindow = newWindow;
	voffx = voffx_[selWindow];
	voffy = voffy_[selWindow];
	vzoomx = vzoomx_[selWindow];
	vzoomy = vzoomy_[selWindow];
}


bool CSmodule_Timeline::mouseDown(int mx, int my, int mk)
{
	mx -= x+sx;
	my -= y+sy;
	// check for window select
	if ((mx>=0)&&(mx<=CS_TL_MAXWINDOW*10)
		&&(my>=-10)&&(my<=-2))
	{
		setWindow(max(0,min(CS_TL_MAXWINDOW-1, mx/10 )));
		editMode = 0;
		return true;
	}
	// check for viewmode select
	if ((mx>=sw-40)&&(mx<=sw)
		&&(my>=-10)&&(my<=-2))
	{
		viewMode = max(0,min(3, (mx-(sw-40))/10 ));
		editMode = 0;
		return true;
	}

	// leave if not within timeline view
	if ((mx<0)||(my<0)||(mx>sw)||(my>sh))
	{
		editMode = 0;
		return false;
	}

	// move or delete
	if (focusPoint>=0)
	{
		if (mk == FL_LEFT_MOUSE)
			editMode = 1;
		else
		if (mk == FL_RIGHT_MOUSE)
		{
			deletePoint(focusPoint);
			focusPoint = 0;
			editMode = 0;
		}
		return true;
	}

	if (mk == FL_LEFT_MOUSE)
	{

		// add new point?
		int y = getVY(getValue(getRX(mx)));
		if ((nrp==0)||((my>=y-3) && (my<=y+3)))
		{
			focusPoint = addPoint(getRX(mx), getRY(my));
			editMode = 1;
			if ((doLoop)&&(nrp>1))
			{
				if (focusPoint==0)
					Y[nrp-1] = Y[0];
				else
					Y[0] = Y[nrp-1];
			}

			return true;
		}

		// else move view
		editMode = 2;
		lmx = mx;
		lmy = my;
		voffxo = voffx;
		voffyo = voffy;
		return true;
	}

	if (mk == FL_RIGHT_MOUSE)
	{
		// zoom view
		editMode = 3;
		lmx = mx;
		lmy = my;
		lmxr = getRX(mx);
		lmyr = getRY(my);
		voffxo = voffx;
		voffyo = voffy;
		vzoomxo = vzoomx;
		vzoomyo = vzoomy;
		return true;
	}

	return false;
}

bool CSmodule_Timeline::mouseMove(int mx, int my, int mk)
{
	mx -= x+sx;
	my -= y+sy;

	// move view
	if (editMode==2)
	{
		voffx = voffxo - (lmx-mx);
		voffy = voffyo + (lmy-my);
		return true;
	}

	// zoom view
	if (editMode==3)
	{
		float vzx = vzoomxo - 3.0*(lmx-mx);
		float vzy = vzoomyo + 3.0*(lmy-my);
		if ((vzx>=2)&&(vzy>=2))
		{
			voffx = voffxo + 3.0*lmxr*(lmx-mx);
			voffy = voffyo - 3.0*lmyr*(lmy-my);
			vzoomx = vzx;
			vzoomy = vzy;
		}
		return true;
	}

	// quit if outside timeline view
	if ((mx<0)||(my<0)||(mx>sw)||(my>sh))
		return 0;

	int r = 3;

	// find point in focus
	if (editMode==0)
	{
		int ofp = focusPoint;
		focusPoint = -1;
		for (int i=0;i<nrp;i++)
		{
			int px = getVX(X[i]);
			int py = getVY(Y[i]);
			if ((mx>=px-r)&&(mx<=px+r)&&(my>=py-r)&&(my<=py+r))
			{
				focusPoint = i;
				return true;
			}
		}
		if (ofp!=focusPoint) return true;
	}

	// move point
	if ((editMode==1) && (focusPoint>=0) && (focusPoint<nrp))
	{
		Y[focusPoint] = getRY(my);
		if ((focusPoint==0) && (nrp>1))
			X[0] = min(getRX(mx), X[1]-0.001);
		else
		if ((focusPoint==nrp-1) && (nrp>1))
			X[focusPoint] = max(X[focusPoint-1]+0.001, getRX(mx));
		else
		if ((focusPoint>0) && (focusPoint<nrp-1))
			X[focusPoint] = max(X[focusPoint-1]+0.001, min(getRX(mx), X[focusPoint+1]-0.001 ));
		else
			X[focusPoint] = getRX(mx);

		if ((doLoop)&&(nrp>1))
		{
			if (focusPoint==0)
				Y[nrp-1] = Y[0];
			else
				Y[0] = Y[nrp-1];
		}

		return true;
	}

	return false;
}


bool CSmodule_Timeline::mouseUp(int mx, int my)
{
	editMode = 0;
	return true;
}


void CSmodule_Timeline::draw(int offx, int offy, float zoom)
{
	CSmodule::draw(offx,offy,zoom);

	// background
	int bx=offx+zoom*(sx+x);
	int by=offy+zoom*(sy+y);
	int bw=zoom*sw;
	int bh=zoom*sh;
	fl_color(FL_BLACK);
	fl_rectf(bx,by,bw,bh);

	// window select buttons
	for (int i=0;i<CS_TL_MAXWINDOW;i++)
	{
		fl_color((i==selWindow)? FL_GREEN:FL_BLACK);
		fl_rectf(bx + zoom*(i*10), by - zoom*10, zoom*8, zoom*8);
	}
	// viewmode select buttons
	for (int i=0;i<4;i++)
	{
		fl_color((i==viewMode)? FL_GREEN:FL_BLACK);
		fl_rectf(bx + bw+zoom*(-40+i*10), by - zoom*10, zoom*8, zoom*8);
	}

	// x lines
	int rx = getRX(0);
	int st = max(1, getRX(10)-getRX(0));
	int bb = -20, bbb = 0;
	bool doo = true;
	while (doo)
	{
		int lx = zoom*(getVX(rx));
		if (lx>=bw) doo = false;
		else
		if ((lx>=0))
		{
			fl_color((rx==0)? FL_DARK_GREEN:FL_DARK3);
			fl_yxline(bx+lx, by, by+bh);
			// label
			bbb = bx+lx+2;
			if (bbb>bb+10)
			{
				sprintf(valueStr, "%d", rx);
				fl_color(FL_DARK1);
				fl_draw(valueStr, bbb,by+10);
				bb = bbb;
			}
		}
		rx +=st;
	}
	// y lines
	int ry = getRY(0);
	st = max(1, getRY(0)-getRY(10));
	doo = true;
	bb = -20;
	while (doo)
	{
		int ly = zoom*(getVY(ry));
		if (ly>=bh) doo = false;
		else
		if ((ly>=0))
		{
			fl_color((ry==0)? FL_DARK_GREEN:FL_DARK3);
			fl_xyline(bx, by+ly, bx+bw);
			// label
			bbb = by+ly+10;
			if (bbb>bb+10)
			{
				sprintf(valueStr, "%d", ry);
				fl_color(FL_DARK1);
				fl_draw(valueStr, bx+2,bbb);
				bb = bbb;
			}
		}
		ry -=st;
	}

	int r = 0;
	// data points
	for (int i=0;i<nrp;i++)
	{
		int px = zoom*getVX(X[i]);
		if ((px<0)||(px>bw)) continue;
		int py = zoom*getVY(Y[i]);
		if ((py<0)||(py>bh)) continue;

		if (i==focusPoint)
		{
			r = max(3, 3*zoom);
			fl_color( FL_GREEN );
		} else {
			r = max(2, 2*zoom);
			fl_color( FL_WHITE );
		}
		fl_rectf(bx+px-r,by+py-r,r<<1,r<<1);
	}

	// data lines

	fl_color(FL_WHITE);
	int py1=0;
	for (int i=0;i<sw;i+=5)
	{
		int pk = getVY( getValue( getRX(i) ) );
		int py = by + zoom * pk;
		if ((i>0)&&(pk>=0)&&(zoom*pk<=bh)) fl_line( bx + zoom * (i-5), py1, bx + zoom*i, py);
		py1 = py;
	}


	// changed value
	if (focusPoint>=0)
	{
		fl_color(FL_WHITE);
		fl_font(FL_HELVETICA, 10);
		sprintf(valueStr, "%g : %g", X[focusPoint], *_off + *_amp * Y[focusPoint]);
		fl_draw(valueStr,
			bx+4+zoom*getVX(X[focusPoint]),
			by+zoom*getVY(Y[focusPoint])
			);
	}

	// seq position
	fl_color(FL_RED);
	int px = getVX(*_pos);
	if ((px>=0)&&(px<=sw))
	{
		px *= zoom;
		fl_yxline(bx+px,by,by+bh);
	}

}


int CSmodule_Timeline::getVX(csfloat x)
{
	return x*vzoomx + voffx;
}

int CSmodule_Timeline::getVY(csfloat y)
{
	return (-y)*vzoomy - voffy + (sh>>1);
}

csfloat CSmodule_Timeline::getRX(int x)
{
	return ((csfloat)x - voffx) / vzoomx;
}

csfloat CSmodule_Timeline::getRY(int y)
{
	return ((csfloat)-y - voffy + (sh>>1)) / vzoomy;
}


int CSmodule_Timeline::getClosestPoint(csfloat x)
{
	/* this function returns -1 for no points
	   OR always the index AFTER the best match */
	if (nrp<1) return -1;

	int i = 0;
	for (int a=0, b=nrp; (a+1)<b; )
	{
		i = (a + b) >> 1;
		if (X[i] < x) a = i; else b = i;
	}

	//printf("closest to %g = %g (%d)\n", x, X[i], i);
	return i;

}

int CSmodule_Timeline::addPoint(csfloat x, csfloat y)
{
	// first point
	if (nrp<1)
	{
		X[0] = x;
		Y[0] = y;
		nrp = 1;
		return 0;
	}

	// alloc new space
	if (nrp>=nrpa-1)
	{
		nrpa += CSMOD_TIMELINE_MIN;
		X = (csfloat*) realloc(X, nrpa * sizeof(csfloat));
		Y = (csfloat*) realloc(Y, nrpa * sizeof(csfloat));
	}

	// find place to add
	int k = getClosestPoint(x);
	if ( (k<nrp) && (x > X[k]) ) k++;

	// shift rest of points
	for (int i=nrp;i>k;i--)
	{
		X[i] = X[i-1];
		Y[i] = Y[i-1];
	}
	X[k] = x;
	Y[k] = y;

	nrp++;

	return k;

	/*
	printf("\n");
	for (int i=0;i<nrp;i++)
		printf("%g : %g \n", X[i], Y[i]);
		*/
}

void CSmodule_Timeline::deletePoint(int k)
{
	if ((k<0)||(k>=nrp)) return;

	// shift points
	for (int i=k;i<nrp-1;i++)
	{
		X[i] = X[i+1];
		Y[i] = Y[i+1];
	}

	nrp--;
}


csfloat CSmodule_Timeline::getValue(csfloat x)
{
	if (nrp<1) return 0.0;
	int k = getClosestPoint(x);
	// no points
	if (k<0) return 0.0;
	// after last point
	if (k>=nrp) return Y[nrp-1];

	// move before index if smaller
	bool xk = (x <= X[k]);
	if ( (k>0) && xk) k--;
	// before first point
	if ((k==0)&&(x<X[0])) return Y[0];
	// after last point
	if (k>=nrp-1) return Y[k];

	csfloat t,y0,y1,y2,y3,ys1,ys2,a1;
	int pt0,pt2,pt3;
	switch (viewMode)
	{
		default:
			return Y[k];
			break;

		case 1:
			// linear interpolate
			t = ( x - X[k] ) / max(0.0001, X[k+1] - X[k]);
			return Y[k] * (1.0-t) + t * Y[k+1];
			break;

		case 2:
			// smooth interpolate
			t = ( x - X[k] ) / max(0.0001, X[k+1] - X[k]);
			t = *_smth*t*t*(1-t) + pow(t, *_smth);
			return Y[k] * (1.0-t) + t * Y[k+1];
			break;

		case 3:
			// cubic out

			// when not enough points
			if (nrp<4)
			{
				// linear interpolate
				t = ( x - X[k] ) / max(0.0001, X[k+1] - X[k]);
				return Y[k] * (1.0-t) + t * Y[k+1];
			}
			if (doLoop)
			{
				pt0 = k-1; if (pt0<0) pt0 = nrp-2;
				pt2 = k+1; if (pt2>=nrp-1) pt2 = 0;
				pt3 = k+2; if (pt3>=nrp-1) pt3 = 1;
			} else {
				pt0 = k-1; if (pt0<0) pt0 = 0;
				pt2 = k+1; if (pt2>=nrp) pt2 = nrp-1;
				pt3 = k+2; if (pt3>=nrp) pt3 = nrp-1;
			}
			y0 = Y[pt0];
			y1 = Y[k];
			y2 = Y[pt2];
			y3 = Y[pt3];
			// final shape
			ys1 = (y2-y0) * 0.5;
			ys2 = (y3-y1) * 0.5;
			a1 = y2-y1;
			t = ( x - X[k] ) / max(0.0001, X[k+1] - X[k]);
			return y1 + (a1 + ( a1 - ys1 + (ys2-a1)*t) * (t-1) ) * t;

			break;
	}
}
