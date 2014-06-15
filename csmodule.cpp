/*

	CYMASONICS MODULAR EDITOR

	CSmodule source

*/


#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <typeinfo>

#include "csmodule.h"
#include "csmodcontainer.h"
#include "csmodule_container.h"

using namespace std;

// ------------------------------------- small helper -----------------------------------------------------

char *copyString(const char *str)
{
	int len = strlen(str);
	char *s = (char*) calloc(len+1,1);
	strcpy(s, str);
	return s;
}

char *addString(const char *str, const char *suffix)
{
	int l1 = strlen(str);
	int l2 = strlen(suffix);
	int len = l1+l2;
	char *s = (char*) calloc(len+1,1);

	strcpy(s, str);
	char *d = s+l1;
	strcpy(d, suffix);

	return s;
}


char *int2char(int i)
{
	char *buf = (char*)calloc(64,1);
	sprintf(buf, "%d",i);
	char *b = copyString(buf);
	free(buf);
	return b;
}

char *int2char(const char *prefix, int i)
{
	char *buf = (char*)calloc(64+strlen(prefix), 1);
	sprintf(buf, "%s%d",prefix,i);
	char *b = copyString(buf);
	free(buf);
	return b;
}


char *readString(FILE *buf)
{
	char *str = (char*) calloc(1024,1);
	fscanf(buf, "%s", str);

	if (str[0]=='"')
	{
		int l=strlen(str);
		if (str[l-1]=='"')
		{
			// ok, simply remove quotes
			char *ret = (char*) calloc(l-1, 1);
			char *sp = str;
			sp++; strncpy(ret, sp, l-2);
			free(str);
			return ret;
		}
		// no trailing " in this string? so keep on reading

		// remove first "
		char *ret = (char*) calloc(l+1024, 1);
		char *sp = str;
		sp++; strncpy(ret, sp, l-1);
		sp = ret + l-1;
		for (int c=0;c!='"'; )
		{
			c = fgetc(buf);
			if (c!='"') {
				*sp = (char)c;
				sp++;
			}
		}
		ret = (char*) realloc(ret, strlen(ret)+1);
		return ret;
	}

	str = (char*) realloc(str, strlen(str)+1);
	return str;
}














// -------------------------------- CSmoduleWindow --------------------------------------------------------

CSmoduleExtWindow::CSmoduleExtWindow(CSmodule *parent)
					: Fl_Window(20,20,CSMOD_INIT_EXTWIN_WIDTH,CSMOD_INIT_EXTWIN_HEIGHT,
					copyString(parent->name))
{
	#ifdef CSMOD_DEBUGG
	printf("CSmoduleExtWindow::CSmoduleWindow(0x%p)\n",parent);
	#endif

	end(); // not adding anything

	ready = false;
	this->parent = parent;

	size_range(160,160);
	oldx = x(); oldy = y();
	oldw = w(); oldh = h();

	//set_non_modal(); // always stay on top
	ready = true;
}

void CSmoduleExtWindow::draw()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmoduleExtWindow::draw()\n");
	#endif

	parent->drawWindow();
}

/*
void CSmoduleExtWindow::drawAndSave(const char *filename)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmoduleExtWindow::draw(\"%s\")\n",filename);
	#endif

	parent->drawWindow();
	parent->storeWindow(filename);
}
*/

void CSmoduleExtWindow::resize(int x,int y,int w,int h)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmoduleExtWindow::resize(%d,%d,%d,%d)\n",x,y,w,h);
	#endif

	Fl_Window::resize(x,y,w,h);

	if ((!ready)||(!parent->ready)) return;

	// when pos is same, this has to be a resize event
	if ((oldx==x)&&(oldy==y))
		parent->onResizeWindow(w,h);

	oldx = x; oldy = y;
}



// ----------------------------------- CSmodule -----------------------------------------------------------

CSmodule::CSmodule()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSmodule::CSmodule()\n");
	#endif
	clear();
}

CSmodule::~CSmodule()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSmodule::~CSmodule()\n");
	#endif
	release();
}

void CSmodule::clear()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSmodule::clear()\n");
	#endif

	parent = 0;
	parentRoot = 0;
	ready = false;

	bname = 0;
	name = 0;
	uname = 0;
	gname = 0;

	nrCon = 0;
	con = 0;
	nrProp = 0;
	property = 0;

	setSampleRate(44100);

	isSelected = false;
	isSelectedArea = false;
	canAdd = true;
	isStep = false;
	canResize = false;

	fileVersion = 1;
	readFileVersion = 1;

	x = 0;
	y = 0;
	headerHeight = 20;
	inputStart = 10;
	inputHeight = 25;
	outputStart = inputStart;
	outputHeight = inputHeight;
	minWidth = 120;
	minHeight = headerHeight + 2; // arbitrary at this point
	minHeight1 = minHeight;
	width = minWidth;
	height = minHeight;
	connectorRad = 5;
	connectorFocusRad = 7;

	CSsetColor(bodyColor, 90,120,108);
	CSsetColor(headerColor, 50,90,120);
	CSsetColor(headerFocusColor, 80,130,170);
	CSsetColor(headerFontColor, 255,255,255);
	CSsetColor(connectorColor, 200,200,200);
	CSsetColor(connectorFocusColor, 255,255,255);
	CSsetColor(labelColor, 250,250,250);
	CSsetColor(labelFocusColor, 240,255,240);
	CSsetColor(labelEditColor, 255,255,100);
	updateColors();

	window = 0;
	doDrawWindow = true;
	doCallDrawWindow = false;
}


void CSmodule::release()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSmodule::release()\n");
	#endif

	if (bname) free(bname);
	if (name) free(name);
	if (uname) free(uname);
	if (gname) free(gname);

	// release connectors
	if (con)
	{
		for (int i=0;i<nrCon;i++) releaseConnector(con[i]);
		free(con);
	}

	// release properties
	if (property)
	{
		for (int i=0;i<nrProp;i++) releaseProperty(property[i]);
		free(property);
	}

	// destroy window
	if (window) delete window;

	clear();
}




void CSmodule::copyFrom(CSmodule *mod)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSmodule::copyFrom(0x%p)\n",mod);
	#endif

	// be sure it's the same class
	if (strcmp(bname, mod->bname))
	{
		printf("WARNING: CSmodule::copyFrom(): class clash: %s -> %s\n",mod->bname,name);
		return;
	}

	// copy username
	if (uname) free(uname); uname = copyString(mod->uname);

	// design settings
	x = mod->x; y = mod->y;

	// audio settings
	setSampleRate(mod->sampleRate);

	// properties
	for (int i=0;i<nrProp;i++)
	{
		switch (property[i]->typ)
		{
			case CS_PROP_INT: property[i]->ivalue = mod->property[i]->ivalue; break;
			case CS_PROP_FLOAT: property[i]->fvalue = mod->property[i]->fvalue; break;
			case CS_PROP_STRING:
				if (property[i]->svalue) free(property[i]->svalue);
				property[i]->svalue = copyString(mod->property[i]->svalue);
				break;
		}
		// signify a property change
		propertyCallback(property[i]);
	}

	/* input values
	   note: for variable number of inputs both modules should be equal
		at this point, since properties have been set already. however,
		for container modules the inputs (and outputs) will be created when
		all the container contents are copied so they are not here yet, so
		we use min(nrCon,mod->nrCon) to be save anyway. */
	for (int i=0;i<min(nrCon,mod->nrCon);i++)
		setConnectorValue(con[i], mod->con[i]->uvalue);

}




	//------------------ load/save ---------------------------------------------

void CSmodule::storeValues(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodule::storeValues(0x%p)\n",buf);
	#endif

	fprintf(buf, "val { ");
	for (int i=0;i<nrCon;i++)
	if (
		// if input
		(con[i]->dir==CS_CONDIR_IN) &&
		// and uservalue != defaultValue
		(con[i]->uvalue!=con[i]->defaultValue)
		)
	{
		// id and value pairs
		fprintf(buf, "\"%s\" %f ", con[i]->name, con[i]->uvalue);
	}

	fprintf(buf, "} ");
}

void CSmodule::storeProperties(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodule::storeProperties(0x%p)\n",buf);
	#endif

	fprintf(buf, "properties { ");
	for (int i=0;i<nrProp;i++)
	{
		// id and value pairs
		fprintf(buf, "\"%s\" ", property[i]->name);
		switch (property[i]->typ)
		{
			case CS_PROP_INT: fprintf(buf, "%d ", property[i]->ivalue); break;
			case CS_PROP_FLOAT: fprintf(buf, "%f ", property[i]->fvalue); break;
			case CS_PROP_STRING: fprintf(buf, "\"%s\" ", property[i]->svalue); break;
		}
	}

	fprintf(buf, "} ");
}

void CSmodule::store(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodule::store(0x%p)\n",buf);
	#endif

	int conFileVersion = (parent)? parent->fileVersion:CSMOD_CONTAINER_FILEVERSION;

	fprintf(buf, "module { ");

	// base class name
	fprintf(buf, "\"%s\" ", bname);
	// special case for container input/output
	if (CSMOD_CONTAINER_FILEVERSION>=4)
	{
		if (!strcmp(bname,"container input"))
		{
			CSmodule_ContainerInput *m =
				dynamic_cast<CSmodule_ContainerInput*>(this);
			fprintf(buf, "\"%s\" ", m->containerConnector->name);
		} else
		if (!strcmp(bname,"container output"))
		{
			CSmodule_ContainerOutput *m =
				dynamic_cast<CSmodule_ContainerOutput*>(this);
			fprintf(buf, "\"%s\" ", m->containerConnector->name);
		}
	}
	// session id, and user name
	fprintf(buf, "\"%s\" \"%s\" ", name, uname);
	// file version
	fprintf(buf, "%d ",fileVersion);
	// position
	fprintf(buf, "%d %d ",x,y);
	// size
	if (conFileVersion>=3)
		fprintf(buf, "%d %d ",width,height);

	// store property values
	storeProperties(buf);

	// store additional data before the value block
	/* 2011/03/23 */
	storeAddBeforeValue(buf);

	// store the input values
	storeValues(buf);

	// additional user data
	storeAdd(buf);

	fprintf(buf, "} ");
}

void CSmodule::storeConnectors(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodule::storeConnectors(0x%p)\n",buf);
	#endif
	fprintf(buf, " c0n { ");
	fprintf(buf, "%d ", nrCon);
	for (int i=0;i<nrCon;i++)
	{
		fprintf(buf, "%d %d \"%s\" \"%s\" ",
			con[i]->dir, con[i]->typ,
			con[i]->name, con[i]->uname);
	}
	fprintf(buf, " } ");
}

void CSmodule::storeAddBeforeValue(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodule::storeAddBeforeValue(0x%p) in base class\n",buf);
	#endif

	// do nothing here
}


void CSmodule::storeAdd(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodule::storeAdd(0x%p) in base class\n",buf);
	#endif

	// do nothing here
}




void CSmodule::restoreValues(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodule::restoreValues(0x%p)\n",buf);
	#endif

	fscanf(buf, " val { ");
	bool doit = true;
	while (doit)
	{
		char *str = readString(buf);

		if (strcmp(str,"}")==0) doit = false;
		else
		{
			float v;
			int e = fscanf(buf, " %f ", &v);
			if (e!=1) CSERROR("module::restore (input value)");
			int i=getConnectorIndex(str);
			if (i>=0) setConnectorValue(con[i], v);
		}

		free(str);
	}

}

void CSmodule::restoreProperties(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodule::restoreProperties(0x%p)\n",buf);
	#endif

	fscanf(buf, " properties {");
	bool doit = true;

	while (doit)
	{
		char *str = readString(buf);

		if (strcmp(str, "}")==0) { doit=false; free(str); continue; }

		int i=getPropertyIndex(str);
		free(str);

		// skip unknown properties
		if (i==-1) {
				str = readString(buf);
				free(str);
				continue;
				//CSERROR("module::restore (unknown property)");
			}

		// read property value
		int e=0;
		switch (property[i]->typ)
		{
			case CS_PROP_INT: e = fscanf(buf, " %d ", &property[i]->ivalue); break;
			case CS_PROP_FLOAT: e = fscanf(buf, " %f ", &property[i]->fvalue); break;
			case CS_PROP_STRING:
				if (property[i]->svalue) free(property[i]->svalue);
				property[i]->svalue = readString(buf); e=1; break;
		}
		if (e!=1) CSERROR("module::restore (incorrect property value)");

		// do whatever
		propertyCallback(property[i]);
	}

}

void CSmodule::restore(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodule::restore(0x%p)\n",buf);
	#endif

	int containerFileVersion = (parent)? parent->readFileVersion:CSMOD_CONTAINER_FILEVERSION;

	fscanf(buf, " module { ");
	// skip basename by reading to dummy
	if (name) free(name); name = readString(buf);
	// skip special case for container input/output
	if (parentRoot->readFileVersion>=4)
	{
		if ((!strcmp(bname,"container input"))||(!strcmp(bname,"container output")))
			name = readString(buf);
	}
	// assign id and user name
	name = readString(buf);
	if (uname) free(uname); uname = readString(buf);
	// read file version
	int e = fscanf(buf, " %d ",&readFileVersion);
	if (e!=1) CSERROR("module::restore (file version)");
	// read position
	e = fscanf(buf, " %d %d ",&x,&y);
	if (e!=2) CSERROR("module::restore (position)");
	// read size
	if (containerFileVersion>=3)
	{
		e = fscanf(buf, " %d %d ",&width,&height);
		if (e!=2) CSERROR("module::restore (size)");
		onResize();
		arrangeConnectors();
	}

	// get properties
	restoreProperties(buf);

	// get additional data before the value block
	/* 2011/03/23 */
	restoreAddBeforeValue(buf);

	// read input values
	restoreValues(buf);

	// get additional data
	restoreAdd(buf);

	fscanf(buf, " } ");

}

void CSmodule::restoreConnectors(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodule::restoreConnectors(0x%p)\n",buf);
	#endif

	fscanf(buf, " c0n { ");

	int nr, e = fscanf(buf, "%d", &nr);
	if (e!=1) CSERROR("CSmodule_Container::restoreConnector (nr of cons)");

	for (int i=0;i<nr;i++)
	{
		int di,ty;
		e = fscanf(buf, "%d %d", &di, &ty);
		if (e!=2) CSERROR("CSmodule_Container::restoreConnector (con dir/typ)");
		char *c1 = readString(buf);
		if (!c1) CSERROR("CSmodule_Container::restoreConnector (con name)");
		char *c2 = readString(buf);
		if (!c2) CSERROR("CSmodule_Container::restoreConnector (con uname)");

		// skip if existing
		e = getConnectorIndex(c1);
		if (e>=0) continue;

		// create that connector
		if (di==CS_CONDIR_IN)
			createInput(c1,c2);
		else
			createOutput(c1,c2);
	}

	fscanf(buf, " } ");
	arrangeConnectors();
}

void CSmodule::restoreAdd(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodule::restoreAdd(0x%p) in base class\n",buf);
	#endif

	// do nothing here
}

void CSmodule::restoreAddBeforeValue(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodule::restoreAddBeforeValue(0x%p) in base class\n",buf);
	#endif

	// do nothing here
}



	//------------------- properties -------------------------------------------

char *CSmodule::docString()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::docStrinf()\n");
	#endif

	const char *r =
"no help for this module available \n \
(override virtual char *docString() function!";
	return const_cast<char*>(r);
}

CSmodProperty *CSmodule::createProperty(int typ, const char *name, const char *uname)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::createProperty(%d, \"%s\", \"%s\")\n", typ,name, uname );
	#endif
	CSmodProperty *prop = new CSmodProperty;
	prop->typ = typ;
	prop->name = copyString(name);
	prop->uname = copyString(uname);
	prop->ivalue = 0;
	prop->fvalue = 0.0;
	prop->svalue = 0;
	prop->minValue = -0x10000;
	prop->maxValue = 0x10000;
	prop->parent = this;
	// allocate buffer for string
	if (typ==CS_PROP_STRING) prop->svalue = (char*)calloc(1024,sizeof(char));

	addProperty(prop);
	return prop;
}

CSmodProperty *CSmodule::createPropertyInt(const char *name, const char *uname, int theValue, int minValue, int maxValue)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::createPropertyInt(\"%s\", \"%s\", %d, %d, %d)\n", name, uname, theValue,minValue,maxValue );
	#endif
	CSmodProperty *prop = createProperty(CS_PROP_INT, name, uname);
	prop->ivalue = theValue;
	prop->minValue = minValue;
	prop->maxValue = maxValue;
	return prop;
}

CSmodProperty *CSmodule::createPropertyFloat(const char *name, const char *uname, csfloat theValue, csfloat minValue, csfloat maxValue)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::createPropertyFloat(\"%s\", \"%s\", %f, %f, %f)\n", name, uname, theValue,minValue,maxValue );
	#endif
	CSmodProperty *prop = createProperty(CS_PROP_FLOAT, name, uname);
	prop->fvalue = theValue;
	prop->minValue = minValue;
	prop->maxValue = maxValue;
	return prop;
}

CSmodProperty *CSmodule::createPropertyString(const char *name, const char *uname, const char *theValue)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::createPropertyString(\"%s\", \"%s\", \"%s\")\n", name, uname, theValue);
	#endif
	CSmodProperty *prop = createProperty(CS_PROP_STRING, name, uname);
	if (prop->svalue) free(prop->svalue);
	prop->svalue = copyString(theValue);
	return prop;
}

int CSmodule::getPropertyIndex(const char *name)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::getPropertyIndex(\"%s\")\n",name);
	#endif
	for (int i=0;i<nrProp;i++)
	{
		//printf(".%s. .%s.\n", name, property[i]->name);
		if (strcmp(name, property[i]->name)==0) return i;
	}
	return -1;
}

CSmodProperty *CSmodule::createNameProperty()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::createNameProperty()\n");
	#endif
	return createPropertyString("modname", "module name", uname);
}

void CSmodule::releaseProperty(CSmodProperty *prop)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::releaseProperty(0x%p)\n", prop);
	#endif
	if (prop->svalue) free(prop->svalue);
	if (prop->name) free(prop->name);
	if (prop->uname) free(prop->uname);
	free(prop);
}

void CSmodule::addProperty(CSmodProperty *prop)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::addProperty(0x%p)\n", prop);
	#endif
	if (!property)
	{
		property = (CSmodProperty**) calloc(1,sizeof(int));
		nrProp = 1;
	} else {
		nrProp++;
		property = (CSmodProperty**) realloc(property, nrProp*sizeof(int) );
	}
	property[nrProp-1] = prop;
}

void CSmodule::propertyCallback(CSmodProperty *prop)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::propertyCallback(0x%p)!\n", prop);
	#endif

	// set module name
	if ((prop->typ==CS_PROP_STRING)&&(strcmp(prop->name,"modname")==0)) {
		if (uname) free(uname);
		uname = copyString(prop->svalue);
	}

	refresh();
}

void CSmodule::rename(const char *newName)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::rename(%s)\n", newName);
	#endif

	if (uname) free(uname);
	uname = copyString(newName);
	int i = getPropertyIndex("modname");
	if (i>=0)
	{
		if (property[i]->svalue) free(property[i]->svalue);
		property[i]->svalue = copyString(newName);
	}

}







	//------------------- connectors -------------------------------------------

char *CSmodule::uniqueConnectorName(const char *name)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::uniqueConnectorName(\"%s\")\n",name);
	#endif
	char *buf = copyString(name);
	int count = 1;
	while (getConnectorIndex(buf)!=-1)
	{
		free(buf);
		buf = addString(name, int2char(count));
		/* TODO: think the int2char eats memory, see also CSmodContainer->getUniqueName... */
		count++;
	}
	return buf;
}

CSmoduleConnector *CSmodule::createConnector(int dir, int x,int y, const char *name, const char *uname, csfloat defaultValue)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::createConnector(%d, %d, %d, \"%s\", \"%s\",%f)\n",dir,x,y,name,uname,defaultValue);
	#endif
	CSmoduleConnector *con = new CSmoduleConnector;
	con->parent = this;
	con->dir = dir;
	con->typ = CS_CON_AUDIO;
	con->nrCon = 0;
	con->x = x;
	con->y = y;
	con->name = uniqueConnectorName(name);
	con->uname = copyString(uname);
	con->value = defaultValue;
	con->uvalue = defaultValue;
	con->defaultValue = defaultValue;
	// allocate fixed length (will be adjusted...)
	con->valueStr = (char*) calloc(128,1);
	con->valueStrE = 0;
	con->active = true;
	con->autoArrange = true;
	con->showLabel = true;

	return con;
}

CSmoduleConnector *CSmodule::createInput(const char *name, const char *uname, csfloat defaultValue)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::createInput(\"%s\", \"%s\")\n", name,uname);
	#endif
	CSmoduleConnector *inp = createConnector(CS_CONDIR_IN, 0,0, name, uname, defaultValue);
	addConnector(inp);
	updateConnector(inp);
	return inp;
}

CSmoduleConnector *CSmodule::insertInput(int index, const char *name, const char *uname, csfloat defaultValue)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::insertInput(index, \"%s\", \"%s\")\n", index, name,uname);
	#endif
	CSmoduleConnector *inp = createConnector(CS_CONDIR_IN, 0,0, name, uname, defaultValue);
	insertConnector(inp, index);
	updateConnector(inp);
	return inp;
}

CSmoduleConnector *CSmodule::createOutput(const char *name, const char *uname)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::createOutput(\"%s\", \"%s\")\n", name,uname);
	#endif
	CSmoduleConnector *inp = createConnector(CS_CONDIR_OUT, 0,0, name, uname, 0.0);
	addConnector(inp);
	updateConnector(inp);
	return inp;
}


void CSmodule::releaseConnector(CSmoduleConnector *con)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::releaseConnector(0x%p)\n",con);
	#endif
	if (con->name) free(con->name);
	if (con->uname) free(con->uname);
	if (con->valueStr) free(con->valueStr);
	free(con);
}




void CSmodule::registerConnectors()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::registerConnectors() in base class!\n");
	#endif

	arrangeConnectors();
}

void CSmodule::arrangeConnectors()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::arrangeConnectors()\n");
	#endif
	minHeight1 = minHeight;
	int maxh=0;
	int yi=headerHeight+inputStart;
	int yo=headerHeight+outputStart;
	for (int i=0;i<nrCon;i++)
	if ((con[i]->active)&&(con[i]->autoArrange))
	{
		if (con[i]->dir==CS_CONDIR_IN)
		{
			con[i]->x = 0;
			con[i]->y = yi;
			yi+=inputHeight;
		}
		else
		{
			con[i]->x = width;
			con[i]->y = yo;
			yo+=outputHeight;
		}

		maxh = max(maxh, con[i]->y);
	}

	if ((con)&&(nrCon>0)) minHeight1 = max(minHeight, maxh + inputHeight);
	if (canResize)
		height = max(height, minHeight);
	else
		height = minHeight;
	onResize();

	// give the container a hint that we might have changed a connection
	if (parent) parent->getValueUpdateList();
}


int CSmodule::nrInputs()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::nrInputs()\n");
	#endif
	int n=0;
	for (int i=0;i<nrCon;i++)
		if ((con[i]->active)&&(con[i]->dir==CS_CONDIR_IN)) n++;
	return n;
}

int CSmodule::nrOutputs()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::nrOutputs()\n");
	#endif
	int n=0;
	for (int i=0;i<nrCon;i++)
		if ((con[i]->active)&&(con[i]->dir==CS_CONDIR_OUT)) n++;
	return n;
}

int CSmodule::getConnectorIndex(const char *name)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::getConnectorIndex(\"%s\")\n",name);
	#endif
	for (int i=0;i<nrCon;i++) if (strcmp(name, con[i]->name) == 0) return i;
	return -1;
}

int CSmodule::getConnectorIndex(CSmoduleConnector *inp)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::getConnector(0x%p)\n",inp);
	#endif
	for (int i=0;i<nrCon;i++) if (con[i] == inp) return i;
	return -1;
}


void CSmodule::addConnector(CSmoduleConnector *inp)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::addConnector(0x%p)\n",inp);
	#endif
	if (con==0) {
		con = (CSmoduleConnector**) calloc(1,sizeof(int));
		nrCon = 1;
	} else {
		con = (CSmoduleConnector**) realloc(con, (nrCon+1)*sizeof(int));
		nrCon++;
	}
	con[nrCon-1] = inp;
	registerConnectors();
}

void CSmodule::insertConnector(CSmoduleConnector *inp, int index)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::insertConnector(0x%p, %d)\n",inp,index);
	#endif
	bool oldReady=ready;
	ready = false;

	if (con==0) {
		con = (CSmoduleConnector**) calloc(1,sizeof(int));
		con[0] = inp;
		nrCon = 1;
	} else {
		con = (CSmoduleConnector**) realloc(con, (nrCon+1)*sizeof(int));
		index = max(0,min(nrCon, index));
		nrCon++;
		for (int i=nrCon-1;i>index;i--)
			con[i] = con[i-1];
		con[index] = inp;

	}
	registerConnectors();
	ready = oldReady;
}

void CSmodule::deleteConnector(int index)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::deleteConnector(%d)\n",index);
	#endif
	if ((index<0)||(index>=nrCon)) return;

	// disconnect
	if (parent) parent->disconnectConnector(con[index]);

	releaseConnector(con[index]);
	for (int i=index;i<nrCon-1;i++)
		con[i] = con[i+1];
	nrCon--;
	registerConnectors();
}



void CSmodule::updateConnector(CSmoduleConnector *con)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::updateConnector(0x%p)\n", con);
	#endif
	if (con->dir==CS_CONDIR_IN)
		sprintf(con->valueStr, "%g", con->uvalue);
	else
		sprintf(con->valueStr, "%g", con->value);
}

void CSmodule::setConnectorValue(CSmoduleConnector *con, csfloat newValue)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::setConnectorValue(0x%p)\n", con);
	#endif
	con->uvalue = newValue;
	updateConnector(con);
	if (parent) parent->getValueUpdateList();
}

void CSmodule::setConnectorActive(CSmoduleConnector *con, bool act)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::setConnectorActive(0x%p, %d)\n", con, act);
	#endif

	con->active = act;
	if ((!con->active) && (parent))
		parent->disconnectConnector(con);
	arrangeConnectors();
}


int CSmodule::getConnectedConnectors(CSmoduleConnector *co, CSmoduleConnector **conlist)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::getConnectedConnectors(0x%p, 0x%p)\n", co, conlist);
	#endif
	if (!parent) return 0;

	int k=0;
	if (co->dir==CS_CONDIR_OUT)
	for (int i=0;i<parent->nrCon;i++)
	{
		if (parent->con[i]->from == co)
		{
			conlist[k] = parent->con[i]->to;
			k++;
		}
	}
	else
	for (int i=0;i<parent->nrCon;i++)
	{
		if (parent->con[i]->to == co)
		{
			conlist[k] = parent->con[i]->from;
			k++;
		}
	}

	return k;
}



	// -------------------------- timeline -------------------------------------

CStimeline *CSmodule::createTimeline(int length, int skip)
{
	#ifdef CSMOD_DEBUG
	printf("CSmodule::createTimeline(%d, %d)\n",length,update);
	#endif
	CStimeline *t = new CStimeline;
	t->length = length;
	t->data = (csfloat*) calloc(length, sizeof(csfloat));
	t->skip = skip;
	t->skipc = 0;
	t->pos = 0;

	return t;
}

void CSmodule::releaseTimeline(CStimeline *timeline)
{
	#ifdef CSMOD_DEBUG
	printf("CSmodule::releaseTimeline(0x%p)\n",timeline);
	#endif
	if (timeline->data) free(timeline->data);
	free(timeline);
}

void CSmodule::recordTimeline(CStimeline *tl, csfloat value)
{
	// check update skips
	if (tl->skip)
	{
		tl->skipc++;
		if (tl->skipc>tl->skip) tl->skipc=0;
	}

	if (tl->skipc) return;

	// record value in ringbuffer
	tl->data[tl->pos++] = value;
	if (tl->pos>=tl->length) tl->pos = 0;
}





// ------------------------------------- GUI -------------------------------------------

void CSmodule::refresh()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::refresh()\n");
	#endif
	if (parent) parent->refresh();
}

void CSmodule::onResize()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::onResize()\n");
	#endif
	width = max(width, minWidth);
	height = max(height, minHeight1);
}


void CSmodule::setPos(int newx, int newy)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::setPos(%d, %d)\n",newx,newy);
	#endif
	x = newx;
	y = newy;
}

void CSmodule::move(int offx, int offy)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::move(%d, %d)\n",offx,offy);
	#endif
	setPos(x+offx,y+offy);
}

bool CSmodule::isOnHeader(int px, int py)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::isOnHeader(%d, %d)\n",px,py);
	#endif
	return (
		(px>=x) && (px<=x+width) &&
		(py>=y) && (py<=y+headerHeight)
		);
}

int CSmodule::isOnConnector(int px, int py)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::isOnConnctor(%d, %d)\n",px,py);
	#endif
	// first test bounding box around module
	//  (assumes that connectors lay WITHIN modules,
	//   which should always be the case)
	if (!(
		(px>=x-connectorRad) && (px<=x+width+connectorRad) &&
		(py>=y-connectorRad) && (py<=y+height+connectorRad)
		)) return -1;

	// now check connectors each
	for (int i=0;i<nrCon;i++)
	if (con[i]->active)
	{
		if (
			(px>=x+con[i]->x-connectorRad) && (px<=x+con[i]->x+connectorRad) &&
			(py>=y+con[i]->y-connectorRad) && (py<=y+con[i]->y+connectorRad)
			) return i;
	}
	return -1;
}

int CSmodule::isOnValue(int px, int py)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::isOnValue(%d, %d)\n",px,py);
	#endif
	// within module?
	if (!(
		(px>=x) && (px<=x+width) &&
		(py>=y) && (py<=y+height)
		)) return -1;
	// check each input connector value
	for (int i=0;i<nrCon;i++)
	if ((con[i]->active)&&(con[i]->dir==CS_CONDIR_IN)&&(con[i]->showLabel))
	{
		if (
			(px>=x+con[i]->x+5) && (px<=x+con[i]->x+50) &&
			(py>=y+con[i]->y) && (py<=y+con[i]->y+inputHeight)
			) return i;
	}
	return -1;
}

bool CSmodule::isOnModule(int px,int py)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::isOnModule(%d, %d)\n",px,py);
	#endif
	return (
		(px>=x) && (px<=x+width) &&
		(py>=y) && (py<=y+height)
		);
}

bool CSmodule::isOnResize(int px,int py)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::isOnResize(%d, %d)\n",px,py);
	#endif
	return (
		(px>=x+width-8) && (px<=x+width) &&
		(py>=y+height-8) && (py<=y+height)
		);
}

bool CSmodule::isInBox(int x1,int y1, int x2, int y2)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::isInBox(%d, %d, %d, %d)\n",x1,y1,x2,y2);
	#endif

	return (
		(x>=x1)&&(x+width<=x2)&&(y>=y1)&&(y+height<=y2)
		);
}



bool CSmodule::mouseDown(int mx, int my, int mk)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::mouseDown(%d, %d, %d) in base class\n",mx,my,mk);
	#endif
	return false;
}

bool CSmodule::mouseMove(int mx, int my, int mk)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::mouseMove(%d, %d, %d) in base class\n",mx,my,mk);
	#endif
	return false;
}

bool CSmodule::mouseUp(int mx, int my)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::mouseUp(%d, %d) in base class\n",mx,my);
	#endif
	return false;
}





// --------------------------------- external windows --------------------------------

void CSmodule::createWindow()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::createWindow()\n");
	#endif

	if (window) delete window;

	if ((parent)&&(parent->parent)) {
		window = new CSmoduleExtWindow(this);
	}

}

void CSmodule::drawWindow()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::drawWindow() in base class!!\n");
	#endif

	// do nothing here
}

void CSmodule::drawAndSaveWindow(const char *filename)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::drawAndSaveWindow(\"%s\") in base class!!\n", filename);
	#endif

	// do nothing here
}

void CSmodule::onResizeWindow(int newW, int newH)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::onResizeWindow(%d, %d) in base class!!\n",newW,newH);
	#endif

	// do nothing here
}


void CSmodule::storeWindow(const char *filename)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::storeWindow(\"%s\")\n",filename);
	#endif

	if (!window) return;

	FILE *f = fopen(filename, "wb");
	if (!f)
	{
		printf("ERROR SAViNG %s\n", filename);
		return;
	}

	png_structp png_ptr = png_create_write_struct(
		PNG_LIBPNG_VER_STRING,0,0,0);

	png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
		png_destroy_write_struct(&png_ptr,
			(png_infopp)NULL);
		fclose(f);
		return;
    }

    int w = window->w(), h = window->h();

    png_init_io(png_ptr, f);
    png_set_IHDR(
		png_ptr, info_ptr, w, h,
		8,
		PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
		);

	// get window image data
	//window->make_current();
	unsigned char *img = (unsigned char*) calloc(w*h*3, 1);
	fl_read_image(img, 0,0,w,h);

	// and save scanline pointers
	int *scanline = (int*) calloc(h, sizeof(int)), *s = scanline;
	for (int i=0;i<h;i++)
	{
		*s = (int)img + i*w*3;
		s++;
	}

	png_set_rows(png_ptr, info_ptr, (png_byte**)scanline);
	png_write_png(png_ptr, info_ptr, 0, NULL);

	free(scanline);
	free(img);
	fclose(f);
}

void CSmodule::storePNG(const char *filename, unsigned char *img, int w, int h, int spacing)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::storePNG(\"%s\", 0x%p, %d, %d, %d)\n", filename, img, w, h, spacing);
	#endif

	FILE *f = fopen(filename, "wb");
	if (!f)
	{
		printf("ERROR SAViNG %s\n", filename);
		return;
	}

	png_structp png_ptr = png_create_write_struct(
		PNG_LIBPNG_VER_STRING,0,0,0);

	png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
		png_destroy_write_struct(&png_ptr,
			(png_infopp)NULL);
		fclose(f);
		return;
    }

    png_init_io(png_ptr, f);
    png_set_IHDR(
		png_ptr, info_ptr, w, h,
		8,
		(spacing==3)? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
		);

	// save scanline pointers
	int *scanline = (int*) calloc(h, sizeof(int)), *s = scanline;
	for (int i=0;i<h;i++)
	{
		*s = (int)img + i*w*spacing;
		s++;
	}

	png_set_rows(png_ptr, info_ptr, (png_byte**)scanline);
	png_write_png(png_ptr, info_ptr, 0, NULL);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	free(scanline);
	fclose(f);
}


	// --------------------- interaction with container ------------------------

void CSmodule::setSampleRate(int sr)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::setSampleRate(%d)\n", sr);
	#endif
	sampleRate = sr;
	isampleRate = 1.0/max(1,sr);
	normSampleRate = 44100.0/max(1,sr);
}

void CSmodule::onAddModule()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::onAddModule()\n");
	#endif
	// do nothing
}




	// ---------------------- calculation --------------------------------------


void CSmodule::updateInputs()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::updateInputs()\n");
	#endif

	for (int i=0;i<nrCon;i++) if (con[i]->dir==CS_CONDIR_IN)
	{
		con[i]->value = con[i]->uvalue;
	}
}

void CSmodule::updateOutputs()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::updateOutputs()\n");
	#endif

	/*
		TODO!! it's too slow

	// remove special float flags from all outputs
	for (int i=0;i<nrCon;i++)
	if ((con[i]->active)&&(con[i]->dir==CS_CONDIR_OUT))
	{
		CSclipFloat(con[i]->value);
	}
	*/
}

void CSmodule::step()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::step() in base class!!\n");
	#endif

	// do nothing in base class
}


void CSmodule::stepLow()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::stepLow()\n");
	#endif

	// do nothing in base class
}



// -------------------------------------- GFX -------------------------------------------------

void CSmodule::updateColors()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::updateColors()\n");
	#endif

	float b = 1.5, d = 0.66;

	CSmulColor( bodyColor, bodyColorB, b);
	CSmulColor( bodyColor, bodyColorD, d);
	CSmulColorRGB( bodyColor, bodySelColor, 1.2,1.2,1.4);
	CSmulColor( headerColor, headerColorB, b);
	CSmulColor( headerColor, headerColorD, d);
	CSmulColor( headerFocusColor, headerFocusColorB, b);
	CSmulColor( headerFocusColor, headerFocusColorD, d);
	CSmulColorRGB( headerColor, headerSelColor, 1.6,1.6,1.8);
	CSmulColor( connectorColor, connectorZeroColor, 0.7);
}

void CSmodule::printModuleInfo()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodule::printModuleInfo()\n");
	#endif
	printf("module class %s\n", typeid(this).name());
	printf("base/id/user name: \"%s\", \"%s\", \"%s\"\n", bname, name, uname);
	printf("size: %d x %d\n", width, height);
	printf("nr connectors: %d\n", nrCon);
}


void CSmodule::drawConnector(CSmoduleConnector *con, int offx, int offy, float zoom)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::drawConnection(0x%p, %d, %d, %f)\n",con,offx,offy,zoom);
	#endif

	if (!con->active) return;

	int rad=max((float)2, connectorRad*zoom);
	if ((parent))
	{
		// if connector is in focus
		if (parent->selCon==con)
		{
			fl_color(connectorFocusColor);
			rad = max((float)3, connectorFocusRad*zoom);
		} else {
			fl_color( (con->nrCon>0)? connectorColor : connectorZeroColor);
		}
		// if cable drag, see if match
		if (
			((parent->dragCon)&&(parent->dragCon!=con)&&(parent->dragCon->dir!=con->dir)
				&&(parent->dragCon->typ==con->typ))
			||
			((parent->dragBundle)&&(parent->dragBundle->dir==con->dir)
				&&(parent->dragBundle->typ==con->typ)) )
			fl_color(parent->connectionDragColor);
	}

	switch (con->typ)
	{
		case CS_CON_SCOPE:
			fl_rectf(offx+(x+con->x)*zoom-rad, offy+(y+con->y)*zoom-rad, rad<<1, rad<<1);
			break;
		default:
			// need to draw polygon to make circle filled
			fl_begin_polygon();
			fl_circle(offx+(x+con->x)*zoom, offy+(y+con->y)*zoom, rad);
			fl_end_polygon();
			break;
	}

	if (!con->showLabel) return;

	// draw input labels
	fl_font(FL_HELVETICA, 10*zoom);
	if (con->dir==CS_CONDIR_IN)
	{
		bool sel = ((parent)&&(parent->selValue==con)); // in focus
		bool self = sel || ((parent)&&(parent->selCon==con)); // in focus
		bool seled = ((parent)&&(parent->editValue==con)); // editing
			fl_color( (sel)? labelFocusColor : labelColor );
		int
			xo = offx+(x+con->x+1)*zoom+rad,
			yoo = (self)? max(10, fl_height()) : fl_height(),
			yo = offy+(y+con->y)*zoom+(yoo>>2);
		// name
		fl_font(FL_HELVETICA, (self)? max((float)9,11*zoom) : 10*zoom);
		fl_draw(con->uname, xo,yo);
		xo += zoom*3;
		// value
		if (!seled)
		{
			if (!sel)
				fl_font(FL_HELVETICA, fl_size()+1);
			else
				fl_font(FL_HELVETICA_BOLD, fl_size()+3);
			fl_draw(con->valueStr,xo,yo+yoo);
		} else {
			// edit value
			fl_color(labelEditColor);
			fl_font(FL_HELVETICA_BOLD, 15*zoom);
			fl_draw(con->valueStrE,xo,yo+yoo);
		}
	}
	else
	// draw output labels
	{
		bool self = ((parent)&&((parent->selCon==con)||(parent->selValue==con))); // in focus
		fl_font(FL_HELVETICA, (self)? max((float)9,11*zoom) : 10*zoom );
		fl_color( labelColor );
		int
			xo = offx+(x+con->x)*zoom,
			yoo = (self)? max(9, fl_height()) : fl_height(),
			yo = offy+(y+con->y)*zoom+(yoo>>2);
		// name
		fl_draw(con->uname,xo-fl_width(con->uname)-(rad+3),yo);
		xo -= zoom*3;
		// value
		fl_font(FL_HELVETICA, fl_size()+1);
		fl_draw(con->valueStr,xo-fl_width(con->valueStr)-(rad+3),yo+yoo);
	}

}


void CSmodule::drawBody(int offx, int offy, float zoom)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::drawBody(%d, %d, %f)\n",offx,offy,zoom);
	#endif

	#ifdef CSMOD_GLGUI

	#else
	// header
	int tlx = offx+x*zoom;
	int tly = offy+y*zoom;
	int tw = width*zoom;
	int th = headerHeight*zoom;
	bool sele = ( ((parent)&&(parent->selModule==this)) || (isSelected xor isSelectedArea));
	bool headInFocus = ((parent)&&(parent->selHead==this));
	if (sele)
		fl_color( headerSelColor );
	else
		fl_color( (headInFocus)? headerFocusColor : headerColor);
	fl_rectf(tlx, tly, tw, th);
	// shades frame
	fl_line_style(FL_SOLID);
	fl_color( (headInFocus)? headerFocusColorB : headerColorB);
	fl_xyline( tlx, tly, tlx+tw-2 );
	fl_yxline( tlx, tly, tly+th-1 );
	fl_color( (headInFocus)? headerFocusColorD : headerColorD);
	fl_xyline( tlx, tly+th-1, tlx+tw);
	fl_yxline( tlx+tw-1, tly+1, tly+th-1);

	// header caption
	fl_font((headInFocus)? FL_HELVETICA_BOLD : FL_HELVETICA, 14*zoom);
	fl_color( headerFontColor );
	fl_draw( uname, offx+x*zoom+3, offy+(y+13)*zoom );

	// body
	tly = offy+(y+headerHeight)*zoom;
	th = (height-headerHeight)*zoom;
	fl_color( (sele)? bodySelColor : bodyColor );
	fl_rectf(tlx, tly, width*zoom, th);
	// shades
	fl_color( bodyColorB );
	fl_yxline( tlx, tly, tly+th-2 );
	fl_color( bodyColorD );
	fl_xyline( tlx, tly+th-1, tlx+tw );
	fl_yxline( tlx+tw-1, tly+1, tly+th-1 );
	// selection frame
	if (sele)
		fl_frame("UULL", offx+x*zoom-2, offy+y*zoom-2, width*zoom+4, height*zoom+4);

	// resize handle
	if (canResize)
	{
		int r = 7*zoom;
		if ((parent)&&((parent->selResizeModule==this)||(parent->resizeModule==this)))
			fl_color(headerSelColor);
		else
			fl_color(headerColor);
		fl_polygon(tlx+tw,tly+th-r,
					tlx+tw-r,tly+th,
					tlx+tw,tly+th);
	}

	// connection handles
	for (int i=0;i<nrCon;i++)
		drawConnector(con[i], offx,offy,zoom);

	#endif // CSMOD_GLGUI
}


void CSmodule::drawTimeline(CStimeline *tl, int x, int y, int w, int h,
								bool withBackground, int backColor, int offset)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::draw(0x%p, %d, %d, %d, %d)\n",tl, x,y,w,h);
	#endif

	// draw background
	if (withBackground)
	{
		int c = fl_color();
		fl_color(backColor);
		fl_rectf(x,y,w,h);
		fl_color(c);
	}

	// draw data
	float x1=0,x2=0,y1=0,y2=0;
	for (int i=1;i<tl->length;i++)
	{
		int p=tl->pos+i+offset;
		while (p<0) p+=tl->length;
		while (p>=tl->length) p-=tl->length;
		x1 = x + (float)i/tl->length*w,
		y1 = y + (1.0-max(-1.0f,min(1.0f, tl->data[p])))*h/2;
		if (i>1)
			fl_line(x1,y1,x2,y2);
		x2 = x1; y2 = y1;
	}
}

void CSmodule::draw(int offx, int offy, float zoom)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodule::draw(%d, %d, %f)\n",offx,offy,zoom);
	#endif

	// update output connector values
	for (int i=0;i<nrCon;i++)
		if (con[i]->dir==CS_CONDIR_OUT) updateConnector(con[i]);

	drawBody(offx, offy, zoom);
}


