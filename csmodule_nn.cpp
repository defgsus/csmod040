#include "math.h"
#include "csmodule_nn.h"

char *CSmodule_NN::docString()
{
	const char *r = "\
this module represents one layer of hidden cells of a simple error-backpropagation neuronal network.\n\n\
it has 1-4 input cells, 1-100 hidden cells and 1-4 output cells, and connections from \
input->hidden->ouput. the learning method for the connection weights is gradient descent through \
error backpropagtion of the difference between output state and desired state.\n\
\n\
<reset> will 'brainwash' the net and reset the weights to small random values.\n\
<rnd.seed> is the integer start seed of the random generator to reset the net.\n\
<rnd.amp> is the maximum amplitude of the randomized weights\n\
\n\
<learnrate> is the speed of the weight change or learning process. it is divided by the \
current sample rate.\n\
\n\
<input x> is an arbitrary input to the network. values should be between -1.0 and 1.0.\n\
<desired x> is the value that is expected from the xth output for the given input.\n\
\n\
<in error x> is the xth input cell's accumulated error. the error from the output cells is \
propagated backwards through the current network weights and output in <in error x> output. \
see below for further explanation.\n\
\n\
<output x> is the actual output of the network, which is trained to become <desired x> for \
the given input.\n\
\n\
notes\n\
nr of inputs/hidden cells/outputs can be set in properties, as long with the activation function \
(0=liner, 1=tangens hyperbolicus, 2=sin).\n\
the network can be used as an experimental follower or as a function approximator. in the latter \
case it is important that you choose meaningful scales and ranges for input and desired states.\n\
it is almost always helpful to supply a bias (like 0.5 or 1.0 or so) to an input of the network.\n\
the higher the learning rate is, the faster the weights adjust to the current input and the given task, \
but it is likely that the network 'forgets' about handling other input/desired pairs.\n\
to see what the network actually has learned, set learnrate to 0.\n\
in general this module can not be asked serious questions, it's more a building block for more \
sophisticated networks.\n\
\n\
how it works and how to chain multiple backprop modules:\n\
since there is only one layer of hidden cells, you may want to chain more modules to get a deeper \
network. first of all, refere to good literature for the pros and cons of multiple layer backprop networks. \
then you have to understand how the thing works of'course. consider a 1-input, 3-hidden, 1-output network. \
we supply a value on the input and a value as the desired output. the input values is passed through to the \
3 hidden cells (multiplied by the individual input->hidden weights) and further propagated to the output cell \
(multiplied with the individual hidden->output weights). the difference between the output and the desired state \
is calculated. this is the output error. this error now is propagted backwards through the network, the same way \
as the input passes forward. each weight in the network is then changed by a small amount (learnrate) to minimize \
that error, considering the current input. you can think of it like: every cell in the network (except the outputs) \
are considered to be potentially responsible for the output error. the greater the weight between two cells grows, \
the greater the possible error responisbility for the cell that sends.\n\
now to chain multiple modules, you can use the <in error x> value (which is the output error backwards propagated \
to the inputs) to pass the error through to a previous network in the module chain. to be precise: we want the \
output error of the previous network to be the accumulated input error of this network. and the output error is \
indirectly set through the <desired x> inputs. it all breaks down to connecting the <in error x> of network #2 \
+ the <output x> of network #1 with <desired x> if network #1. so the desired states of network #1 are it's \
outputs + the <in error x> of network #2.\n\
don't worry, if you do not understand this, you won't have fun with the module and can leave that to someone else ;)\
";
	return const_cast<char*>(r);
}


CSmodule_NN* CSmodule_NN::newOne()
{
	return new CSmodule_NN();
}


void CSmodule_NN::init()
{
	gname = copyString("neuro");
	bname = copyString("neuronal net");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- layout ---

	headerHeight = 40;

	// --- inputs outputs ----

	_rst = &(createInput("rst","reset")->value);
	_seed = &(createInput("see","rnd.seed")->value);
	_ramp = &(createInput("ra","rnd.amp",0.01)->value); *_ramp = 0.01;
	_lr = &(createInput("lr","learnrate",1.0)->value);

	_err = &(createOutput("e","error")->value);

	for (int i=0;i<CSMOD_NN_MAX_IN;i++)
	{
		char *n = int2char("in",i);
		char *n1 = int2char("input ",i+1);
		__in[i] = createInput(n,n1);
		_in[i] = &__in[i]->value;
		free(n); free(n1);

		n = int2char("ine",i);
		n1 = int2char("in error ",i+1);
		__ine[i] = createOutput(n,n1);
		_ine[i] = &__ine[i]->value;
		free(n); free(n1);
	}

	for (int i=0;i<CSMOD_NN_MAX_OUT;i++)
	{
		char *n  = int2char("des",i);
		char *n1 = int2char("desired ",i+1);
		__des[i] = createInput(n,n1);
		_des[i] = &__des[i]->value;
		free(n); free(n1);
		n  = int2char("out",i);
		n1 = int2char("output ",i+1);
		__out[i] = createOutput(n,n1);
		_out[i] = &__out[i]->value;
		free(n); free(n1);
	}

    // -- nn --

    requestInit = false;

	activation = 0;
	weightSeed = rand()%0xffffff;
	*_seed = weightSeed;
	/*
	cell = 0;
	error = 0;
	errorOut = 0;
	weightIn = 0;
	weightOut = 0;
	*/
	nrHidden = 10;

	oldNrIn = 0;
	oldNrOut = 0;
	nrIn = 1;
	nrOut = 1;

	setNrOut(1);
	setNrIn(1);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrin","nr of inputs", nrOut, 1, CSMOD_NN_MAX_IN);
	createPropertyInt("nrhid","nr of hidden cells", nrHidden, 1, CSMOD_NN_MAX);
	createPropertyInt("nrout","nr of outputs", nrOut, 1, CSMOD_NN_MAX_OUT);
	createPropertyInt("act","activation function", activation, 0, 2);
	//createPropertyInt("rseed","random weight seed", weightSeed);

	lrst = 0.0;

}

void CSmodule_NN::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"act")==0)
	{
		activation = prop->ivalue;
	}
	else
	if (strcmp(prop->name,"rseed")==0)
	{
		weightSeed = prop->ivalue;
		brainWash();
	}
	else
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	if (strcmp(prop->name,"nrhid")==0)
	{
		initNet(nrIn, prop->ivalue, nrOut);
	}
	if (strcmp(prop->name,"nrout")==0)
		setNrOut(prop->ivalue);

	else
	CSmodule::propertyCallback(prop);
}


void CSmodule_NN::setNrOut(int newNrOut)
{
	for (int i=0;i<CSMOD_NN_MAX_OUT;i++)
	{
		bool b = (i<newNrOut);
		setConnectorActive(__out[i], b);
		setConnectorActive(__des[i], b);
	}

	arrangeConnectors();

	initNet(nrIn,nrHidden,newNrOut);
}


void CSmodule_NN::setNrIn(int newNrIn)
{
	for (int i=0;i<CSMOD_NN_MAX_IN;i++)
	{
		bool b = (i<newNrIn);
		setConnectorActive(__in[i], b);
		setConnectorActive(__ine[i], b);
	}

	arrangeConnectors();

	initNet(newNrIn,nrHidden,nrOut);
}




void CSmodule_NN::step()
{
	// reset net
	if ( (lrst<=0) && ( *_rst>0 ) ) brainWash();
	lrst = *_rst;

	// propagate in->hid
	for (int i=0;i<nrHidden;i++)
	{
		cell[i] = 0.0;
		for (int j=0;j<nrIn;j++)
			cell[i] += weightIn[i][j] * *_in[j];
	}

	switch (activation)
	{
		case 1:
			for (int i=0;i<nrHidden;i++) cell[i] = tanhf(cell[i])/0.73;
			break;
		case 2:
			for (int i=0;i<nrHidden;i++) cell[i] = sin(cell[i]*PI);
			break;
	}


	// propagate hid->out
	for (int i=0;i<nrOut;i++)
	{
		*_out[i] = 0.0;
		for (int j=0;j<nrHidden;j++)
			*_out[i] += weightOut[i][j] * cell[j];
	}

	// activation function
	switch (activation)
	{
		case 1:
			for (int i=0;i<nrOut;i++) *_out[i] = tanhf(*_out[i])/0.73;
			break;
		case 2:
			for (int i=0;i<nrOut;i++) *_out[i] = sin(*_out[i]*PI);
			break;
	}


	// --- get error / train ---

	// learnrate
	csfloat lr = *_lr * isampleRate;


	// first clear hidden error
	for (int i=0;i<nrHidden;i++) error[i] = 0.0;

	// get output error and propagate -> hidden error
	// sie haben meine vollste rückendeckung dabei
	*_err = 0.0;
	for (int i=0;i<nrOut;i++)
	{
		// get error
		errorOut[i] = *_des[i] - *_out[i];
		*_err += fabs(errorOut[i]);

		// prop back
		for (int j=0;j<nrHidden;j++)
			error[j] += weightOut[i][j] * errorOut[i];
	}

	// prop error to input
	for (int i=0;i<nrIn;i++) *_ine[i] = 0.0;

	for (int i=0;i<nrHidden;i++)
		for (int j=0;j<nrIn;j++)
			*_ine[j] += weightIn[i][j] * error[i];


	// start training
	if (lr<=0.0) return;


	// train weightOut
	for (int i=0;i<nrOut;i++)
	{
		for (int j=0;j<nrHidden;j++)
			weightOut[i][j] += lr * errorOut[i] * cell[j];
	}

	// train weightIn
	for (int i=0;i<nrHidden;i++)
	{
		for (int j=0;j<nrIn;j++)
			weightIn[i][j] += lr * error[i] * *_in[j];
	}

}



void CSmodule_NN::initNet(int nrIn, int nrHid, int nrOut)
{
	this->nrIn = nrIn;
	this->nrHidden = nrHid;
	this->nrOut = nrOut;

	brainWash();
}

void CSmodule_NN::brainWash()
{
	srand((int)*_seed);

	//printf("%d %d %d\n", nrIn,nrHidden,nrOut);

	for (int i=0;i<CSMOD_NN_MAX;i++)
	{
		cell[i] = 0.0;
		error[i] = 0.0;

		for (int j=0;j<nrIn;j++)
			weightIn[i][j] = ((csfloat)rand()/RAND_MAX-0.5) * *_ramp;
	}

	for (int i=0;i<nrOut;i++)
	{
		*_out[i] = 0.0;
		errorOut[i] = 0.0;

		for (int j=0;j<nrHidden;j++)
			weightOut[i][j] = ((csfloat)rand()/RAND_MAX-0.5) * *_ramp;
	}

}


void CSmodule_NN::draw(int offx, int offy, float zoom)
{
	CSmodule::draw(offx,offy,zoom);

	if (nrHidden<1) return;

	// draw hidden cell states
	int a = max(2, zoom*(width - 10));
	int x1 = offx+zoom*(x+5);
	int y1 = offy+zoom*(y+(headerHeight>>1));
	int y2 = offy+zoom*(y+headerHeight*2/3);
	for (int i=0;i<a;i++)
	{
		int ci = i*nrHidden/a;
		uchar c = min(255,max(0, abs(cell[ci]*255) ));
		if (cell[ci]>0)
			fl_color(c << 16);
		else
			fl_color(c << 24);

		fl_yxline(x1+i, y1, y2);
	}

	// draw weights
	y1 = y2+2;
	y2 = offy+zoom*(y+headerHeight-4);
	for (int i=0;i<a;i++)
	{
		int ci = i*CSMOD_NN_MAX*CSMOD_NN_MAX_IN / a;
		int cj = ci % CSMOD_NN_MAX_IN;
			ci = ci / CSMOD_NN_MAX_IN;
		uchar c = min(255,max(0, abs(weightIn[ci][cj]*255) ));
		if (weightIn[ci][cj]>0)
			fl_color(c << 16);
		else
			fl_color(c << 24);

		fl_yxline(x1+i, y1, y2);
	}
}




























char *CSmodule_NNLSM::docString()
{
	const char *r = "\
this module represents a random feedback-connected layer of hidden cells and a simply output layer.\n\n\
it has 1-4 input cells, 1-100 hidden cells and 1-4 output cells, and connections from \
input->hidden<->hidden->ouput. the learning method for the output weights is gradient descent by the \
difference between output state and desired state.\n\
\n\
<reset> will 'brainwash' the net and reset the weights to small random values.\n\
<rnd.seed> is the integer start seed of the random generator to reset the net.\n\
<rnd.amp> is the maximum amplitude of the randomized weights\n\
\n\
<learnrate> is the speed of the weight change or learning process. it is divided by the \
current sample rate.\n\
\n\
<input x> is an arbitrary input to the network. values should be between -1.0 and 1.0.\n\
<desired x> is the value that is expected from the xth output for the given input.\n\
\n\
notes\n\
nr of inputs/hidden cells/outputs can be set in properties, as long with the activation function \
(0=liner, 1=tangens hyperbolicus, 2=sin).\n\
the network can be used as a experimental follower or as a function approximator. in the latter \
case it is important that you choose meaningful scales and ranges for input and desired states.\n\
it is almost always helpful to supply a bias (like 0.5 or 1.0 or so) to an input of the network.\n\
the higher the learning rate is, the faster the weights adjust to the current input and the given task, \
but it is likely that the network 'forgets' about handling other input/desired pairs.\n\
to see what the network actually has learned, set learnrate to 0.\n\
in general this module can not be asked serious questions, it's more a building block for more \
sophisticated networks.\
\n\
how it works:\n\
the liquid state machine is a great invention of our century. it's based upon the theory that a bunch of \
randomly (and a bit sparsely) interconnected feedback cells, is capable of doing solving a lot of the problems you may want to \
'ask' the network for. since feedback connected cells are hard (or even impossible) to train by error \
backpropagation, only the output weights are trained by gradient descent. each output is connected to ALL \
cells in the feedback bunch (the 'liquid'). by passing a desired value for a certain input, the outputs \
learn to find the solution within the 'boiling liquid'.\n\
\n\
the liquid's connectivity can be controlled from the property window. set the number of cells for the liquid and \
the number of random hidden->hidden connections. a sparse connectivity is better in most cases. note that however \
the liquid state machine theory sounds like a good general problem solver, normally a lot of fine tuning is to be \
done to solve certain tasks. those fine tunings are not possible at the moment :-(\
";
	return const_cast<char*>(r);
}


CSmodule_NNLSM* CSmodule_NNLSM::newOne()
{
	return new CSmodule_NNLSM();
}


void CSmodule_NNLSM::init()
{
	gname = copyString("neuro");
	bname = copyString("liquid state machine");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- layout ---

	headerHeight = 40;

	// --- inputs outputs ----

	_rst = &(createInput("rst","reset")->value);
	_seed = &(createInput("see","rnd.seed",rand()%0xffff)->value);
	_rampl = &(createInput("ral","rnd.amp",0.9)->value);
	_ramp = &(createInput("ra","rnd.amp.out",0.01)->value);
	__gate = createInput("g","gate");
	_gate = &__gate->value;
	_lr = &(createInput("lr","learnrate",1.0)->value);

	_err = &(createOutput("e","error")->value);

	for (int i=0;i<CSMOD_NN_MAX_IN;i++)
	{
		char *n = int2char("in",i);
		char *n1 = int2char("input ",i+1);
		__in[i] = createInput(n,n1);
		_in[i] = &__in[i]->value;
		free(n); free(n1);
	}

	for (int i=0;i<CSMOD_NN_MAX_OUT;i++)
	{
		char *n  = int2char("des",i);
		char *n1 = int2char("desired ",i+1);
		__des[i] = createInput(n,n1);
		_des[i] = &__des[i]->value;
		free(n); free(n1);
		n  = int2char("out",i);
		n1 = int2char("output ",i+1);
		__out[i] = createOutput(n,n1);
		_out[i] = &__out[i]->value;
		free(n); free(n1);
	}

    // -- nn --

    // init pointers
    for (int i=0;i<CSMOD_LSM_MAX;i++)
    {
		weightInIndex[i] = _in[0];
		weightInHidIndex[i] = &celltemp[0];
		weightHidFromIndex[i] = &cell[0];
		weightHidToIndex[i] = &celltemp[0];
    }

	activation = 0;
	connectRad = 4;
	filterPower = 2;

	oldNrIn = 0;
	oldNrOut = 0;
	initNet(1, 5, 2,8, 5, 1);

	setNrOut(1);
	setNrIn(1);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("act","activation function", activation, 0, 2);
	createPropertyInt("nrin","nr of inputs", nrOut, 1, CSMOD_NN_MAX_IN);
	createPropertyInt("nrout","nr of outputs", nrOut, 1, CSMOD_NN_MAX_OUT);
	createPropertyInt("nrincon","nr of input-hidden con.", nrInCon, 1, CSMOD_LSM_MAX);
	createPropertyInt("nrhidw","nr of hidden cells (width)", nrHidW, 1, CSMOD_LSM_MAX_WIDTH);
	createPropertyInt("nrhidh","nr of hidden cells (height)", nrHidH, 1, CSMOD_LSM_MAX_HEIGHT);
	createPropertyInt("nrhidcon","nr of hidden-hidden con.", nrHidCon, 1, CSMOD_LSM_MAX);
	createPropertyFloat("conrad","hidden-hidden max distance", connectRad, 1.0, CSMOD_LSM_MAX_HEIGHT+CSMOD_LSM_MAX_WIDTH);
	createPropertyFloat("fpow","lowpass filter potence", filterPower, 0.0, 100.0);

	//createPropertyInt("rseed","random weight seed", weightSeed);

	lrst = 0.0;
	lgate = 0.0;

}

void CSmodule_NNLSM::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"act")==0)
	{
		activation = prop->ivalue;
	}
	else
	if (strcmp(prop->name,"conrad")==0)
	{
		connectRad = prop->fvalue;
		brainWash();
	}
	else
	if (strcmp(prop->name,"fpow")==0)
	{
		filterPower = prop->fvalue;
		brainWash();
	}
	else
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	if (strcmp(prop->name,"nrincon")==0)
	{
		initNet(nrIn, prop->ivalue, nrHidW, nrHidH, nrHidCon, nrOut);
	}
	else
	if (strcmp(prop->name,"nrhidw")==0)
	{
		initNet(nrIn, nrInCon, prop->ivalue, nrHidH, nrHidCon, nrOut);
	}
	else
	if (strcmp(prop->name,"nrhidh")==0)
	{
		initNet(nrIn, nrInCon, nrHidW, prop->ivalue, nrHidCon, nrOut);
	}
	else
	if (strcmp(prop->name,"nrhidcon")==0)
	{
		initNet(nrIn, nrInCon, nrHidW, nrHidH, prop->ivalue, nrOut);
	}
	else
	if (strcmp(prop->name,"nrout")==0)
		setNrOut(prop->ivalue);

	else
	CSmodule::propertyCallback(prop);
}


void CSmodule_NNLSM::setNrOut(int newNrOut)
{
	for (int i=0;i<CSMOD_NN_MAX_OUT;i++)
	{
		bool b = (i<newNrOut);
		setConnectorActive(__out[i], b);
		setConnectorActive(__des[i], b);
	}

	arrangeConnectors();

	initNet(nrIn,nrInCon, nrHidW,nrHidH,nrHidCon,newNrOut);
}


void CSmodule_NNLSM::setNrIn(int newNrIn)
{
	for (int i=0;i<CSMOD_NN_MAX_IN;i++)
	{
		bool b = (i<newNrIn);
		setConnectorActive(__in[i], b);
	}

	arrangeConnectors();

	initNet(newNrIn,nrInCon, nrHidW,nrHidH,nrHidCon, nrOut);
}




void CSmodule_NNLSM::step()
{
	// reset net
	if ( (lrst<=0) && ( *_rst>0 ) ) brainWash();
	lrst = *_rst;

	if (__gate->nrCon)
	{
		if (!((lgate<=0.0)&&(*_gate>0.0))) { lgate = *_gate; return; }
		lgate = *_gate;
	}

	// clear temp layer
	for (int i=0;i<nrHid;i++)
		celltemp[i] = 0.0;

	// propagate in->hid
	for (int i=0;i<nrInCon;i++)
	{
		*weightInHidIndex[i] +=
			weightIn[i] * *weightInIndex[i];
	}

	// propagate hid->hid (the liquid)
	for (int i=0;i<nrHidCon;i++)
		*weightHidToIndex[i] +=
			weightHid[i] * *weightHidFromIndex[i];

	switch (activation)
	{
		case 0:
			for (int i=0;i<nrHid;i++) cell[i] += celllp[i]*(celltemp[i] - cell[i]);
			break;
		case 1:
			for (int i=0;i<nrHid;i++) cell[i] += celllp[i]*(tanhf(celltemp[i])*TANH_1_I - cell[i]);
			break;
		case 2:
			for (int i=0;i<nrHid;i++) cell[i] += celllp[i]*(sin(celltemp[i]*PI) - cell[i]);
			break;
	}


	// propagate hid->out
	for (int i=0;i<nrOut;i++)
	{
		*_out[i] = 0.0;
		for (int j=0;j<nrHid;j++)
			*_out[i] += weightOut[i][j] * cell[j];
	}

	// activation function
	/*
	switch (activation)
	{
		case 1:
			for (int i=0;i<nrOut;i++) *_out[i] = tanhf(*_out[i])/0.73;
			break;
		case 2:
			for (int i=0;i<nrOut;i++) *_out[i] = sin(*_out[i]*PI);
			break;
	}
	*/


	// --- get error / train ---

	// learnrate
	csfloat lr = *_lr * isampleRate;

	if (lr<=0.0) return;

	// get output error and adjust hid->out weights
	*_err = 0.0;
	for (int i=0;i<nrOut;i++)
	{
		// get error
		errorOut[i] = *_des[i] - *_out[i];
		*_err += fabs(errorOut[i]);

		for (int j=0;j<nrHid;j++)
			weightOut[i][j] += lr * errorOut[i] * cell[j];
	}

}



void CSmodule_NNLSM::initNet(
	int nrIn, int nrInCon, int nrHidW, int nrHidH, int nrHidCon, int nrOut)
{
	bool oldReady = ready;
	ready = false;

	this->nrIn = nrIn;
	this->nrInCon = nrInCon;
	this->nrHidW = nrHidW;
	this->nrHidH = nrHidH;
	this->nrHid = nrHidW*nrHidW*nrHidH;
	this->nrHidCon = nrHidCon;
	this->nrOut = nrOut;

	brainWash();

	ready = oldReady;
}

void CSmodule_NNLSM::brainWash()
{

	srand((int)*_seed);

	for (int i=0;i<nrHid;i++)
	{
		// reset cell state
		cell[i] = celltemp[i] = 0.0;
		// lowpass filter coefficient
		celllp[i] = (csfloat)rand()/RAND_MAX * (csfloat)rand()/RAND_MAX;
		celllp[i] = powf(celllp[i], filterPower);
	}

	// input->hidden weights
	for (int i=0;i<nrInCon;i++)
	{
		// random input
		weightInIndex[i] = _in[ i % nrIn ];
		// random hidden cell
		weightInHidIndex[i] = &celltemp[ rand() % nrHid ];
		// connection weight
		weightIn[i] = ((csfloat)rand()/RAND_MAX-(csfloat)rand()/RAND_MAX) * *_rampl;
	}

	// hidden->hidden connections
	for (int i=0;i<nrHidCon;)
	{
		int
			// a random cell
			ci = rand() % nrHid,
			// position of cell[ci]
			x1 = ci%nrHidW,
			y1 = (ci/nrHidW)%nrHidW,
			z1 = ci/(nrHidW*nrHidW),
			// random position to connect with
			x2 = rand()%nrHidW,
			y2 = rand()%nrHidW,
			z2 = rand()%nrHidH,
			/*
			x2 = min(nrHidW-1, abs(x1 + rand()%rad - rand()%rad)),
			y2 = min(nrHidW-1, abs(x1 + rand()%rad - rand()%rad)),
			z2 = min(nrHidH-1, abs(x1 + rand()%rad - rand()%rad)),
			*/
			// the random cell index
			cj = x2 + (y2 + z2*nrHidW)*nrHidW;

		// get distance
		csfloat d = CSdist((csfloat)x1,(csfloat)y1,(csfloat)z1,(csfloat)x2,(csfloat)y2,(csfloat)z2);
		// skip if too far
		if ((d<1.0)||(d>connectRad)) continue;

		// create connection
		weightHidFromIndex[i] = &cell[cj];
		weightHidToIndex[i] = &celltemp[ci];
		// set weight according to distance
		d = 1.0-min(0.99, d/connectRad);
		weightHid[i] = d * (csfloat)(rand()-rand())/RAND_MAX * *_rampl;

		//printf("%d,%d,%d - %d,%d,%d w:%g\n",x1,y1,z1,x2,y2,z2,weightHid[i]);
		i++;
	}

	// output cells
	for (int i=0;i<nrOut;i++)
	{
		*_out[i] = 0.0;
		errorOut[i] = 0.0;

		// random hidden->output weights
		for (int j=0;j<nrHid;j++)
			weightOut[i][j] =
				((csfloat)rand()/RAND_MAX-(csfloat)rand()/RAND_MAX) * *_ramp;
	}

}


void CSmodule_NNLSM::draw(int offx, int offy, float zoom)
{
	CSmodule::draw(offx,offy,zoom);

	if (nrHid<1) return;

	// draw hidden cell states
	int a = max(2, zoom*(width - 10));
	int x1 = offx+zoom*(x+5);
	int y1 = offy+zoom*(y+(headerHeight>>1));
	int y2 = offy+zoom*(y+headerHeight*2/3);
	for (int i=0;i<a;i++)
	{
		int ci = i*nrHid/a;
		uchar c = min(255,max(0, abs(cell[ci]*255) ));
		if (cell[ci]>0)
			fl_color(c << 16);
		else
			fl_color(c << 24);

		fl_yxline(x1+i, y1, y2);
	}

}





















char *CSmodule_SOM::docString()
{
	const char *r = "\
kohenan network or self organizing map\
";
	return const_cast<char*>(r);
}


CSmodule_SOM* CSmodule_SOM::newOne()
{
	return new CSmodule_SOM();
}


void CSmodule_SOM::init()
{
	gname = copyString("neuro");
	bname = copyString("self organizing map");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- layout ---

	minWidth = width = 220;
	minHeight = 400;
	canResize = true;

	// --- inputs outputs ----

	char n[128],n1[128];
	for (int i=0;i<CSMOD_SOM_MAX_PARAM;i++)
	{
		sprintf(n,"pi%d",i);
		sprintf(n1,"param #%d",i+1);
		__p_in[i] = createInput(n,n1);
		_p_in[i] = &__p_in[i]->value;

	}

	_clr = &(createInput("clr","CLEAR")->value);
	_seed = &(createInput("sed","seed",rand()%1000)->value);
	_ramp = &(createInput("ram","rnd.amp",0.01)->value);

	_gateStore = &(createInput("gs","STORE")->value);
	_opac = &(createInput("opa","opacity",0.1)->value);
	_rad = &(createInput("rad","radius",3.0)->value);

	_gateRead = &(createInput("gr","READ")->value);
	_x = &(createInput("x","X")->value);
	_y = &(createInput("y","Y")->value);

	_gateMatch = &(createInput("gm","MATCH")->value);

	for (int i=0;i<CSMOD_SOM_MAX_PARAM;i++)
	{
		sprintf(n,"po%d",i);
		sprintf(n1,"param #%d",i+1);
		__p_out[i] = createOutput(n,n1);
		_p_out[i] = &__p_out[i]->value;

	}

	_ox = &(createOutput("ox","X")->value);
	_oy = &(createOutput("oy","Y")->value);

	onResize();

    // -- nn --

    brainWash();

	setNrParam(3);
	nrX = 8;
	nrY = 8;

	// radius map
	for (int rad=0;rad<CSMOD_SOM_MAX_WIDTH;rad++)
	for (int y=0;y<CSMOD_SOM_MAX_WIDTH;y++)
	for (int x=0;x<CSMOD_SOM_MAX_WIDTH;x++)
	{
		csfloat d = CSdist(
			(csfloat)x,(csfloat)y,(csfloat)0,
			(csfloat)rad,(csfloat)rad,(csfloat)0);
		if (d==0)
			d = 1.0;
		else
			d = (d>rad)? 0.0 : 1.0-d/rad;

		radamp[(y*CSMOD_SOM_MAX_WIDTH+x)*CSMOD_SOM_MAX_WIDTH+rad] = d;
	}

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrp","nr parameters", nrParam, 1, CSMOD_SOM_MAX_PARAM);
	createPropertyInt("nrx","width", nrX, 1, CSMOD_SOM_MAX_WIDTH);
	createPropertyInt("nry","height", nrY, 1, CSMOD_SOM_MAX_WIDTH);

	lclr = lgateStore = lgateMatch = 0.0;

}

void CSmodule_SOM::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrp")==0)
		setNrParam(prop->ivalue);
	else
	if (strcmp(prop->name,"nrx")==0)
		nrX = prop->ivalue;
	else
	if (strcmp(prop->name,"nry")==0)
		nrY = prop->ivalue;
	else
	CSmodule::propertyCallback(prop);
}


void CSmodule_SOM::setNrParam(int newNr)
{
	nrParam = newNr;
	for (int i=0;i<CSMOD_SOM_MAX_PARAM;i++)
	{
		bool b = (i<nrParam);
		setConnectorActive(__p_in[i], b);
		setConnectorActive(__p_out[i], b);
	}

	arrangeConnectors();
}

void CSmodule_SOM::onResize()
{
	sx = 80;
	sw = width-sx*2;
	sy = headerHeight + 10;
	sh = sw;
	height = max(sy+sh+10,max(height,minHeight));
}


void CSmodule_SOM::step()
{
	// reset net
	if ( (lclr<=0) && ( *_clr>0 ) ) brainWash();
	lclr = *_clr;

	bool matchOnly = ( (lgateMatch<=0) && ( *_gateMatch>0 ) );
	lgateMatch = *_gateMatch;

	// store or match an entry
	if ( matchOnly || ( (lgateStore<=0) && ( *_gateStore>0 ) ) )
	{
		int x,y;
		bestMatch(_p_in, &x,&y);
		*_ox = x;
		*_oy = y;

		// only output the parameters there
		if (matchOnly)
		{
			int	p = ((y<<CSMOD_SOM_MAX_WIDTH_SHIFT)+x)<<CSMOD_SOM_MAX_PARAM_SHIFT;
			csfloat *c = &cell[p];
			for (int k=0;k<nrParam;k++)
				*_p_out[k] = *c++;
			return;
		}

		// draw in the map
		csfloat opac = max((csfloat)0,min((csfloat)1.0, *_opac ));
		if (opac<=0.0) return;

		// draw single dot
		if (*_rad<1.0)
		{
			csfloat opac1 = 1.0-opac;
			int p=((y<<CSMOD_SOM_MAX_WIDTH_SHIFT)+x) << CSMOD_SOM_MAX_PARAM_SHIFT;

			// match cell parameters
			for (int k=0;k<nrParam;k++)
			{
				cell[p] = cell[p] * opac1 + opac * *_p_in[k];
				p++;
			}
		}
		else
		// draw circle
		{
			int rad = min(CSMOD_SOM_MAX_WIDTH, *_rad);
			for (int j1=0;j1<rad*2+1;j1++)
			{
				int j = j1+y-rad;
				if ((j<0)||(j>=nrY)) continue;

				int pj = (j<<CSMOD_SOM_MAX_WIDTH_SHIFT)<<CSMOD_SOM_MAX_PARAM_SHIFT;
				for (int i1=0;i1<rad*2+1;i1++)
				{
					int i = i1+x-rad;
					if ((i<0)||(i>=nrY)) continue;

					int p = pj + (i<<CSMOD_SOM_MAX_PARAM_SHIFT);

					// amplitude
					csfloat
						o = opac*radamp[
							(((j1<<CSMOD_SOM_MAX_WIDTH_SHIFT)+i1)<<CSMOD_SOM_MAX_WIDTH_SHIFT)+rad],
						o1 = 1.0-o;

					// match cell parameter
					for (int k=0;k<nrParam;k++)
					{
						cell[p] = cell[p] * o1 + o * *_p_in[k];
						p++;
					}
				}
			}
		}
	}
	lgateStore = *_gateStore;


	// read X/Y pos
	if ( (lgateRead<=0) && ( *_gateRead>0 ) )
	{
		int
			x = max(0,min(nrX-1, (int)*_x )),
			y = max(0,min(nrY-1, (int)*_y )),
		// feed cell into outputs
			p = ((y<<CSMOD_SOM_MAX_WIDTH_SHIFT)+x)<<CSMOD_SOM_MAX_PARAM_SHIFT;
		csfloat *c = &cell[p];
		for (int k=0;k<nrParam;k++)
			*_p_out[k] = *c++;
	}
	lgateRead = *_gateRead;


}




void CSmodule_SOM::draw(int offx, int offy, float zoom)
{
	CSmodule::draw(offx,offy,zoom);

	int
		bx = offx+zoom*(x+sx),
		bw = zoom*sw,
		by = offy+zoom*(y+sy),
		bh = zoom*sh;

	// background
	fl_color(FL_DARK3);
	fl_rectf(bx,by,bw,bh);

	if ((nrX<=0)||(nrY<=0)) return;
	int
		bw1 = bw/nrX,
		bh1 = bh/nrY;

	// map
	for (int j=0;j<nrY;j++)
	for (int i=0;i<nrX;i++)
	{
		csfloat f = cell[ ((j<<CSMOD_SOM_MAX_WIDTH_SHIFT)+i)<<CSMOD_SOM_MAX_PARAM_SHIFT ];
		if (f<0)
			fl_color(max(0,min(255, (int)(-f*255) ))<<24);
		else
			fl_color(max(0,min(255, (int)(f*255) ))<<16);
		fl_rectf(bx+bw1*i,by+bh1*j, bw1+1,bh1+1);
	}

	// cur pos
	//fl_color(FL_WHITE);
	//fl_rect(bx+bw1* *_ox, by+bh1* *_oy, bw1, bh1);
}





void CSmodule_SOM::brainWash()
{
	srand((int)*_seed);

	for (int i=0;i<CSMOD_SOM_MAX_WIDTH*CSMOD_SOM_MAX_WIDTH*CSMOD_SOM_MAX_PARAM;i++)
	{
		cell[i] = *_ramp * (csfloat)rand()/RAND_MAX;
	}

}

void CSmodule_SOM::bestMatch(csfloat **param, int *x, int *y)
{
	csfloat d,bd = 0;
	*x = *y = -1;
	for (int j=0;j<nrY;j++)
	{
		int p1 = (j<<CSMOD_SOM_MAX_WIDTH_SHIFT)<<CSMOD_SOM_MAX_PARAM_SHIFT;
		for (int i=0;i<nrX;i++)
		{
			int p = p1 + (i<<CSMOD_SOM_MAX_PARAM_SHIFT);
			// get difference to each field
			d = 0.0;
			for (int k=0;k<nrParam;k++)
			{
				d += fabs(*param[k] - cell[p]);
				p++;
			}
			// remember best
			if ((*x<0)||(d<bd))
			{
				bd = d;
				*x = i; *y = j;
			}
		}
	}

	if (*x<0) { *x = *y = 0; return; }
}


