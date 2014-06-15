#include "csmodcontainer.h"
#include "csmodule_container.h"



// ------------------------ CSmodule_Container --------------------------------

char *CSmodule_Container::docString()
{
	const char *r = "\
this module is used to collect or contain other modules. when double-clicking on it's \
header you will access the contents of the container module. it will be \
empty on creation. you can place any module (also audio ins/outs, new container modules) inside.\n\n\
to provide an input and output to the parent container press alt + 'i' and 'o' \
when inside. to rename the inputs/outputs use their individual property windows.\n\
PLEASE NOTE: *currently* duplicating (ctrl+D) container inputs and outputs wont work properly.\n\
\n\
each container module has an <active> input. if this is > 0.0 the container within is active and will \
be processed normaly. if it goes <= 0.0, the container (and all sub-containers) wents in-active. no \
audio outs will be mixed to the global output buffer, no scope3d modules will be rendered. all is completely OFF!\
";
	return const_cast<char*>(r);
}


CSmodule_Container* CSmodule_Container::newOne()
{
	return new CSmodule_Container();
}

CSmodule_Container::~CSmodule_Container()
{
	delete container;
}

void CSmodule_Container::copyFrom(CSmodule *mod)
{
	ready = false;

	// first copy the default settings
	// container inputs and outputs are not catched here
	CSmodule::copyFrom(mod);

	// now go through original container and copy each module

	CSmodule_Container *c = dynamic_cast<CSmodule_Container*>(mod);
	if (!c) return;

	active = c->active;

	for (int i=0;i<c->container->nr;i++)
	{
		// add a new instance
		CSmodule
			*m,
			// use this module as source
			*mFrom = c->container->module[i];

		// ...except, check for special case of container input/output modules
		if (strcmp(mFrom->bname, "container input")==0)
		{
			CSmodule_ContainerInput *mci =
				dynamic_cast<CSmodule_ContainerInput*>(mFrom);
			m = newContainerCon(CS_CONDIR_IN, mci->containerConnector->name, mci->containerConnector->uname);
		}
		else
		if (strcmp(mFrom->bname, "container output")==0)
		{
			CSmodule_ContainerOutput *mco =
				dynamic_cast<CSmodule_ContainerOutput*>(mFrom);
			m = newContainerCon(CS_CONDIR_OUT, mco->containerConnector->name, mco->containerConnector->uname);
		}
		else
			m = container->addModule(mFrom);

		// copy settings
		m->copyFrom(mFrom);
		// also copy unique index name
		free(m->name); m->name = copyString(mFrom->name);
	}

	// the inputs (and outputs) should be there now
	// copy input connector values
	for (int i=0;i<min(nrCon,mod->nrCon);i++)
		setConnectorValue(con[i], mod->con[i]->uvalue);

	// now copy connectivity...
	for (int i=0;i<c->container->nrCon;i++)
		container->connect(
			c->container->con[i]->from->parent->name,
			c->container->con[i]->from->name,
			c->container->con[i]->to->parent->name,
			c->container->con[i]->to->name);

	// copy container design settings
	container->offx = c->container->offx;
	container->offy = c->container->offy;
	container->zoom = c->container->zoom;
	container->offx_ = c->container->offx_;
	container->offy_ = c->container->offy_;
	container->zoom_ = c->container->zoom_;

	ready = true;
}



void CSmodule_Container::init()
{
	gname = copyString("system");
	bname = copyString("container");
	name = copyString(bname);
	uname = copyString(bname);

	fileVersion = 2;

	// --- inputs outputs ----

	_active = &createInput("act","active", 1.0)->value;

	// rest will be created at runtime through adding CSmodule_ContainerInput/Output

	// ---- properties ----

	createNameProperty();
	//createPropertyInt("sr","sample rate", sampleRate, 1, 44100*16);

	// --- look ---

	CSsetColor(bodyColor, 80,120,138);
	updateColors();

	active = true;
	lactive = 1.0;
}

void CSmodule_Container::onAddModule()
{
	// create container
	container = new CSmodContainer();
	// container should know parent
	container->rootContainer = parent->rootContainer;
	// assign this module, so container knows he's part of this module
	container->containerModule = this;
	// make container background look different
	CSsetColor(container->backgroundColor, 40,42,50);
}

void CSmodule_Container::storeAddBeforeValue(FILE *buf)
{
	if (fileVersion<2) return;

	// store connector settings
	storeConnectors(buf);
}

void CSmodule_Container::restoreAddBeforeValue(FILE *buf)
{
	if (readFileVersion<2) return;

	restoreConnectors(buf);
}

void CSmodule_Container::storeAdd(FILE *buf)
{
	fprintf(buf,"\n");
	container->store(buf);
}

void CSmodule_Container::restoreAdd(FILE *buf)
{
	container->restore(buf);
}


void CSmodule_Container::arrangeConnectors()
{
	// since connectors can be removed
	// reset module height
	minHeight = headerHeight;
	height = minHeight;
	// and arrange connectors
	CSmodule::arrangeConnectors();
}

int CSmodule_Container::nrContainerInputs()
{
	// count all inputs that start with 'in'
	int n =0;
	for (int i=0;i<nrCon;i++)
	if ((con[i]->dir==CS_CONDIR_IN) &&
		(con[i]->name) &&
		(con[i]->name[0]=='i') &&
		(con[i]->name[1]=='n')
		)
		n++;
	return n;
}


int CSmodule_Container::swapContainerInput(int index, int dir)
{
	if ((index<0) || (index>=nrCon)) return max(0,min(nrCon-1, index ));
	if (con[index]->dir != CS_CONDIR_IN) return index;

	if (dir<0)
	for (int i=index-1;i>=0;i--)
	if (con[i]->dir == CS_CONDIR_IN)
	{
		CSmoduleConnector *bu = con[i];
		con[i] = con[index]; con[index] = bu;
		arrangeConnectors();
		return i;
	} else
	for (int i=index+1;i<nrCon;i++)
	if (con[i]->dir == CS_CONDIR_IN)
	{
		CSmoduleConnector *bu = con[i];
		con[i] = con[index]; con[index] = bu;
		arrangeConnectors();
		return i;
	}

	return index;
}


void CSmodule_Container::step()
{
	// trap active change
	if (*_active != lactive) setActive(*_active>0);
	lactive = *_active;

	if (active)	container->step();
}


void CSmodule_Container::stepLow()
{
	/*
	// trap active change
	!! TODO !!
	this is not good because multiple thread access
	*_active could be 0.0 for a small time during update!
	on the other hand it would be cool to have stepLow also activate the container

	if (*_active != lactive) setActive(*_active>0);
	lactive = *_active;
	*/

	if (active) container->stepLow();
}

void CSmodule_Container::setActive(bool act)
{
	active = act;
	// set the active flag in CSmodContainer
	container->active = act;

	// maybe clear outs here when inactive??
	if (!active)
	{
		for (int i=0;i<nrCon;i++)
		if (con[i]->dir==CS_CONDIR_OUT)
			con[i]->value = 0.0;
	}

	// go through container and (de-)activate all subs
	for (int i=0;i<container->nr;i++)
	{
		CSmodule_Container *c =
			dynamic_cast<CSmodule_Container*>(container->module[i]);
		if (!c) continue;

		// and continue recusively
		c->setActive(act);
	}
}


CSmodule *CSmodule_Container::newContainerCon
	(int dir, const char *Name, const char *Uname)
{
	/*
	need to:
		- create a CSmoduleConnector in this->
		- create a CSmodule_ContainerInput/Output in this->container;
		- link them together
	*/

	// generate an indexed name for the connector
	char *cName = (Name)? copyString(Name) :
		int2char( (dir==CS_CONDIR_IN)? "in":"out",
			(dir==CS_CONDIR_IN)? nrContainerInputs()+1:nrOutputs()+1 );
	char *cUname = (Uname)? copyString(Uname) : copyString(cName);

	// create a connector or use existing with matching name
	int i= getConnectorIndex(cName);
	CSmoduleConnector *co = (i>=0)? con[i] :
		(dir==CS_CONDIR_IN)? createInput(cName,cUname) : createOutput(cName,cUname);

	free(cName);

	// create the connector module

	// ...now, *co is connector, *m becomes the CSmodule_ContainerInput/Output module
	if (dir==CS_CONDIR_IN)
	{
		CSmodule_ContainerInput *m1 = new CSmodule_ContainerInput();
		CSmodule_ContainerInput *m = (CSmodule_ContainerInput*)container->addModule(m1);
		delete m1;
		// let m know the container module and connector
		m->containerModule = this;
		m->containerConnector = co;
		// assign name property
		if (m->uname) free(m->uname);
		m->uname = cUname;
		m->createPropertyString("modname", "name of connector", m->uname);
		return m;
	}
	else
	{
		CSmodule_ContainerOutput *m1 = new CSmodule_ContainerOutput();
		CSmodule_ContainerOutput *m = (CSmodule_ContainerOutput*)container->addModule(m1);
		delete m1;
		// let m know the container module and connector
		m->containerModule = this;
		m->containerConnector = co;
		// assign name property
		m->uname = cUname;
		m->createPropertyString("modname", "name of connector", m->uname);
		return m;
	}

}













// ------------------------ CSmodule_ContainerInput --------------------------------

CSmodule_ContainerInput::CSmodule_ContainerInput()
{
	containerModule = 0;
	containerConnector = 0;
}

CSmodule_ContainerInput::~CSmodule_ContainerInput()
{
	// when deleting the module, also remove the input of container module
	if ((dynamic_cast<CSmodule_Container*>(containerModule))&&(containerConnector))
		containerModule->deleteConnector(
			containerModule->getConnectorIndex(containerConnector)
			);
}

CSmodule_ContainerInput* CSmodule_ContainerInput::newOne()
{
	return new CSmodule_ContainerInput();
}


void CSmodule_ContainerInput::init()
{
	bname = copyString("container input");
	name = copyString("input");
	uname = copyString(name);

	canAdd = false;

	minWidth = 80;
	width = minWidth;

	CSsetColor(bodyColor, 90,80,70);
	updateColors();

	// --- inputs outputs ----

	_out = &(createOutput("out","input")->value);

	// ---- properties ----

	// will be done later... see CSmodule_Container::newContainerInput() */

}


void CSmodule_ContainerInput::propertyCallback(CSmodProperty *prop)
{
	// call base class method
	CSmodule::propertyCallback(prop);

	// set container module's input name
	if ((prop->typ==CS_PROP_STRING)&&(strcmp(prop->name,"modname")==0)
	&&(containerConnector)) {
		if (containerConnector->uname) free(containerConnector->uname);
		containerConnector->uname = copyString(prop->svalue);
	}

}


void CSmodule_ContainerInput::step()
{
	// get the value from the original container module input
	if (containerConnector) *_out = containerConnector->value;
}







// ------------------------ CSmodule_ContainerOutput --------------------------------


CSmodule_ContainerOutput::CSmodule_ContainerOutput()
{
	containerModule = 0;
	containerConnector = 0;
}

CSmodule_ContainerOutput::~CSmodule_ContainerOutput()
{
	// when deleting the module, also remove the output of container module
	if ((dynamic_cast<CSmodule_Container*>(containerModule))&&(containerConnector))
		containerModule->deleteConnector(
			containerModule->getConnectorIndex(containerConnector)
			);
}

CSmodule_ContainerOutput* CSmodule_ContainerOutput::newOne()
{
	return new CSmodule_ContainerOutput();
}


void CSmodule_ContainerOutput::init()
{
	containerModule = 0;
	containerConnector = 0;

	bname = copyString("container output");
	name = copyString("output");
	uname = copyString(name);

	canAdd = false;

	minWidth = 80;
	width = minWidth;

	CSsetColor(bodyColor, 90,80,70);
	updateColors();

	// --- inputs outputs ----

	_in = &(createInput("in","output")->value);

	// ---- properties ----

	// will be done later... see CSmodule_Container::newContainerOutput() */

}


void CSmodule_ContainerOutput::propertyCallback(CSmodProperty *prop)
{
	// call base class method
	CSmodule::propertyCallback(prop);

	// set container module's input name
	if ((prop->typ==CS_PROP_STRING)&&(strcmp(prop->name,"modname")==0)
	&&(containerConnector)) {
		if (containerConnector->uname) free(containerConnector->uname);
		containerConnector->uname = copyString(prop->svalue);
	}

}


void CSmodule_ContainerOutput::step()
{
	// send the value to the original container module output
	if (containerConnector) containerConnector->value = *_in;
}
