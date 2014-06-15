#include "csmodule_logic.h"

char *CSmodule_Logic::docString()
{
	const char *r = "\
the module performs logic operations on the inputs, which are interpretated as bits.\n\n\
<in> is an input which in interpretated as false/off for <= 0.0 and true/on for > 0.0.\n\
<AND>, <OR> or <XOR> is the output result of the logic operation for all inputs (0 or 1).\n\
<!AND>, <!OR> a.s.o. is the inverted <output> for convenience.\
";
	return const_cast<char*>(r);
}

CSmodule_Logic* CSmodule_Logic::newOne()
{
	return new CSmodule_Logic();
}


void CSmodule_Logic::init()
{
	gname = copyString("logic");
	bname = copyString("logic");
	name = copyString("logic AND");
	uname = copyString(bname);

	// --- inputs outputs ----

	_out = &(createOutput("out","output")->value);
	_nout = &(createOutput("out!","!output")->value);

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("in",i);
		__in[i] = createInput(n,"in");
		free(n);
		_in[i] = &(__in[i]->value);
	}

	oldNrIn = 0;
	setNrIn(2);

	mop = 0;

	// ---- properties ----

	createNameProperty();
	createPropertyInt("mop", "operation (0-2,AND,OR,XOR)", mop, 0,2);
	createPropertyInt("nrin", "nr of inputs", nrIn, 2,CSMOD_MAX_CHAN);

	setMop(mop);
}

void CSmodule_Logic::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	if (strcmp(prop->name,"mop")==0)
		setMop(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_Logic::setNrIn(int newNrIn)
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

void CSmodule_Logic::setMop(int newMop)
{
	mop = newMop;
	int k = getPropertyIndex("mop");
	if (k>=0) property[k]->ivalue = mop;

	if (con[0]->uname) free(con[0]->uname);
	if (con[1]->uname) free(con[1]->uname);
	switch (mop)
	{
		default:
			con[0]->uname = copyString("AND");
			con[1]->uname = copyString("!AND"); break;
		case 1:
			con[0]->uname = copyString("OR");
			con[1]->uname = copyString("!OR"); break;
		case 2:
			con[0]->uname = copyString("XOR");
			con[1]->uname = copyString("!XOR"); break;
	}
}


void CSmodule_Logic::step()
{
	// start with first input
	*_out = (*_in[0] > 0.0);

	// operate the rest
	switch (mop)
	{
		default: // AND
			for (int i=1;i<nrIn;i++)
				*_out = (*_out>0.0) & (*_in[i]>0.0);
			break;
		case 1: // OR
			for (int i=1;i<nrIn;i++)
				*_out = (*_out>0.0) | (*_in[i]>0.0);
			break;
		case 2: // XOR
			for (int i=1;i<nrIn;i++)
				*_out = (*_out>0.0) xor (*_in[i]>0.0);
			break;
	}
	*_nout = (*_out <= 0.0);
}












CSmodule_LogicOR* CSmodule_LogicOR::newOne()
{
	return new CSmodule_LogicOR();
}


void CSmodule_LogicOR::init()
{
	CSmodule_Logic::init();
	if (name) free(name); name = copyString("logic OR");
	setMop(1);
}



CSmodule_LogicXOR* CSmodule_LogicXOR::newOne()
{
	return new CSmodule_LogicXOR();
}


void CSmodule_LogicXOR::init()
{
	CSmodule_Logic::init();
	if (name) free(name); name = copyString("logic XOR");
	setMop(2);
}


















char *CSmodule_GateHold::docString()
{
	const char *r = "\
the module holds a gate signal in <gate> for <hold (sec)> seconds + <hold (sam)> samples.\
";
	return const_cast<char*>(r);
}

CSmodule_GateHold* CSmodule_GateHold::newOne()
{
	return new CSmodule_GateHold();
}


void CSmodule_GateHold::init()
{
	gname = copyString("logic");
	bname = copyString("gate hold");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_gate = &(createInput("g","gate")->value);
	_hold = &(createInput("h","hold (sec)",0.0)->value);
	_holds = &(createInput("hs","hold (sam)",1.0)->value);
	_out = &(createOutput("out","output")->value);

	lgate = 0.0;
	time = 0;

	// ---- properties ----

	createNameProperty();
}


void CSmodule_GateHold::step()
{
	if ((lgate<=0.0)&&(*_gate>0.0))
	{
		*_out = *_gate;
		time = 0;
	}
	lgate = *_gate;

	if (*_out)
	{
		time++;
		if (time >= sampleRate * *_hold + *_holds) *_out = 0.0;
	}
}










char *CSmodule_GateDelay::docString()
{
	const char *r = "\
the module delays a gate signal for a certain time.\n\n\
when a positive edge receives at <gate>, the module starts to count backwards the time that is \
given in <delay (sec)> at the time of the gate. when the time is over, it sends a one sample long \
gate at <output>.\n\
the module works monophonic which means any new gate input will discard the old one if it's not \
already send through.\
";
	return const_cast<char*>(r);
}

CSmodule_GateDelay* CSmodule_GateDelay::newOne()
{
	return new CSmodule_GateDelay();
}


void CSmodule_GateDelay::init()
{
	gname = copyString("logic");
	bname = copyString("gate delay");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_gate = &(createInput("g","gate")->value);
	_sec = &(createInput("d","delay (sec)",1.0)->value);
	_out = &(createOutput("out","output")->value);

	lgate = 0.0;
	time = 0;

	// ---- properties ----

	createNameProperty();
}


void CSmodule_GateDelay::step()
{
	if ((lgate<=0.0)&&(*_gate>0.0))
	{
		*_out = 0.0;
		time = sampleRate * *_sec;
	}
	lgate = *_gate;

	if (time)
	{
		time--;
		/* well, thats not very exact, actually 2 samples early,
		   but efficient programming */
		if (time==1) *_out = 1.0; else
		if (time==0) *_out = 0.0;
	}
}











char *CSmodule_GateRepeat::docString()
{
	const char *r = "\
the module repeats a gate signal in <gate> <repeat> times.\
";
	return const_cast<char*>(r);
}

CSmodule_GateRepeat* CSmodule_GateRepeat::newOne()
{
	return new CSmodule_GateRepeat();
}


void CSmodule_GateRepeat::init()
{
	gname = copyString("logic");
	bname = copyString("gate repeat");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_gate = &(createInput("g","gate")->value);
	_rep = &(createInput("r","repeat",2.0)->value);
	_out = &(createOutput("out","output")->value);

	lgate = 0.0;
	time = 0;

	// ---- properties ----

	createNameProperty();
}


void CSmodule_GateRepeat::step()
{
	if ((lgate<=0.0)&&(*_gate>0.0))
	{
		gate = *_gate;
		*_out = 0.0;
		time = *_rep;
	}
	lgate = *_gate;

	if (gate)
	{
		if (time == 0) gate = 0.0;
		if (*_out) { *_out = 0.0; time--; } else *_out = gate;
	}
}









char *CSmodule_OnChange::docString()
{
	const char *r = "\
the module simply outputs 1.0 if the input has changed, 0.0 otherwise.\
";
	return const_cast<char*>(r);
}

CSmodule_OnChange* CSmodule_OnChange::newOne()
{
	return new CSmodule_OnChange();
}


void CSmodule_OnChange::init()
{
	gname = copyString("logic");
	bname = copyString("on change");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_in = &(createInput("i","input")->value);
	_out = &(createOutput("o","output")->value);

	lin = 0.0;

	// ---- properties ----

	createNameProperty();
}


void CSmodule_OnChange::step()
{
	if (*_in!=lin)
		*_out = 1.0;
	else
		*_out = 0.0;

	lin = *_in;
}
















char *CSmodule_Merge::docString()
{
	const char *r = "\
\
";
	return const_cast<char*>(r);
}

CSmodule_Merge* CSmodule_Merge::newOne()
{
	return new CSmodule_Merge();
}


void CSmodule_Merge::init()
{
	gname = copyString("logic");
	bname = copyString("merger");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_rst = &(createInput("rst","reset")->value);

	char n[128], n1[128];
	__out[0] = createOutput("out0","GATE");
	_out[0] = &__out[0]->value;
	for (int i=1;i<CSMOD_MERGE_MAX_PARAM;i++)
	{
		sprintf(n,"out%d",i);
		sprintf(n1,"param #%d",i);
		__out[i] = createOutput(n,n1);
		_out[i] = &(__out[i]->value);
	}

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	for (int j=0;j<CSMOD_MERGE_MAX_PARAM;j++)
	{
		int k=(i<<CSMOD_MERGE_MAX_PARAM_SHIFT)+j;
		sprintf(n,"p%d_%d",i,j);
		if (j==0)
			sprintf(n1,"- #%d GATE -",i+1);
		else
			sprintf(n1,"param %d",j);

		__param[k] = createInput(n,n1);
		_param[k] = &(__param[k]->value);
	}

	setNrIn(2,2);
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
		lgate[i] = 0.0;
	bufpos = 0;
	toSend = 0;
	waitFrame = 0;

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrin", "nr of channels", nrIn, 2,CSMOD_MAX_CHAN);
	createPropertyInt("nrpar", "nr of parameters", nrParam, 1,CSMOD_MERGE_MAX_PARAM);
}

void CSmodule_Merge::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue,nrParam);
	else
	if (strcmp(prop->name,"nrpar")==0)
		setNrIn(nrIn, prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_Merge::setNrIn(int newNrIn, int newNrParam)
{
	nrIn = newNrIn;
	nrParam = newNrParam;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	for (int j=0;j<CSMOD_MERGE_MAX_PARAM;j++)
	{
		int k = (i<<CSMOD_MERGE_MAX_PARAM_SHIFT)+j;
		setConnectorActive(__param[k], (i<nrIn)&&(j<nrParam));
	}
	for (int j=0;j<CSMOD_MERGE_MAX_PARAM;j++)
		setConnectorActive(__out[j], (j<nrParam) );

	arrangeConnectors();
}


void CSmodule_Merge::step()
{
	// go through all channel inputs
	for (int i=0;i<nrIn;i++)
	{
		int k=(i<<CSMOD_MERGE_MAX_PARAM_SHIFT);
		// check for gate
		if ((lgate[i]<=0.0) && (*_param[k]>0.0))
		{
			// store in buffer
			for (int j=0;j<nrParam;j++)
				buf[bufpos+j] = *_param[k+j];

			bufpos = (bufpos+CSMOD_MERGE_MAX_PARAM) & (CSMOD_MERGE_MAX_BUF_SIZE-1);
			toSend++;
		}
		lgate[i] = *_param[k];
	}

	// reset
	if ((lrst<=0.0)&&(*_rst>0.0))
	{
		for (int i=0;i<nrIn;i++)
			lgate[i] = 0.0;
		for (int j=0;j<nrParam;j++)
			*_out[j] = 0.0;
		bufpos = 0;
		toSend = 0;
		waitFrame = 0;

		// reset wins!
		return;
	}

	// skip every next frame when sending
	if (waitFrame) {
		// reset gate-out to make ready for next gate
		*_out[0] = 0.0;
		waitFrame = 0;
		return;
		}

	// send off the buffer
	if (toSend)
	{
		bufpos = (bufpos-CSMOD_MERGE_MAX_PARAM) & (CSMOD_MERGE_MAX_BUF_SIZE-1);
		for (int j=0;j<nrParam;j++)
			*_out[j] = buf[bufpos+j];

		toSend--;
		waitFrame = 1;
	}

}
















char *CSmodule_StackFILO::docString()
{
	const char *r = "\
the module implements a First-In-Last-Out Stack for a number of parameters. the number can be set \
in properties.\n\n\
a gate in <Push> will copy all values from the <Val x> inputs to the current stack position which is \
output in <stackpos>. the stack position will then increase by one.\n\
a gate in <Pop> will decrease the stack position by one and copy the values from the stack to the <val x> \
outputs.\n\
a gate in <Clear> will reset the stack position and clear the outputs.\n\n\
if the stack is full the stack pointer will not increase. if the stack is empty the stack pointer will not \
decrease.\
";
	return const_cast<char*>(r);
}

CSmodule_StackFILO* CSmodule_StackFILO::newOne()
{
	return new CSmodule_StackFILO();
}


void CSmodule_StackFILO::init()
{
	gname = copyString("logic");
	bname = copyString("stack (FILO)");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_push = &createInput("psh", "Push")->value;
	_pop = &createInput("pop", "Pop")->value;
	_clear = &createInput("clr", "Clear")->value;

	char n[128],n1[128];
	for (int i=0;i<CSMOD_STACK_MAX_PARAM;i++)
	{
		snprintf(n,127, "p%d", i);
		snprintf(n1,127, "val %d",i+1);
		__in[i] = createInput(n,n1);
		_in[i] = &(__in[i]->value);
	}

	_stackpos = &createOutput("pos", "stack pos")->value;

	for (int i=0;i<CSMOD_STACK_MAX_PARAM;i++)
	{
		snprintf(n,127, "po%d", i);
		snprintf(n1,127, "val %d",i+1);
		__out[i] = createOutput(n,n1);
		_out[i] = &(__out[i]->value);
	}

	setNrIn(1);

	lpush = lpop = lclear = 0.0;

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrin", "nr of simultanious values", nrIn, 1,CSMOD_STACK_MAX_PARAM);
}

void CSmodule_StackFILO::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_StackFILO::setNrIn(int newNrIn)
{
	nrIn = newNrIn;
	for (int i=0;i<CSMOD_STACK_MAX_PARAM;i++)
	{
		setConnectorActive(__in[i], (i<nrIn));
		setConnectorActive(__out[i], (i<nrIn));
	}

	arrangeConnectors();
}



void CSmodule_StackFILO::step()
{
	// push something onto the stack
	if ((lpush<=0.0)&&(*_push>0.0))
	{
		// go to actual position in stack[]
		csfloat *p = stack;
		p += (stackpos << CSMOD_STACK_MAX_PARAM_SHIFT);

		// copy values
		for (int i=0;i<nrIn;i++)
			*p++ = *_in[i];

		// inc stack counter
		if (stackpos<CSMOD_STACK_MAX-1) stackpos++;
	}
	lpush = *_push;

	// pop something from the stack
	if ((lpop<=0.0)&&(*_pop>0.0))
	{
		// go to previous stack position
		if (stackpos>0) stackpos--;
		csfloat *p = stack;
		p += (stackpos << CSMOD_STACK_MAX_PARAM_SHIFT);

		// copy values;
		for (int i=0;i<nrIn;i++)
			*_out[i] = *p++;
	}
	lpop = *_pop;

	// clear the stack
	if ((lclear<=0.0)&&(*_clear>0.0))
	{
		stackpos = 0;
		csfloat *p = stack;
		for (int i=0;i<nrIn;i++)
		{
			*_out[i] = 0.0;
			*p++ = 0.0;
		}
	}
	lclear = *_clear;

	// update stackpos output
	*_stackpos = stackpos;
}













char *CSmodule_CounterFloat::docString()
{
	const char *r = "\
the module simply adds the value in <direction> constantly to the <output>. it's used to have free running \
numbers (within bounds) without effort.\n\n\
the value in <direction> will be added to the <output> in each sample step.\n\
if the <output> becomes >= <max> or <= <min> it will stay at <max> (or <min>) and the respective <gate min> or \
<gate max> is set to 1.0.\n\n\
a gate in <Reset> will reset the <output> to the value in <load>.\
\
";
	return const_cast<char*>(r);
}

CSmodule_CounterFloat* CSmodule_CounterFloat::newOne()
{
	return new CSmodule_CounterFloat();
}


void CSmodule_CounterFloat::init()
{
	gname = copyString("logic");
	bname = copyString("counter (float)");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_direction = &createInput("dir", "direction")->value;
	_min = &createInput("min", "min", -1.0)->value;
	_max = &createInput("max", "max", 1.0)->value;
	_rst = &createInput("rst", "Reset")->value;
	_load = &createInput("lod", "load")->value;

	_out = &createOutput("out", "output")->value;
	_gmin = &createOutput("gmi", "Gate min")->value;
	_gmax = &createOutput("gma", "Gate max")->value;

	lrst = 0.0;

	// ---- properties ----

	createNameProperty();
}


void CSmodule_CounterFloat::step()
{
	*_gmax = 0.0;
	*_gmin = 0.0;

	// count
	*_out += *_direction;

	// reset counter
	if ((lrst<=0.0)&&(*_rst>0.0))
	{
		*_out = *_load;
	}
	lrst = *_rst;

	if (*_out>=*_max) { *_out = *_max; *_gmax = 1.0; }
	else
	if (*_out<=*_min) { *_out = *_min; *_gmin = 1.0; }


}
