#include <algorithm>
#include <math.h>
#include "csmodule_control.h"

char *CSmodule_Switch::docString()
{
	const char *r = "\
this module represents a switch that can be pressed with the mouse. \
the behaviour can be set in properties.\n\n\
in toggle mode it toggles between the <off> and <on> value.\n\
in gate mode it sends the <on> value for one sample and returns to the <off> value afterward.\n\n\
sending a gate signal to the <gate> input has the same effect as pressing the button on the module.\
";
	return const_cast<char*>(r);
}


CSmodule_Switch* CSmodule_Switch::newOne()
{
	return new CSmodule_Switch();
}

void CSmodule_Switch::copyFrom(CSmodule *mod)
{
	CSmodule::copyFrom(mod);
	CSmodule_Switch *m = dynamic_cast<CSmodule_Switch*>(mod);
	if (!m) return;
	mode = m->mode;
	on = m->on;
}

void CSmodule_Switch::storeAdd(FILE *buf)
{
	// write value
	fprintf(buf, "val { %d } ", on);
}

void CSmodule_Switch::restoreAdd(FILE *buf)
{
	fscanf(buf, " val { ");
	int e = fscanf(buf, "%d", &on);
	if (e!=1) CSERROR("CSmodule_Switch::restoreAdd (value)");
	fscanf(buf, " } ");
}


void CSmodule_Switch::init()
{
	gname = copyString("control");
	bname = copyString("switch");
	name = copyString("switch (toggle)");
	uname = copyString(bname);

	if (!parent) return;

	// --- look ---

	sx = 60;
	sy = headerHeight+5;
	sw = 32;
	sh = 32;
	minWidth = sx+sw+60;
	minHeight = sy+sh+5;

	// --- inputs outputs ----

	_on = &(createInput("on","on",1.0)->value);
	_off = &(createInput("off","off")->value);
	_gate = &(createInput("g","gate")->value);
	_out = &(createOutput("out","out")->value);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("mode", "0=toggle, 1=gate", 0, 0, 1);

	mode = 0;
	on = 0;
	lgate = 0;
	wasScreenUpdate = 0;

}

void CSmodule_Switch::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"mode")==0)
		mode = prop->ivalue;
	else
	CSmodule::propertyCallback(prop);
}


void CSmodule_Switch::step()
{
	// check gate input
	if ((lgate<=0)&&(*_gate>0.0))
	{
		if (mode==0) on = 1-on;	else on = 1;
		if (on) wasScreenUpdate = 1;
	}
	lgate = *_gate;

	if (mode==0)
	{
		*_out = (on)? *_on : *_off;
	}
	else
	{
		*_out = (on)? *_on : *_off;
		if (on) wasScreenUpdate = 1;
		on = 0;
	}
}


void CSmodule_Switch::draw(int offx, int offy, float zoom)
{
	CSmodule::draw(offx,offy,zoom);

	int
		bx = offx+zoom*(x+sx),
		by = offy+zoom*(y+sy),
		bw = zoom*sw,
		bh = zoom*sh;

	fl_color(FL_WHITE);
	fl_font(FL_HELVETICA_ITALIC,zoom*10);
	fl_draw((mode==0)? "toggle" : "gate",
		offx+zoom*(x+width-30), offy+zoom*(y+12));

	fl_color((on || wasScreenUpdate)? FL_GREEN : FL_DARK_GREEN);
	fl_rectf(bx,by,bw,bh);

	wasScreenUpdate = 0;
}

bool CSmodule_Switch::mouseDown(int mx, int my, int /*mk*/)
{
	mx -= x+sx;
	my -= y+sy;

	// leave if not within button
	if ((mx<0)||(my<0)||(mx>sw)||(my>sh))
		return false;

	if (mode==0)
		on = 1-on;
	else
	{
		on = 1;
		wasScreenUpdate = 1;
	}

	return true;
}












CSmodule_SwitchGate* CSmodule_SwitchGate::newOne()
{
	return new CSmodule_SwitchGate();
}

void CSmodule_SwitchGate::init()
{
	CSmodule_Switch::init();

	free(name); name = copyString("switch (gate)");

	mode = 1;
	int i = getPropertyIndex("mode");
	if (i>=0) property[i]->ivalue = mode;
}





















char *CSmodule_SwitchMatrix::docString()
{
	const char *r = "\
this module connects a column of inputs to a column of outputs. \
the nr of inputs/outputs can be set in properties.\n\n\
the value in <in x> is added to the specific output if the switch between input and output is on.\n\n\
switches can be controlled with the mouse. left key will toggle the switch between on an off, right \
key will always turn it off. the current bank can be cleared at once by pressing the top-right 'R' button.\n\n\
the module has 256 possible banks for switch values.\n\
<bank (send)> (integer) selects the bank you want to use for connecting inputs to outputs.\n\
<bank (edit)> (integer) selects the bank you want to edit.\n\
NOTE that the audio process must be running to make changes to these inputs visible.\n\n\
properties:\n\
<bank fade?> 0 means the send-bank is switched from one to another, 1 means the send-bank is faded from \
one to the next. f.e. <bank (send)> = 1.3 means ~70% of bank #1 and ~30% of bank #2. the fading is slightly \
non-linear.\n\
<on/off (0) or float(1)> selects the switch mode. on/off means switches can only be on or off, float \
means switch positions can be anywhere between (and including) -1.0 and 1.0. in float mode the mouse \
must be clicked and dragged on a switch to change the value. normally the value is changed in 1/100th steps, \
if ALT is pressed it changes in 1/10000th steps.\n\n\
<nr of inputs> and <nr of outputs> sets the number of input/output connections (can be up to 32 each).\n\n\
<group count x/y> lets you arrange the switches in groups on the x and/or the y axis. this can be useful to \
keep an oversight over large switch matrixes.\n\n\
<single input> = 1 means: only one input can be routed to an output. if you select a second input for the \
same output, the other switch will turn off.\n\
<single output> is the same only for outputs so an input can be routed to only one of the outputs.\n\
when groups are activated with <group count x/y>, then the single constraint only works within the selected group.\n\
!!WARNING!! when selecting one or both of these options, ALL switches in ALL banks will be immidiately adjusted \
to fit these constraints, whereby the first on-state (from left or top) is the one that stays on.\
";
	return const_cast<char*>(r);
}


CSmodule_SwitchMatrix* CSmodule_SwitchMatrix::newOne()
{
	return new CSmodule_SwitchMatrix();
}

void CSmodule_SwitchMatrix::copyFrom(CSmodule *mod)
{
	CSmodule::copyFrom(mod);
	CSmodule_SwitchMatrix *m = dynamic_cast<CSmodule_SwitchMatrix*>(mod);
	if (!m) return;
	for (int k=0;k<CSMOD_SWITCH_MAX_BANK;k++)
	{
		bankUsed[k] = m->bankUsed[k];
		for (int j=0;j<CSMOD_SWITCH_MAX;j++)
		for (int i=0;i<CSMOD_SWITCH_MAX;i++)
			state[k][j][i] = m->state[k][j][i];
	}
}

void CSmodule_SwitchMatrix::storeAdd(FILE *buf)
{

	// write value
	int ub = 0;
	for (int i=0;i<CSMOD_SWITCH_MAX_BANK;i++)
		if (bankUsed[i]) ub++;

	fprintf(buf, "val { %d %d ", CSMOD_SWITCH_MAX, ub);

	for (int k=0;k<CSMOD_SWITCH_MAX_BANK;k++)
	{
		if (bankUsed[k])
		{
			fprintf(buf, "%d ",k);
			for (int j=0;j<CSMOD_SWITCH_MAX;j++)
			for (int i=0;i<CSMOD_SWITCH_MAX;i++)
				fprintf(buf, "%g ", state[k][j][i]);
		}
	}
	fprintf(buf, "} ");

}

void CSmodule_SwitchMatrix::restoreAdd(FILE *buf)
{

	fscanf(buf, " val { ");
	int n,nb,bn;
	int e = fscanf(buf, "%d", &n);
	if (e!=1) CSERROR("CSmodule_SwitchMatrix::restoreAdd (nr states)");
	e = fscanf(buf, "%d", &nb);
	if (e!=1) CSERROR("CSmodule_SwitchMatrix::restoreAdd (nr used banks)");

	resetStates();
	csfloat v;
	for (int k=0;k<nb;k++)
	{
		e = fscanf(buf, "%d", &bn);
		if (e!=1) CSERROR("CSmodule_SwitchMatrix::restoreAdd (bank number)");

		for (int j=0;j<n;j++) for (int i=0;i<n;i++)
		{
			e = fscanf(buf, "%g", &v);
			if (e!=1) CSERROR("CSmodule_SwitchMatrix::restoreAdd (states)");
			if ((j<CSMOD_SWITCH_MAX)&&(i<CSMOD_SWITCH_MAX))
				state[bn][j][i] = v;
		}

		bankUsed[bn] = true;
	}
	fscanf(buf, " } ");

}


void CSmodule_SwitchMatrix::init()
{
	gname = copyString("control");
	bname = copyString("switch matrix");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- look ---

	sx = 60;
	sy = headerHeight+5+inputHeight*2;
	sz = inputHeight;

	// --- inputs outputs ----

	_bankRead = &createInput("rb","bank (send)")->value;
	_bankWrite = &createInput("wb","bank (edit)")->value;

	char n[128];
	for (int i=0;i<CSMOD_SWITCH_MAX;i++)
	{
		sprintf(n,"in%d",i);
		__in[i] = createInput(n,n);
		_in[i] = &(__in[i]->value);
		__in[i]->autoArrange = false;

		sprintf(n,"out%d",i);
		__out[i] = createOutput(n,n);
		_out[i] = &(__out[i]->value);
		__out[i]->autoArrange = false;
		__out[i]->showLabel = false;
		__out[i]->x = sx + i*sz + sz/2;
	}

	offx = 4;
	offy = 4;
	setNrIn(4);
	setNrOut(4);
	doFadeBank = 0;
	singleIn = 0;
	singleOut = 0;
	doValDrag = false;
	editX = editY = 0;

	// ---- properties ----

	createNameProperty();
	createPropertyInt("fadeb", "fade bank? 0=off, 1=linear", 0, 0,1);
	createPropertyInt("float", "on/off (0) or float (1)", 0, 0,1);
	createPropertyInt("nrin", "nr of inputs", nrIn, 1,CSMOD_SWITCH_MAX);
	createPropertyInt("nrout", "nr of outputs", nrOut, 1,CSMOD_SWITCH_MAX);
	createPropertyInt("offx", "group count x", offx, 0,CSMOD_SWITCH_MAX);
	createPropertyInt("offy", "group count y", offy, 0,CSMOD_SWITCH_MAX);
	createPropertyInt("singin", "single input (0/1)?", singleIn, 0,1);
	createPropertyInt("singout", "single output (0/1)?", singleOut, 0,1);

	resetStates();
}

void CSmodule_SwitchMatrix::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"fadeb")==0)
		doFadeBank = prop->ivalue;
	else
	if (strcmp(prop->name,"float")==0)
		useFloat = prop->ivalue;
	else
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	if (strcmp(prop->name,"nrout")==0)
		setNrOut(prop->ivalue);
	else
	if (strcmp(prop->name,"offx")==0)
		{ offx = prop->ivalue; setNrOut(nrOut); }
	else
	if (strcmp(prop->name,"offy")==0)
		{ offy = prop->ivalue; setNrIn(nrIn); }
	else
	if (strcmp(prop->name,"singin")==0)
		singleIn = prop->ivalue;
	else
	if (strcmp(prop->name,"singout")==0)
		singleOut = prop->ivalue;
	else
	CSmodule::propertyCallback(prop);

	checkStates();
}


void CSmodule_SwitchMatrix::setNrIn(int newNrIn)
{
	nrIn = newNrIn;

	for (int i=0;i<CSMOD_SWITCH_MAX;i++)
	{
		setConnectorActive(__in[i], (i<nrIn));
	}

	sh = sz*nrIn + ((offy)? (nrIn/offy)*4:0);
	minHeight = sy+sh+5;
	height = minHeight;

	arrangeConnectors();
	for (int i=0;i<CSMOD_SWITCH_MAX;i++)
	{
		__in[i]->x = 0;
		__in[i]->y = sy+5 + sz*i + ((offy)? (i/offy)*4:0);
		__out[i]->y = height;
		__out[i]->x = sx + sz/2 + sz*i + ((offx)? (i/offx)*4:0);
	}
}

void CSmodule_SwitchMatrix::setNrOut(int newNrOut)
{
	nrOut = newNrOut;

	for (int i=0;i<CSMOD_SWITCH_MAX;i++)
	{
		setConnectorActive(__out[i], (i<nrOut));
	}

	sw = sz*nrOut + ((offx)? (nrOut/offx)*4:0);
	minWidth = sx+sw+5;
	width = minWidth;

	arrangeConnectors();
	for (int i=0;i<CSMOD_SWITCH_MAX;i++)
	{
		__in[i]->x = 0;
		__in[i]->y = sy+5 + sz*i + ((offy)? (i/offy)*4:0);
		__out[i]->y = height;
		__out[i]->x = sx + sz/2 + sz*i + ((offx)? (i/offx)*4:0);
	}
}

void CSmodule_SwitchMatrix::setState(int x,int y, csfloat stat)
{
	if (stat>0)
	{
		int a,b;
		if (singleIn)
		{
			if (offy)
			{
				a = (y/offy)*offy;
				b = a+offy;
				if (b>=nrIn) b = nrIn;
				for (int i=a;i<b;i++) state[curWriteBank][i][x] = 0;
			} else
				for (int i=0;i<nrIn;i++) state[curWriteBank][i][x] = 0;
		}
		if (singleOut)
		{
			if (offx)
			{
				a = (x/offx)*offx;
				b = a+offx;
				if (b>=nrOut) b = nrOut;
				for (int i=a;i<b;i++) state[curWriteBank][y][i] = 0;
			} else
				for (int i=0;i<nrOut;i++) state[curWriteBank][y][i] = 0;
		}
	}

	state[curWriteBank][y][x] = stat;
	bankUsed[curWriteBank] = true;
}

void CSmodule_SwitchMatrix::checkStates()
{
	bool wasOn;
	if (singleIn)
	{
		if (offy)
		{
			for (int k=0;k<CSMOD_SWITCH_MAX_BANK;k++)
			for (int z=0;z<CSMOD_SWITCH_MAX;z++)
			for (int j=0;j<CSMOD_SWITCH_MAX/offy;j++)
			{
				wasOn = false;
				for (int i=0;i<offy;i++)
				{
					if (wasOn) state[k][j*offx+i][z] = 0; else
						if (state[k][j*offx+i][z]) wasOn = true;
				}
			}
		}
		else
		{
			wasOn = false;
			for (int k=0;k<CSMOD_SWITCH_MAX_BANK;k++)
			for (int z=0;z<CSMOD_SWITCH_MAX;z++)
			for (int i=0;i<CSMOD_SWITCH_MAX;i++)
			{
				if (wasOn) state[k][i][z] = 0; else
					if (state[k][i][z]) wasOn = true;
			}
		}
	}

	if (singleOut)
	{
		if (offx)
		{
			for (int k=0;k<CSMOD_SWITCH_MAX_BANK;k++)
			for (int z=0;z<CSMOD_SWITCH_MAX;z++)
			for (int j=0;j<CSMOD_SWITCH_MAX/offx;j++)
			{
				wasOn = false;
				for (int i=0;i<offx;i++)
				{
					if (wasOn) state[k][z][j*offx+i] = 0; else
						if (state[k][z][j*offx+i]) wasOn = true;
				}
			}
		}
		else
		{
			wasOn = false;
			for (int k=0;k<CSMOD_SWITCH_MAX_BANK;k++)
			for (int z=0;z<CSMOD_SWITCH_MAX;z++)
			for (int i=0;i<CSMOD_SWITCH_MAX;i++)
			{
				if (wasOn) state[k][z][i] = 0; else
					if (state[k][z][i]) wasOn = true;
			}
		}
	}
}

void CSmodule_SwitchMatrix::resetStates(int bankNr)
{
	if (bankNr==-1)
	{
		for (int k=0;k<CSMOD_SWITCH_MAX_BANK;k++)
		{
			bankUsed[k] = false;
			for (int j=0;j<CSMOD_SWITCH_MAX;j++)
			for (int i=0;i<CSMOD_SWITCH_MAX;i++)
				state[k][j][i] = 0.0;
		}
	}
	else
	{
		bankUsed[bankNr] = false;
		for (int j=0;j<CSMOD_SWITCH_MAX;j++)
		for (int i=0;i<CSMOD_SWITCH_MAX;i++)
			state[bankNr][j][i] = 0.0;
	}

}
void CSmodule_SwitchMatrix::step()
{
	// update read/write bank index
	if (curReadBank != *_bankRead)
        curReadBank = std::max(0,std::min(CSMOD_SWITCH_MAX_BANK-1, (int)*_bankRead ));
	if (curWriteBank != *_bankWrite)
		{
        curWriteBank = std::max(0,std::min(CSMOD_SWITCH_MAX_BANK-1, (int)*_bankWrite ));
		doValDrag = false;
		}

	// clear outputs
	for (int i=0;i<nrOut;i++) *_out[i] = 0.0;

	// sum to outputs
	for (int j=0;j<nrIn;j++)
	{
		csfloat *s = &state[curReadBank][j][0];
		for (int i=0;i<nrOut;i++)
			*_out[i] += *s++ * *_in[j];
	}

	// fade banks
	if (doFadeBank==1)
	{
		// get bank fade
		csfloat t = max((csfloat)0,min((csfloat)CSMOD_SWITCH_MAX_BANK-1, *_bankRead )) - curReadBank;
		t = 2*t*t*(1-t)+t*t;
		if (t>0.0)
		{
			// sum next bank
			curReadBank = (curReadBank+1) % CSMOD_SWITCH_MAX_BANK;
			// clear temp outputs
			for (int i=0;i<nrOut;i++) o[i] = 0.0;
			for (int j=0;j<nrIn;j++)
			{
				csfloat
					*s = &state[curReadBank][j][0];
				for (int i=0;i<nrOut;i++)
					o[i] += *s++ * *_in[j];

			}
			// mix with temp outs
			for (int i=0;i<nrOut;i++)
				*_out[i] = *_out[i] * (1.0-t) + t * o[i];
		}
	}

}


void CSmodule_SwitchMatrix::draw(int offx, int offy, float zoom)
{
	CSmodule::draw(offx,offy,zoom);

	int
		bx = offx+zoom*(x+sx),
		by = offy+zoom*(y+sy),
		x1,y1,y2,
		bsz = zoom*(sz-1);

	if (!useFloat)
	{
		for (int j=0;j<nrIn;j++)
		for (int i=0;i<nrOut;i++)
		{
			fl_color((state[curWriteBank][j][i])? FL_GREEN : FL_DARK_GREEN);
			x1 = bx+zoom*(i*sz + ((this->offx)? (i/this->offx)*4:0));
			y1 = by+zoom*(j*sz + ((this->offy)? (j/this->offy)*4:0));
			fl_rectf(x1,y1,bsz,bsz);
		}
	}
	else
	{
		for (int j=0;j<nrIn;j++)
		for (int i=0;i<nrOut;i++)
		{
			x1 = bx+zoom*(i*sz + ((this->offx)? (i/this->offx)*4:0));
			y1 = by+zoom*(j*sz + ((this->offy)? (j/this->offy)*4:0));
			if (state[curWriteBank][j][i]>=0.0)
			{
				y2 = bsz*(1.0-state[curWriteBank][j][i]);
				fl_color(FL_DARK_GREEN);
				fl_rectf(x1,y1,bsz,y2);
				fl_color(FL_GREEN);
				fl_rectf(x1,y1+y2,bsz,bsz-y2);
			}
			else
			{
				y2 = bsz*(-state[curWriteBank][j][i]);
				fl_color(FL_GREEN);
				fl_rectf(x1,y1,bsz,y2);
				fl_color(FL_DARK_GREEN);
				fl_rectf(x1,y1+y2,bsz,bsz-y2);
			}
		}
		// show dragged value
		if (doValDrag)
		{
			x1 = bx+zoom*(sz+editX*sz + ((this->offx)? (editX/this->offx)*4:0));
			y1 = by+zoom*(10+editY*sz + ((this->offy)? (editY/this->offy)*4:0));
			fl_color(FL_WHITE);
            fl_font(FL_HELVETICA_BOLD, std::max(10,(int)(10*zoom)));
			char v[128];
			sprintf(v, "%g", state[curWriteBank][editY][editX]);
			fl_draw(v, x1,y1);
		}
	}

	// reset switch
	fl_color(FL_BLACK);
	x1 = offx+zoom*(x+sx+sw-12);
	y1 = offy+zoom*(y+headerHeight+5);
	bsz = zoom*12;
	fl_rectf(x1,y1,bsz,bsz);
	fl_color(FL_WHITE);
	fl_font(FL_HELVETICA_BOLD_ITALIC, 10*zoom);
	fl_draw("R", x1+3,y1+10*zoom);
}

bool CSmodule_SwitchMatrix::mouseDown(int mx, int my, int mk)
{
	doValDrag = false;
	lastY = my;

	mx -= x;
	my -= y;

	// check reset button
	if ((mx>=sx+sw-12)&&(mx<=sx+sw)&&
		(my>=headerHeight+5)&&(my<headerHeight+17))
	{
		resetStates(curWriteBank);
		return true;
	}

	mx -= sx;
	my -= sy;

	// leave if not within switches
	if ((mx<0)||(my<0)||(mx>sw)||(my>sh))
		return false;

	mx = (mx - ((offx)? (mx/sz/offx)*4:0))/sz;
	my = (my - ((offy)? (my/sz/offy)*4:0))/sz;
	if ((mx>=nrOut)||(my>=nrIn)) return false;

	if (mk==FL_RIGHT_MOUSE)
		setState(mx,my, 0.0);
	else
	if (!useFloat)
		setState(mx,my, 1.0-state[curWriteBank][my][mx]);
	else
	{
		doValDrag = true;
		editX = mx; editY = my;
	}

	return true;
}

bool CSmodule_SwitchMatrix::mouseMove(int /*mx*/, int my, int /*mk*/)
{
	if ((doValDrag)&&(useFloat))
	{
		csfloat di = 100.0;
		if (Fl::event_state()&FL_ALT) di = 10000.0;
		csfloat ns = max((csfloat)-1,min((csfloat)1,
			state[curWriteBank][editY][editX] + (csfloat)(lastY-my)/di
			));
		// remove float precision error
		csfloat ns1 = (float)((int)(fabs(ns)*di+0.5)/di);
		if (ns<0) ns1 = -ns1;
		setState(editX,editY,ns1);
		lastY = my;
		return true;
	}
	return false;
}

bool CSmodule_SwitchMatrix::mouseUp(int /*mx*/, int /*my*/)
{
	doValDrag = false;
	return true;
}
