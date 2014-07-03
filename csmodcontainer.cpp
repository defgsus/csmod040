/*

   CYMASONICS MODULAR EDITOR

   module container 'n' stuff

   */

#include <algorithm>
#include <math.h>
#include <time.h>
#include <typeinfo>

#include "csmodcontainer.h"
#include "csmod.h"

#include "csmod_installed_modules.h"

#include "csmodule_audioin.h"
#include "csmodule_audioout.h"
#include "csmodule_system.h"
#include "csmodule_midi.h"


using namespace std;


// ----------------------------------- CSmodContainer -----------------------------------------------------------

CSmodContainer::CSmodContainer()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSmodContainer::CSmodContainer()\n");
	#endif
	clear();

}

CSmodContainer::~CSmodContainer()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSmodContainer::~CSmodContainer()\n");
	#endif
	release();
}

void CSmodContainer::clear()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSmodContainer::clear()\n");
	#endif

	parent = 0;
	rootContainer = 0;
	nr = 0;
	module = 0;

	nrCon = 0;
	con = 0;

	containerModule = 0;

	fileVersion = CSMOD_CONTAINER_FILEVERSION;
	readFileVersion = 1;

	offx = offy = 0;
	zoom = zoom_ = 1.0;
	offx_ = offy_ = 0;
	offxs =	offys = 0;

	lmx = lmy = 0;
	lmx1 = lmy1 = 0;
	lmk = 0;
	conx = cony = 0;
	lastTimeDown = time(NULL);
	mouseHasMoved = false;

	selHead = 0;
	dragHead = 0;
	selModule = 0;
	editModule = 0;
	selResizeModule = 0;
	resizeModule = 0;

	selCon = 0;
	dragCon = 0;
	selValue = 0;
	dragValue = 0;
	dragBundle = 0;
	editValue = 0;

	dragView = 0;
	selCable = 0;
	selCableMin = selCableMax = 0.0;
	doCableBehind = false;

	curWidth = curHeight = 0;

	isMultiSelect = selArea = false;
	selAreaX0 = selAreaY0 = selAreaX1 = selAreaX2 = selAreaY1 = selAreaY2 = 0;

	editCursor = editCursorX = 0;

	CSsetColor(backgroundColor, 40,40,40);
	CSsetColor(backgroundGridColor, 50,50,60);
	CSsetColor(backgroundInactiveColor, 50,20,20);
	CSsetColor(connectionColor, 200,200,200);
	CSsetColor(connectionSelColor, 200,240,230);
	CSsetColor(connectionDragColor, 100,255,100);
	CSsetColor(selAreaColor, 150,150,255);

	inputSamples = 0;
	inputSampleSize = inputSamplePos = inputSampleReadPos = 0;

	nrValueUpdateList = 0;
	nrValueUpdateListAlloc = 0;
	valueUpdateList = 0;

	isRendering = false;


	// --- midi stuff ----
#ifdef CSMOD_USE_MIDI
	pu_midi = 0;

	for (int i=0;i<CSMOD_MAX_NOTE;i++)
	{
		noteOn[i] = 0;
	}
	lastNoteMono = 0;
	lastNoteVelMono = 0;
	lastNoteMonoChanged = false;
	for (int i=0;i<CSMOD_MAX_CONTROLLER;i++)
		controllerValue[i] = 0;
	for (int i=0;i<CSMOD_MAX_NRPN_CONTROLLER;i++)
		controller14Value[i] = 0;

    midi_panic();
#endif

	// calculation can begin

	active = ready = true;
}


void CSmodContainer::release()
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGI)
	printf("CSmodContainer::release()\n");
	#endif

	ready = false;

	// delete connections
	if (con)
	{
		while (nrCon>0) deleteConnection(nrCon-1);
		free(con);
	}

	// delete modules
	if (module)
	{
		while (nr>0) deleteModule(nr-1);
		free(module);
	}

	// free buffers
	if (inputSamples) free(inputSamples);
	inputSamples = 0;

	if (valueUpdateList) free(valueUpdateList);
	nrValueUpdateList = 0;
	nrValueUpdateListAlloc = 0;


	// free menus
#ifdef CSMOD_USE_MIDI
	if (pu_midi) delete pu_midi;
#endif
	clear();
}









	//------------------ load/save ---------------------------------------------

void CSmodContainer::storeConnections(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodContainer::storeConnections(0x%p)\n",buf);
	#endif

	fprintf(buf, "connections { \n");

	for (int i=0;i<nrCon;i++)
	{
		// module(from) connector module(to) connector
		fprintf(buf, "\"%s\" \"%s\" \"%s\" \"%s\" \n",
			con[i]->from->parent->name, con[i]->from->name,
			con[i]->to->parent->name, con[i]->to->name);
	}

	fprintf(buf, "} ");
}

void CSmodContainer::store(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodContainer::store(0x%p)\n",buf);
	#endif

	fprintf(buf, "container { ");
	// version
	fprintf(buf, "%d ",fileVersion);
	// viewport
	fprintf(buf, "%d %d %f ", offx_, offy_, zoom_);
	// root?
	if (fileVersion>1) fprintf(buf, "%d ", (this==rootContainer)? 1:0);

	fprintf(buf,"\n");

	// modules
	for (int i=0;i<nr;i++) {
		module[i]->store(buf);
		fprintf(buf, "\n");
	}
	// connections
	storeConnections(buf);

	fprintf(buf, "} \n");
}





void CSmodContainer::restoreConnections(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodContainer::restoreConnections(0x%p)\n",buf);
	#endif

	fscanf(buf, " connections { ");

	bool doit=true;
	while (doit)
	{
		char *str1 = readString(buf);
		if (strcmp(str1, "}")==0) { doit=false; free(str1); continue; }
		char *str2 = readString(buf);
		char *str3 = readString(buf);
		char *str4 = readString(buf);

		connect(str1,str2,str3,str4);

		free(str1);
		free(str2);
		free(str3);
		free(str4);
	}

}


int CSmodContainer::restore(FILE *buf)
{
	#if defined(CSMOD_DEBUGE) || (CSMOD_DEBUGF)
	printf("CSmodContainer::restore(0x%p)\n",buf);
	#endif

	// disable container for a while
	bool oldReady = ready;
	ready = false;

	// TODO!!
	int bpos = ftell(buf);
	//printf("%d\n",bpos);

	fscanf(buf, " container { ");

	int e = fscanf(buf, "%d ",&readFileVersion);
    if (e!=1) CSERROR("container::restore (file version)");
	e = fscanf(buf, "%d %d %f ",&offx_,&offy_,&zoom_);
	if (e!=3) CSERROR("container::restore (viewport)");
	if (readFileVersion>1)
	{
		int r;
		e = fscanf(buf, "%d ",&r);
		if (e!=1) CSERROR("container::restore (root flag)");
		if ((r==0)&&(this==rootContainer))
		{
			printf("is sub-container\n");
			fseek(buf, bpos, SEEK_SET);
			ready = true;
			return 1;
		}
	}

	char *str;
	bool doit = true;

	while (doit)
	{
		// save file position
        fpos_t oldpos;
        fgetpos(buf, &oldpos);

		// see what's next
		str = readString(buf);

		// create the module and load settings
		if (strcmp(str, "module")==0)
		{
			fscanf(buf, " { ");
			char *nam = readString(buf);
			CSmodule *mod;

			// check for special case of container_input/output
			if ((strcmp(nam, "container input")==0))
			{
				char *c=0;
				if (readFileVersion>=4)
					c = readString(buf);
				if (containerModule) mod = containerModule->newContainerCon(CS_CONDIR_IN,c);
					else CSERROR("container::restore (no container module to link)");
			}
			else
			if ((strcmp(nam, "container output")==0))
			{
				char *c=0;
				if (readFileVersion>=4)
					c = readString(buf);
				if (containerModule) mod = containerModule->newContainerCon(CS_CONDIR_OUT,c);
					else CSERROR("container::restore (no container module to link)");
			}
			else
			{
				CSmodule *mod1 = getInstalledModule(nam);
                if (!mod1) CSERROR("container::restore (unknown module) '" << nam << "'");
				mod = addModule(mod1);
			}
			if (nam) free(nam);
            fsetpos(buf, &oldpos);

			// set CSmod parent for container modules
			if (strcmp(mod->bname,"container")==0)
				dynamic_cast<CSmodule_Container*>(mod)->container->parent = parent;

			// read module settings
			mod->restore(buf);
		}

		else
		if (strcmp(str, "connections")==0)
		{
            fsetpos(buf, &oldpos);
			restoreConnections(buf);
		}

		// finally ...
		else
		if (strcmp(str, "}")==0) doit = false;

		free(str);
	}

	// set rootContainer fields
	if (parent) parent->updateSubContainers();

	// set ready state back
	ready = oldReady;

	return 0;
}









	//------------------------- module handling --------------------------------

void CSmodContainer::registerModule(CSmodule *m)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::registerModule(0x%p)\n",m);
	#endif

	// assign this container
	m->parent = this;
	// assign the top container
	m->parentRoot = rootContainer;

	// copy samplerate
	if (parent)
	{
		m->setSampleRate(parent->sampleRate);
	}

	// initialize the module
	m->init();

	// make ID name unique
	// ..delete name so it doesn't appear in getUniqueName()
	if (m->name) free(m->name);	m->name = 0;
	m->name = getUniqueName(m->bname);

	// set nr audio in/out channels
	if (parent)
	{
		if (!strcmp(m->bname,"audio in"))
			dynamic_cast<CSmodule_AudioIn*>(m)->setNrOut(parent->nrChannelsIn);
		else
		if (!strcmp(m->bname,"audio out"))
			dynamic_cast<CSmodule_AudioOut*>(m)->setNrIn(parent->nrChannelsOut);
	}

	// put in place (center of screen)
	if (parent)
	{
		float z = std::max(0.1f, zoom);
		m->setPos(
			(parent->w()-m->width)/2/z-offx_,
			(parent->h()-m->height)/2/z-offy_
			);
	} else
	{
		m->setPos(-offx_,-offy_);
	}
	// avoid collisions
	repositionModule(m);

	m->onAddModule();

	// show the window, if any
	if ((m->window)&&(m->doDrawWindow)) m->window->show();

	m->ready = true;

	getValueUpdateList();
}

CSmodule *CSmodContainer::addModule(CSmodule *m)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::addModule(0x%p)\n",m);
	#endif

	bool oldReady = ready;
	ready = false;

	if (!module) {
        module = (CSmodule**) calloc(1,sizeof(void*));
		nr = 1;
	} else {
        module = (CSmodule**) realloc(module, (nr+1)*sizeof(void*));
		nr++;
	}

	module[nr-1] = m->newOne();
	registerModule(module[nr-1]);

	// select this module
	unselect();
	selModule = module[nr-1];
	ready = oldReady;
	return module[nr-1];
}

void CSmodContainer::repositionModule(CSmodule *mod)
{
	// reposition if same pos as another module
	bool rep = true;
	while (rep)
	{
		rep = false;
		for (int i=0;(i<nr)&&(!rep);i++)
			if ( (module[i]!=mod)
				&& (module[i]->x==mod->x)
				&& (module[i]->y==mod->y) )
				{ mod->move(10,10); rep = true; }
	}
}


char *CSmodContainer::getUniqueName(const char *name)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::getUniqueName(\"%s\")\n",name);
	#endif
	char *buf = copyString(name);
	int count = 1;
	while (getModuleIndex(buf)!=-1)
	{
		free(buf);
		buf = addString(name, int2char(count));
		count++;
	}
	return buf;
}

int CSmodContainer::getModuleIndex(const char *name)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::getModule(\"%s\")\n",name);
	#endif
	if (!module) return -1;
	for (int i=0;i<nr;i++)
	if ( (module[i]->name) &&
		 (strcmp(name, module[i]->name) == 0) ) return i;

	return -1;
}


int CSmodContainer::getModuleIndex(CSmodule *mod)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::getModule(0x%p)\n",mod);
	#endif
	if (!module) return -1;
	for (int i=0;i<nr;i++)
	if ( module[i]==mod ) return i;

	return -1;
}

void CSmodContainer::deleteModule(int index)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::deleteModule(%d)\n",index);
	#endif

	if ((!module)||(index<0)||(index>=nr)) return;

	// exclude from calculation
	module[index]->ready = false;

	// remove all references to this module
	if (selHead==module[index]) selHead = 0;
	if (dragHead==module[index]) dragHead = 0;
	if (selModule==module[index]) selModule = 0;
	if (containerModule==module[index]) containerModule = 0;
	selCon = 0;
	dragCon = 0;
	selValue = 0;
	dragValue = 0;
	dragBundle = 0;
	editValue = 0;
	selCable = 0;

	// disconnect module
	disconnectModule(module[index]);

	// wipe out..
	bool oldReady = ready;
	ready = false;
	delete module[index];
	// ..and remove from list
	for (int i=index;i<nr-1;i++)
		module[i] = module[i+1];
	nr--;
	ready = oldReady;
}


CSmodule *CSmodContainer::duplicateModule(CSmodule *mod)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::duplicateModule(0x%p)\n",mod);
	#endif

	// add a new instance of mod
	CSmodule *m = addModule(mod);
	// copy settings from mod
	m->copyFrom(mod);
	m->move(10,10);
	selModule = m;
	return m;
}



void CSmodContainer::deleteAll()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::deleteAll()\n");
	#endif
	// delete modules
	while (nr>0) deleteModule(nr-1);
	// clear all references
	selHead = 0;
	dragHead = 0;
	selModule = 0;
	editModule = 0;
	selCon = 0;
	dragCon = 0;
	selValue = 0;
	dragValue = 0;
	dragBundle = 0;
	editValue = 0;
	dragView = 0;
	selCable = 0;
	// reset view
	offx = 0;
	offy = 0;
	zoom = 1.0;
	offx_ = 0;
	offy_ = 0;
	offxs = 0;
	offys = 0;
	zoom_ = 1.0;
}


void CSmodContainer::getExtend(int *x1, int *y1, int *x2, int *y2)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::getExtend(0x%p, 0x%p, 0x%p, 0x%p)\n", x1,y1,x2,y2);
	#endif

	if (nr<1)
	{
		*x1 = *y1 = *x2 = *y2 = 0;
		return;
	}

	*x1 = module[0]->x;
	*y1 = module[0]->y;
	*x2 = *x1 + module[0]->width;
	*y2 = *y1 + module[0]->height;

	for (int i=1;i<nr;i++)
	{
		*x1 = min(*x1, module[i]->x);
		*y1 = min(*y1, module[i]->y);
		*x2 = max(*x2, module[i]->x + module[i]->width);
		*y2 = max(*y2, module[i]->y + module[i]->height);
	}
}


void CSmodContainer::getValueUpdateList()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::getValueUpdateList()\n");
	#endif

	// get nr
	int nrv=0;
	for (int i=0;i<nr;i++)
	for (int j=0;j<module[i]->nrCon;j++)
	if ((module[i]->con[j]->dir==CS_CONDIR_IN)
		&&(module[i]->con[j]->active))
		nrv++;

	// alloc list
	if (valueUpdateList==0)
        valueUpdateList = (csfloat**) calloc(nrv*2, sizeof(void*));
	else
	if (nrv*2>=nrValueUpdateListAlloc)
	{
		nrValueUpdateListAlloc = nrv*2;
		valueUpdateList = (csfloat**)
            realloc(valueUpdateList,nrValueUpdateListAlloc*sizeof(void*));
	}

	// store
	int k=0;
	for (int i=0;i<nr;i++)
	for (int j=0;j<module[i]->nrCon;j++)
	if ((module[i]->con[j]->dir==CS_CONDIR_IN)
		&&(module[i]->con[j]->active))
	{
		valueUpdateList[k++] = &module[i]->con[j]->value;
		valueUpdateList[k++] = &module[i]->con[j]->uvalue;
	}

	nrValueUpdateList = nrv;
}




// --------------- module list handling ------------------------------------

CSmoduleList *CSmodContainer::getNewModuleList()
{
	CSmoduleList *m = new CSmoduleList;
	m->nrAlloc = 256;
	m->nr = 0;
    m->m = (CSmodule**) calloc(m->nrAlloc, sizeof(void*));
	return m;
}

void CSmodContainer::freeModuleList(CSmoduleList *list)
{
	free(list->m);
	free(list);
}

void CSmodContainer::addModuleList(CSmoduleList *list, CSmodule *m)
{
	// make space
	if (list->nr>=list->nrAlloc)
	{
		list->nrAlloc += 256;
        list->m = (CSmodule**) realloc(list->m, sizeof(void*) * list->nrAlloc);
	}

	list->m[list->nr] = m;
	list->nr++;
}

void CSmodContainer::addModuleList(CSmoduleList *list, CSmoduleList *listadd)
{
	for (int i=0;i<listadd->nr;i++)
		addModuleList(list, listadd->m[i]);
}








	// ------------------------- connection handling ---------------------------

CSconnection *CSmodContainer::createConnection(CSmoduleConnector *output, CSmoduleConnector *input)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::createConnection(0x%p, 0x%p)\n", output, input);
	#endif

	// find duplicates
	for (int i=0;i<nrCon;i++)
		if ((con[i]->from==output)&&(con[i]->to==input)) return 0;

	CSconnection *con = new CSconnection;
	con->from = output;
	con->to = input;
	// keep track of nr of connections
	con->from->nrCon++;
	con->to->nrCon++;
	con->ready = false;
	return con;
}

void CSmodContainer::releaseConnection(CSconnection *conn)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::releaseConnection(0x%p)\n", conn);
	#endif
	// currently no dynamic data in CSconnection, will there ever be??...
	free(conn);
}

int CSmodContainer::getConnectionIndex(CSconnection *conn)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::getConnectionIndex(0x%p)\n", conn);
	#endif
	if (!con) return -1;
	for (int i=0;i<nrCon;i++) if (con[i]==conn) return i;
	return -1;
}

void CSmodContainer::addConnection(CSconnection *conn)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::addConnection(0x%p)\n", conn);
	#endif
	bool oldReady = ready;
	ready = false;
	if (!con)
	{
        con = (CSconnection**) calloc(1,sizeof(void*));
		nrCon = 1;
	} else {
		nrCon++;
        con = (CSconnection**) realloc(con, nrCon*sizeof(void*));
	}
	con[nrCon-1] = conn;
	getValueUpdateList();
	ready = oldReady;
	conn->ready = true;
}

void CSmodContainer::deleteConnection(int index)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::deleteConnection(%d)\n", index);
	#endif
	if ((!con)||(index<0)||(index>=nrCon)) return;
	con[index]->ready = false;
	con[index]->from->nrCon--;
	con[index]->to->nrCon--;
	releaseConnection(con[index]);
	for (int i=index;i<nrCon-1;i++)
		con[i] = con[i+1];
	nrCon--;
	getValueUpdateList();
}

CSconnection *CSmodContainer::connect(CSmoduleConnector *con1, CSmoduleConnector *con2)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::connect(0x%p, 0x%p)\n", con1, con2);
	#endif
	if (con1->typ!=con2->typ) return 0;
	if (con1->dir==con2->dir) return 0;

	CSconnection *con;
	if (con1->dir==CS_CONDIR_OUT)
		con = createConnection(con1, con2);
	else
		con = createConnection(con2, con1);

	if (con) addConnection(con);
	getValueUpdateList();
	return con;
}

CSconnection *CSmodContainer::connect(const char *mod1, const char *con1, const char *mod2, const char *con2)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::connect(\"%s\", \"%s\", \"%s\", \"%s\")\n",
		mod1, con1, mod2, con2);
	#endif

	int mi1 = getModuleIndex(mod1);
	if (mi1==-1) return 0;
	int mi2 = getModuleIndex(mod2);
	if (mi2==-1) return 0;
	int ci1 = module[mi1]->getConnectorIndex(con1);
	if (ci1==-1) return 0;
	int ci2 = module[mi2]->getConnectorIndex(con2);
	if (ci2==-1) return 0;

	getValueUpdateList();
	return connect(module[mi1]->con[ci1], module[mi2]->con[ci2]);
}

void CSmodContainer::reconnect(CSconnection *conn, CSmoduleConnector *con1, CSmoduleConnector *con2)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::reconnect(0x%p, 0x%p, 0x%p)\n", conn, con1, con2);
	#endif

	// check duplicates
	if ((conn->from==con1)&&(conn->to==con2)) return; // already connected
	for (int i=0;i<nrCon;i++)
		if ((con[i]->from==con1)&&(con[i]->to==con2)) return;

	// reconnect
	conn->ready = false;
	conn->from->nrCon--;
	conn->to->nrCon--;
	conn->from = con1;
	conn->to = con2;
	conn->from->nrCon++;
	conn->to->nrCon++;
	conn->ready = true;
	getValueUpdateList();
}

void CSmodContainer::disconnectModule(CSmodule *mod)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::disconnectModule(0x%p)\n", mod);
	#endif
	for (int i=0;i<nrCon;)
	{
		if ( (con[i]->from->parent==mod) || (con[i]->to->parent==mod) )
			deleteConnection(i);
		else
			i++;
	}
	getValueUpdateList();
}

void CSmodContainer::disconnectConnector(CSmoduleConnector *conn)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::disconnectConnector(0x%p)\n", conn);
	#endif
	for (int i=0;i<nrCon;)
	{
		if ( (con[i]->from==conn) || (con[i]->to==conn) )
			deleteConnection(i);
		else
			i++;
	}
	getValueUpdateList();
}


int CSmodContainer::isOnConnection(int px, int py)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::isOnConnection(%d, %d)\n", px,py);
	#endif
	if (!con) return -1;

	// check if modules are in front
	if (doCableBehind)
	{
		for (int i=0;i<nr;i++)
			if (module[i]->isOnModule(px,py)) return -1;
	}

	int pxs = px*px;
	int pys = py*py;
	int rad = 5;
	int rads = rad*rad;

	for (int i=nrCon-1;i>=0;i--)
	{
		int x1 = (con[i]->from->x+con[i]->from->parent->x);
		int y1 = (con[i]->from->y+con[i]->from->parent->y);
		int x2 = (con[i]->to->x+con[i]->to->parent->x);
		int y2 = (con[i]->to->y+con[i]->to->parent->y);

		// following is a complicated and math intensive
		// circle/line intersection test copied from my small renderer.
		// ...there might be a faster way? (dirty rectangles or something..)

		// at least, we can check for bounding box before testing the intersection
		/* TODO!!
		if (!(
			( (px>=x1)&&(px<=x2)&&(py>=y1)&&(py<=y2) ) ||
			( (px>=x2)&&(px<=x1)&&(py>=y2)&&(py<=y1) )
			)) continue;
			*/

		int dx = x2-x1;
		int dy = y2-y1;
		float a = dx*dx+dy*dy;
		// cable has no length (?)
		if (a==0) continue;

		float b = 2*( dx*(x1-px) + dy*(y1-py) );
		float c = pxs + pys + x1*x1 + y1*y1 - 2*( px*x1 + py*y1 ) - rads;

		float f = b*b - 4*a*c;
		if (f<=0) continue;

		f = sqrt(f);
		a = a*2;
		float u1 = (-b-f)/a;
		float u2 = (-b+f)/a;
		float u = u1;
		if ((u2>=0)&&((u1<0)||(u1>u2))) u = u2;
		if ((u<0)||(u>1)) continue;

		// we are on
		return i;
	}

	return -1;
}











//-------------------------- value edit ------------------------------------

void CSmodContainer::nextSelValue(int dir)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::nextSelValue(%d)\n",dir);
	#endif
	// if no current value, try to select one
	if (!selValue)
	{
		CSmodule *m=0;
		if (selHead) m = selHead;
		if (selModule) m = selModule;
		if ((!m)&&(nr>0)) m = module[0];

		if (m->nrCon>0) selValue = m->con[0];
			else return;
	}

	int x=selValue->parent->x+selValue->x;
	int y=selValue->parent->y+selValue->y;

	float mi = 100000.0;
	CSmoduleConnector *theOne = 0;
	for (int i=0;i<nr;i++)
	for (int j=0;j<module[i]->nrCon;j++)
	if ((module[i]->con[j]->active)&&
		(module[i]->con[j]->dir==CS_CONDIR_IN)&&(module[i]->con[j]!=selValue)) {
		// check out each connector
		int x1=module[i]->x+module[i]->con[j]->x;
		int y1=module[i]->y+module[i]->con[j]->y;
		if (
			( (dir==0)&&(x1<x) ) ||
			( (dir==1)&&(y1<y) ) ||
			( (dir==2)&&(x1>x) ) ||
			( (dir==3)&&(y1>y) ) )
		{
			// get distance
			float di = fabs(x1-x)+fabs(y1-y);
			// is smallest distance?
			if (di<mi)
			{
				mi = di;
				theOne = module[i]->con[j];
			}
		}
	}
	if (theOne) {
		selValue = theOne;
		// also select module
		selModule = theOne->parent;
	}
}

void CSmodContainer::startEdit(CSmoduleConnector *conn)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::startEdit(0x%p)\n",conn);
	#endif

	editValue = conn;
	editCursor = 0;
	editCursorX = 0;
	conn->parent->updateConnector(conn);
	updateEditValue();
}

void CSmodContainer::updateEditValue()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::updateEditValue()\n");
	#endif
	if (!editValue) return;

	editValue->uvalue = atof(editValue->valueStr);

	int l = strlen(editValue->valueStr);
	editCursor = max(0,min(l, editCursor));

	// build a string with a cursor char
	if (editValue->valueStrE) free(editValue->valueStrE);
	editValue->valueStrE = (char*) calloc(l+2,1);
	strncpy(editValue->valueStrE, editValue->valueStr, editCursor);
	char *p = editValue->valueStrE;
	p += editCursor;
	*p = '|';
	char *p1 = editValue->valueStr;
	p++;
	p1 += editCursor;
	if (l-editCursor>0)
		strncpy(p, p1, l-editCursor);
}


void CSmodContainer::editInsertChar(char c)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::editInsertChar(\"%s\")\n", c);
	#endif
	int l = strlen(editValue->valueStr);

	char *p = (char*) calloc(l+2,1);
	char *o = p;

	strncpy(p, editValue->valueStr, editCursor);

	p += editCursor;
	*p = c;
	p++;
	char *p1 = editValue->valueStr;
	p1 += editCursor;
	if (l-editCursor>0)
		strncpy(p, p1, l-editCursor);
	free(editValue->valueStr);
	editValue->valueStr = o;
}

void CSmodContainer::editDeleteChar()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::editDeleteChar()\n");
	#endif
	int l = strlen(editValue->valueStr);

	char *p = (char*) calloc(l,1);
	char *o = p;

	strncpy(p, editValue->valueStr, editCursor);

	p += editCursor;
	char *p1 = editValue->valueStr;
	p1 += editCursor+1;
	if (l-editCursor>1)
		strncpy(p, p1, l-editCursor-1);
	free(editValue->valueStr);
	editValue->valueStr = o;
}








	//------------------------------- G U I ------------------------------------

void CSmodContainer::refresh()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::refresh()\n");
	#endif
	if (parent) parent->refresh();
}

void CSmodContainer::fadeView(float fade)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::fadeView(%f)\n",fade);
	#endif
	zoom_ += fade*(zoom-zoom_);
	/*
	offx_ += fade*(offx-offx_);
	offy_ += fade*(offy-offy_);
	*/
	offx_ += fade*offxs;
	offy_ += fade*offys;
	offxs *= (1.0-fade);
	offys *= (1.0-fade);
}

void CSmodContainer::setViewAll()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::setViewAll()\n");
	#endif
	int x1,y1,x2,y2,w,h;
	getExtend(&x1,&y1,&x2,&y2);
	x1 -= 5;
	y1 -= 5;
	x2 += 5;
	y2 += 5;
	w = max(1, x2-x1);
	h = max(1, y2-y1);
	zoom = 1.0;
	if (parent)
	{
		zoom = min( (float)parent->w() / w, (float)parent->h() / h);
	}
	offxs = zoom*(-x1 - offx_);
	offys = zoom*(-y1 - offy_);
}

bool CSmodContainer::mouseDown(int mx, int my, int mk)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::mouseDown(%d, %d, %d)\n", mx,my,mk);
	#endif

	if (CSpopup_onMouseDown(mx,my,mk)) return true;

	bool alt = (Fl::event_state()&FL_ALT);
	bool shift = (Fl::event_state()&FL_SHIFT);

	mouseHasMoved = false;

	// get seconds between mouse clicks
	time_t theTime = time(NULL);
	time_t timePassed = theTime-lastTimeDown;
	lastTimeDown = theTime;

	// original mouse pos
	int omx=mx;
	int omy=my;

	// set view port
	mx = (mx-offx_)/max(0.001f, zoom_);
	my = (my-offy_)/max(0.001f, zoom_);

	lmx = mx;
	lmy = my;
	lmk = mk;

	// delete cable
	if ((mk==FL_RIGHT_MOUSE)&&(selCable))
	{
		deleteConnection(getConnectionIndex(selCable));
		selCable=0;
		mouseHasMoved = true; /* <- hack, actually just to
									avoid popup of module window */
		return true;
	}

	// start header/module dragging
	if ((mk==FL_LEFT_MOUSE)&&(selHead))
	{
		dragHead = selHead;

		// also select this module

		if (alt) {
			multiSelect(selHead);
			return true;
		}
		// when clicking on a multiselected module without alt
		// then keep the multiselection
		if (!selHead->isSelected) unselect();
		// select single module
		selModule = selHead;

		// on dblclick
		if ((timePassed==0)&&(parent))
		{
			// if dblclicked container-module then switch container
			if (dynamic_cast<CSmodule_Container*>(selHead)!=NULL)
				parent->assignContainer(
					dynamic_cast<CSmodule_Container*>(selHead)->container);
			else
			// if no container then open properties
			parent->propertyWindow->assignModule(selHead);

			if (selHead->doDrawWindow && selHead->window)
				selHead->window->show();

		}
		return true;
	}

	// start connection dragging
	if ((mk==FL_LEFT_MOUSE)&&(selCon))
	{
		conx = mx;
		cony = my;
		dragCon = selCon;
		selCable = 0;
		return true;
	}

	// start bundle dragging
	if ((mk==FL_RIGHT_MOUSE)&&(selCon)&&(selCon->nrCon>0))
	{
		conx = mx;
		cony = my;
		dragCon = 0;
		dragBundle = selCon;
		selCable = 0;
		mouseHasMoved = true; /* <- hack, actually just to
									avoid popup of module window */
		return true;
	}

	// start dragging (or editing) value
	if ((mk==FL_LEFT_MOUSE)&&(selValue))
	{
		if (timePassed>0)
		{
			dragValue = selValue;
			lmy = omy;
			return true;
		}
		else
			// start editing value
			startEdit(selValue);

	}

	// start resizing module
	if ((mk==FL_LEFT_MOUSE)&&(selResizeModule))
	{
		resizeModule = selResizeModule;
		return true;
	}


	// pass mousedown event to module?
	for (int i=0;i<nr;i++)
	{
		if (module[i]->isOnModule(mx,my))
			if (module[i]->mouseDown(mx,my,mk))
			{
				editModule = module[i];
				mouseHasMoved = true; // shut up moduleselectwindow
				return true;
			}
	}


	// leftclick in space
	if (mk==FL_LEFT_MOUSE)
	{
		// clear area select flag
		for (int i=0;i<nr;i++) module[i]->isSelectedArea = false;

		// on dblclick in free space try to pop container
		if ((timePassed==0)&&(parent)&&(!selValue))
		{
			parent->popContainer();
			return true;
		}

		// start drag view offset / or select area

		// select area
		if (shift)
		{
			if (!alt) unselect();

			selArea = true;
			selAreaX0 = selAreaX1 = selAreaX2 = mx;
			selAreaY0 = selAreaY1 = selAreaY2 = my;
			return true;
		}

		// store non-offset mouse pos
		lmx = omx;
		lmy = omy;
		dragView = 1;
		// click in space to deselect module
		unselect();
		return true;
	}

	// start zoom view
	if ((mk==FL_RIGHT_MOUSE)&&(!selHead)&&(!selCon)&&(!selValue))
	{
		// store non-offset mouse pos
		lmx = omx;
		lmy = omy;
//		lmy1 = omy;
		lmx1 = mx;
		lmy1 = my;
		dragView = 2;
		offxo = offx_;
		offyo = offy_;
		zoomo = zoom_;
		return true;
	}


	return false;
}


/** move the view offset when dragging something outside the window */
#define CSmoveTheView() \
	if (omx<=0) offxs += 20; else \
	if ((parent)&&(omx>=parent->w()-1)) offxs -= 20; \
	if (omy<=0) offys += 20; else \
	if ((parent)&&(omy>=parent->h()-1)) offys -= 20; \

/** let nothing be in focus */
#define CSunfocusAll() \
	selValue = 0; selCon = 0; selHead = 0; selCable = 0;

/** remove all references */
#define CSdeselectAll() \
	CSunfocusAll(); \
	editValue = 0; dragBundle = 0; dragCon = 0; \
	dragHead = 0; selModule = 0; dragValue = 0; \
	resizeModule = 0; selResizeHandle = 0; \
	editModule = 0; selArea = false;


bool CSmodContainer::mouseMove(int mx, int my, int mk)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::mouseMove(%d, %d, %d)\n", mx,my,mk);
	#endif

	if (CSpopup_onMouseMove(mx,my)) return true;

	mouseHasMoved = true;
	//bool alt = (Fl::event_state()&FL_ALT);
	//bool shift = (Fl::event_state()&FL_SHIFT);

	// store original pos
	int omx = mx;
	int omy = my;

	// set view port
	mx = (mx-offx_)/max(0.001f, zoom_);
	my = (my-offy_)/max(0.001f, zoom_);

	// pass mouse move right trough when edit focus on a module
	if (editModule)
		return editModule->mouseMove(mx,my,mk);



	bool
		alt = (Fl::event_state()&FL_ALT),
		ctrl = (Fl::event_state()&FL_CTRL),
		testCable = true,
		testHead = true,
		testValue = true,
		hasDragged = false;

	// select area
	if (selArea)
	{
		if (mx>selAreaX0) {
			selAreaX1 = selAreaX0;
			selAreaX2 = mx;
		} else {
			selAreaX1 = mx;
			selAreaX2 = selAreaX0;
		}
		if (my>selAreaY0) {
			selAreaY1 = selAreaY0;
			selAreaY2 = my;
		} else {
			selAreaY1 = my;
			selAreaY2 = selAreaY0;
		}
		multiSelectArea();
		CSmoveTheView();
		return true;
	} else

	// drag view offset
	if (dragView==1)
	{
		offxs += omx-lmx;
		offys += omy-lmy;
		lmx = omx; lmy = omy;
		return true;
	} else

	// zoom view
	if (dragView==2)
	{
		float oldzoom=zoom, zf = 0.01*(lmy-omy);
		zoom = max(0.1f,min(5.0f, zoomo + zf ));
		// adjust offset
		if (zoom!=oldzoom)
		{
			offxs = (offxo - lmx1 * zf - offx_);
			offys = (offyo - lmy1 * zf - offy_);
		}
		return true;
	}

	// drag module
	if (dragHead)
	{
		m_move(dragHead, mx-lmx,my-lmy);
		lmx = mx; lmy = my;
		CSmoveTheView();
		return true;
	}

	// drag cable or bundles
	if (dragCon||dragBundle)
	{
		conx = mx;
		cony = my;
		// move view when dragging out of view
		CSmoveTheView();
		// we still have to check for new connector focus...
		hasDragged = true;
		testCable = false;
		testHead = false;
		testValue = false;
	}

	// drag value
	if (dragValue)
	{
		float scale = 10;
		if (alt) scale = 1000; else
		if (ctrl) scale = 1.0/50;
		csfloat v= dragValue->uvalue + (csfloat)(lmy-omy)/scale;
		// remove rounding error
		dragValue->uvalue =
			(csfloat)((int)(fabs(v)*scale+0.5)/scale);
		if (v<0.0) dragValue->uvalue = -dragValue->uvalue;
		dragValue->parent->setConnectorValue(dragValue, dragValue->uvalue);
		lmy = omy;
		return true;
	}

	// resize module
	if (resizeModule)
	{
		resizeModule->width = max(resizeModule->minWidth,
			resizeModule->width + mx-lmx
			);
		resizeModule->height = max(resizeModule->minHeight,
			resizeModule->height + my-lmy
			);
		lmx = mx;
		lmy = my;
		resizeModule->arrangeConnectors();
		CSmoveTheView();
		return true;
	}


	// ---- tests for something particular in focus ----

	// test if connector is in focus
	CSmoduleConnector *oldSelCon = selCon;
	for (int i=0;i<nr;i++)
	{
		int c= module[i]->isOnConnector(mx,my);
		if (c!=-1)
		{
			CSunfocusAll();
			selCon = module[i]->con[c];
			// test, when connecting if input/output match
			if (dragCon) {
				if ((dragCon->dir==selCon->dir)) selCon=0; else
				if ((dragCon->typ!=selCon->typ)) selCon=0;
			} else
			if (dragBundle) {
				if ((dragBundle->dir!=selCon->dir)) selCon=0; else
				if ((dragBundle->typ!=selCon->typ)) selCon=0;
			}
			return true;
		}
	}
	selCon = 0;
	if ( (selCon != oldSelCon) ) return true;

	// test if cable is in focus
	if (!doCableBehind)
	if (testCable)
	{
		CSconnection *oldSelCable = selCable;
		int c = isOnConnection(mx,my);
		if (c!=-1)
		{
			CSunfocusAll();
			selCable = con[c];
			selCableMin = selCableMax = con[c]->from->value;
			lmx = omx; lmy = omy;
			return true;
		}
		selCable = 0;
		selCableMin = 0;
		selCableMax = 0;
		if (selCable != oldSelCable) return true;
	}

	// test if header is in focus
	if (testHead)
	{
		CSmodule *oldSelHead = selHead;
		for (int i=nr-1;i>=0;i--)
			if (module[i]->isOnHeader(mx,my))
			{
				CSunfocusAll();
				selHead = module[i];
				return true;
			}
		selHead = 0;
		if (selHead != oldSelHead) return true;
	}

	// test if a value is in focus
	if (testValue)
	{
		CSmoduleConnector *oldSelValue = selValue;
		for (int i=nr-1;i>=0;i--)
		{
			int c=module[i]->isOnValue(mx,my);
			if (c!=-1)
			{
				CSunfocusAll();
				selValue = module[i]->con[c];
				return true;
			}
		}
		if (oldSelValue != selValue) return true;
	}
	selValue = 0;


	// test if resize handle is in focus
	{
		CSmodule *oldSelResize = selResizeModule;
		for (int i=nr-1;i>=0;i--)
		{
			if (module[i]->isOnResize(mx,my))
			{
				selResizeModule = module[i];
				return true;
			}
		}
		selResizeModule = 0;
		resizeModule = 0;
		if (oldSelResize!=selResizeModule) return true;
	}


	// pass mouse move events to modules
	for (int i=0;i<nr;i++)
	{
		if (module[i]->isOnModule(mx,my))
			if (module[i]->mouseMove(mx,my,mk))
			{
				mouseHasMoved = true; // shut up moduleselectwindow
				return true;
			}
	}

	// test again if cable is in focus
	if (doCableBehind)
	if (testCable)
	{
		CSconnection *oldSelCable = selCable;
		int c = isOnConnection(mx,my);
		if (c!=-1)
		{
			CSunfocusAll();
			selCable = con[c];
			selCableMin = selCableMax = con[c]->from->value;
			lmx = omx; lmy = omy;
			return true;
		}
		selCable = 0;
		selCableMin = 0;
		selCableMax = 0;
		if (selCable != oldSelCable) return true;
	}


	// nothing done?
	return hasDragged;
}


bool CSmodContainer::mouseUp(int mx, int my)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::mouseUp(%d, %d)\n", mx,my);
	#endif

	if (CSpopup_onMouseUp(mx,my)) return true;

	bool alt = (Fl::event_state()&FL_ALT);
	//bool shift = (Fl::event_state()&FL_SHIFT);


	// pass mouse up trough when edit focus is on a module
	if (editModule)
	{
		bool r = editModule->mouseUp(mx,my);
		editModule = 0;
		return r;
	}

	// set area select
	if (selArea)
	{
		for (int i=0;i<nr;i++)
		if (module[i]->isSelectedArea)
		{
			module[i]->isSelected = module[i]->isSelected xor module[i]->isSelectedArea;
			module[i]->isSelectedArea = false;
			if (!isMultiSelect)
			{
				isMultiSelect = true;
				selModule = module[i];
			}
		}
	}

	// test for cable connection
	if ((dragCon)&&(selCon)) connect(dragCon, selCon);

	// test for cable bundle drag
	if ((dragBundle)&&(selCon))
	{
		//printf("drag bundle: %d, %s\n", dragBundle->dir, dragBundle->name);
		// move all cables
		if (dragBundle->dir==CS_CONDIR_IN)
		{
			for (int i=0;i<nrCon;i++)
				if (dragBundle==con[i]->to)
				{
					if (!alt)
						reconnect(con[i], con[i]->from, selCon);
					else
						connect(con[i]->from, selCon);
				}
		}
		else
		if (dragBundle->dir==CS_CONDIR_OUT)
		{
			for (int i=0;i<nrCon;i++)
				if (dragBundle==con[i]->from)
				{
					if (!alt)
						reconnect(con[i], selCon, con[i]->to);
					else
						connect(selCon, con[i]->to);
				}
		}
	}

	// right-click in space = show module window
	if ((lmk==FL_RIGHT_MOUSE)
		&&(!mouseHasMoved)
		&&(parent))
		parent->moduleWindow->show();

	lmk = 0;
	dragHead = 0;
	dragCon = 0;
	dragValue = 0;
	dragBundle = 0;
	resizeModule = 0;

	dragView = 0;
	selArea = false;

	mouseHasMoved = false;

	// there might have been a change so return true
	return true;
}



bool CSmodContainer::keyDown(int key)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::keyDown(%d)\n", key);
	#endif

	if (CSpopup_onKeyDown(key)) return true;

	bool wantKey = true;
	bool ctrl = ((Fl::event_state()&FL_CTRL) || (Fl::event_state()&FL_COMMAND));
	bool alt = (Fl::event_state()&FL_ALT);
	//bool shift = (Fl::event_state()&FL_SHIFT);

	// value edit keys
	if (editValue) {
		// move around / edit keys
		switch (key)
		{
			case FL_Left: if (editCursor>0) editCursor--; break;
			case FL_Right: if (editCursor<(int)strlen(editValue->valueStr)) editCursor++; break;
			case FL_Enter:
			case FL_Escape: updateEditValue(); editValue = 0; return true; break;
			case FL_Delete: editDeleteChar(); break;
			case FL_BackSpace: if (editCursor>0) { editCursor--; editDeleteChar(); } break;
			case FL_Home: editCursor = 0; break;
			case FL_End: editCursor = strlen(editValue->valueStr); break;
			case FL_Up: updateEditValue(); nextSelValue(1); startEdit(selValue); break;
			case FL_Down: updateEditValue(); nextSelValue(3); startEdit(selValue); break;
			default: wantKey = false;
		}
		// insert chars
		if (
			((key>='0')&&(key<='9')) ||
			(key=='.')||(key=='-')
			) { editInsertChar((char)key); editCursor++; wantKey = true; }
		if ((key==',')) { editInsertChar('.'); editCursor++; wantKey = true; }

		if (wantKey)
		{
			updateEditValue();
			return true;
		}
	}

#ifdef CSMOD_USE_MIDI
	// midi keyboard
	if ((!alt)&&(!ctrl))
	{
		switch (key)
		{
			case 'y': rootContainer->midi_noteOn(48); break;
			case 's': rootContainer->midi_noteOn(49); break;
			case 'x': rootContainer->midi_noteOn(50); break;
			case 'd': rootContainer->midi_noteOn(51); break;
			case 'c': rootContainer->midi_noteOn(52); break;
			case 'v': rootContainer->midi_noteOn(53); break;
			case 'g': rootContainer->midi_noteOn(54); break;
			case 'b': rootContainer->midi_noteOn(55); break;
			case 'h': rootContainer->midi_noteOn(56); break;
			case 'n': rootContainer->midi_noteOn(57); break;
			case 'j': rootContainer->midi_noteOn(58); break;
			case 'm': rootContainer->midi_noteOn(59); break;

			case 'q': rootContainer->midi_noteOn(60); break;
			case '2': rootContainer->midi_noteOn(61); break;
			case 'w': rootContainer->midi_noteOn(62); break;
			case '3': rootContainer->midi_noteOn(63); break;
			case 'e': rootContainer->midi_noteOn(64); break;
			case 'r': rootContainer->midi_noteOn(65); break;
			case '5': rootContainer->midi_noteOn(66); break;
			case 't': rootContainer->midi_noteOn(67); break;
			case '6': rootContainer->midi_noteOn(68); break;
			case 'z': rootContainer->midi_noteOn(69); break;
			case '7': rootContainer->midi_noteOn(70); break;
			case 'u': rootContainer->midi_noteOn(71); break;
			case 'i': rootContainer->midi_noteOn(72); break;
			case '9': rootContainer->midi_noteOn(73); break;
			case 'o': rootContainer->midi_noteOn(74); break;
			case '0': rootContainer->midi_noteOn(75); break;
			case 'p': rootContainer->midi_noteOn(76); break;
			default: wantKey = false;
		}
		if (wantKey) return true;
		wantKey = true;
	}
#endif

	// general keys
	switch (key)
	{
		// move selValue around
		case FL_Left: nextSelValue(0); break;
		case FL_Right: nextSelValue(2); break;
		// move selValue (or swap container input)
		case FL_Up:
			if (ctrl) // swap container input
			{
				CSmodule_Container *m = dynamic_cast<CSmodule_Container*>(selModule);
				if ((m)&&(selValue))
					selValue = m->con[m->swapContainerInput(m->getConnectorIndex(selValue),-1)];
				break;
			}
			nextSelValue(1);
			break;
		case FL_Down:
			if (ctrl) // swap container input
			{
				CSmodule_Container *m = dynamic_cast<CSmodule_Container*>(selModule);
				if ((m)&&(selValue))
					selValue = m->con[m->swapContainerInput(m->getConnectorIndex(selValue),1)];
				break;
			}
			nextSelValue(3);
			break;
		case FL_Enter: if (selValue) startEdit(selValue); break;
		// property window
		case 'p': if (!alt) break;
			if (selModule) parent->propertyWindow->assignModule(selModule); break;
		// container input/output
		case 'i': if (!alt) break;
			if (containerModule) containerModule->newContainerCon(CS_CONDIR_IN);
			break;
		case 'o': if (!alt) break;
			if (containerModule) containerModule->newContainerCon(CS_CONDIR_OUT);
			break;
		// swap cables behind
		case 'b': if (!alt) break;
			doCableBehind = !doCableBehind;
			break;
		// contain modules
		case 'c': if (!ctrl) break;
			if (selModule) m_contain(selModule);
			break;
		// select/view ALL
		case 'a':
			if (alt) { setViewAll(); break; }
			if (!ctrl) break;
			if (!nr) break;
			selModule = module[0];
			isMultiSelect = true;
			for (int i=0;i<nr;i++) module[i]->isSelected = true;
			break;
		// insert module
		case FL_Insert:
			if (parent) parent->moduleWindow->show();
			break;
		// delete module
		case FL_Delete: if (!ctrl) break;
			if (selModule) m_delete(selModule);
			break;
		// duplicate a module
		case 'd': if (!ctrl) break;
			if (selModule) m_duplicate(selModule);
			break;

		// messure time
		case 'm': if (!ctrl) break;
			getCalculationTime();
			break;

		// close on ALT+F4
		case FL_F+4: if (!alt) break;
			parent->hide(); break;

		// else
		default: wantKey = false;
	}

	return wantKey;
}











//---------------------- multi-select handling -----------------------------


void CSmodContainer::multiSelect(CSmodule *m)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::multiSelect(0x%p)\n",m);
	#endif

	// check for de-multiselect module
	if ((isMultiSelect)&&(m->isSelected))
	{
		m->isSelected = false;
		// see if multiselected modules left
        int k=0;
		for (int i=0;i<nr;i++) if (module[i]->isSelected) k++;
		if (!k) isMultiSelect = false;
		selModule = 0;
		return;
	}

	isMultiSelect = true;
	m->isSelected = true;
	if (selModule) selModule->isSelected = true;
	selModule = m;
}

void CSmodContainer::multiSelectArea()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::multiSelectArea()\n");
	#endif

	// clear area select
	for (int i=0;i<nr;i++) module[i]->isSelectedArea = false;

	// test module/area overlap
	for (int i=0;i<nr;i++)
	{
		if (CS_BOX_IN_BOX(
			selAreaX1, selAreaY1, selAreaX2, selAreaY2,
			module[i]->x, module[i]->y,
			module[i]->x+module[i]->width, module[i]->y+module[i]->height
			))
			{
				module[i]->isSelectedArea = true;
			}
	}

}

void CSmodContainer::unselect()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::unselect()\n");
	#endif

	for (int i=0;i<nr;i++)
	{
		module[i]->isSelected = false;
		module[i]->isSelectedArea = false;
	}

	isMultiSelect = false;
	selModule = 0;
}


void CSmodContainer::m_move(CSmodule *m, int ofx, int ofy)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::m_move(0x%p, %d, %d)\n", m, ofx,ofy);
	#endif

	if (!isMultiSelect) { m->move(ofx,ofy); return; }

	for (int i=0;i<nr;i++) if (module[i]->isSelected)
		module[i]->move(ofx,ofy);
}


void CSmodContainer::m_delete(CSmodule *m)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::m_delete(0x%p)\n", m);
	#endif

	if (!isMultiSelect) {
		int i = getModuleIndex(m);
		if (i>=0) deleteModule(i);
		return;
	}

	for (int i=0;i<nr;)
		if (module[i]->isSelected)
			deleteModule(i);
		else i++;
}


void CSmodContainer::m_duplicate(CSmodule *m)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::m_duplicate(0x%p)\n", m);
	#endif

	if (!isMultiSelect)
	{
		duplicateModule(m);
		return;
	}

	// collect all modules
    CSmodule **mo = (CSmodule**) calloc(nr*2, sizeof(void*));

	int nrd = 0;
	for (int i=0;i<nr;i++) if (module[i]->isSelected)
	{
		mo[nrd*2] = module[i];
		nrd++;
	}

	// collect connections
    CSconnection **co = (CSconnection **) calloc(nrCon, sizeof(void*));
	int nrc = 0;

	for (int i=0;i<nrCon;i++)
	{
		if ( (con[i]->from->parent->isSelected) && (con[i]->to->parent->isSelected) )
		{
			co[nrc] = con[i];
			nrc++;
		}
	}

	// duplicate modules
	for (int i=0;i<nrd;i++)
		mo[i*2+1] = duplicateModule(mo[i*2]);


	// connect duplicates
	for (int i=0;i<nrc;i++)
	{
		// find module from/to in mo[]
		int m1=-1, m2=-1;
		for (int j=0;(j<nrd)&&((m1==-1)||(m2==-1));j++)
		{
			if (co[i]->from->parent == mo[j*2]) m1 = j*2+1;
			if (co[i]->to->parent == mo[j*2]) m2 = j*2+1;
		}

		if ((m1!=-1)&&(m2!=-1))
		{
			connect(mo[m1]->name, co[i]->from->name,
					mo[m2]->name, co[i]->to->name );
		}
	}

	// select
	unselect();
	isMultiSelect = true;
	for (int i=0;i<nrd;i++)
		mo[i*2+1]->isSelected = true;

	free(mo);
	free(co);
}



CSmodContainer *CSmodContainer::m_contain(CSmodule *m)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::m_contain(0x%p)\n", m);
	#endif

	// collect all modules
    CSmodule **mo = (CSmodule**) calloc(nr*2, sizeof(void*));
	int nrd = 0;

	if (!isMultiSelect)
	{
		isMultiSelect = true;
		m->isSelected = true;
	}

	for (int i=0;i<nr;i++) if (module[i]->isSelected)
	{
		// exclude container inputs/outputs !!! TODO !!!
		if ((dynamic_cast<CSmodule_ContainerInput*>(module[i])) ||
			(dynamic_cast<CSmodule_ContainerOutput*>(module[i])))
			{
				module[i]->isSelected = false;
				continue;
			}
		mo[nrd*2] = module[i];
		nrd++;
	}

	// all selected module are now in mo[0,2,4,6... > nrd*2]


	// collect connections
	CSconnection
        **co = (CSconnection **) calloc(nrCon, sizeof(void*)),
        **coi = (CSconnection **) calloc(nrCon, sizeof(void*)),
        **coo = (CSconnection **) calloc(nrCon, sizeof(void*));
	int nrc = 0, nrco = 0, nrci = 0;

	for (int i=0;i<nrCon;i++)
	{
		// inside connections
		if ( (con[i]->from->parent->isSelected) && (con[i]->to->parent->isSelected) )
		{
			co[nrc] = con[i];
			nrc++;
		}
		else
		// input connections
		if ( (con[i]->to->parent->isSelected) )
		{
			coi[nrci] = con[i];
			nrci++;
		}
		else
		// output connections
		if ( (con[i]->from->parent->isSelected) )
		{
			coo[nrco] = con[i];
			nrco++;
		}
	}

	// create new container
	CSmodule_Container *cm = dynamic_cast<CSmodule_Container*>(
		addModule((CSmodule_Container*) getInstalledModule("container")) );
	CSmodContainer *c = (cm)->container;


	// copy modules
	for (int i=0;i<nrd;i++)
	{
		mo[i*2+1] = c->addModule(mo[i*2]);
		mo[i*2+1]->copyFrom(mo[i*2]);
		// keep id name
		if (mo[i*2+1]->name) free(mo[i*2+1]->name);
		mo[i*2+1]->name = copyString(mo[i*2]->name);
	}

	// create inputs/outputs

	int x1,y1,x2,y2;
	c->getExtend(&x1,&y1,&x2,&y2);

    CSmodule_ContainerInput **conin = (CSmodule_ContainerInput**) calloc(nrci, sizeof(void*));
	for (int i=0;i<nrci;i++)
	{
		conin[i] = (CSmodule_ContainerInput*)
			cm->newContainerCon(CS_CONDIR_IN, /*coi[i]->to->name*/0, coi[i]->to->uname);
		conin[i]->setPos(x1-conin[i]->width-24, y1 + i*(conin[i]->height+10));
	}

    CSmodule_ContainerOutput **conout = (CSmodule_ContainerOutput**) calloc(nrco, sizeof(void*));
	for (int i=0;i<nrco;i++)
	{
		conout[i] = (CSmodule_ContainerOutput*)
			cm->newContainerCon(CS_CONDIR_OUT, /*coo[i]->from->name*/0,coo[i]->from->uname);
		conout[i]->setPos(x2+24, y1 + i*(conout[i]->height+10));
	}

	// copy connections
	for (int i=0;i<nrc;i++)
	{
		c->connect(co[i]->from->parent->name, co[i]->from->name,
				   co[i]->to->parent->name, co[i]->to->name);
	}

	for (int i=0;i<nrci;i++)
	{
		c->connect(conin[i]->name, conin[i]->con[0]->name,
				   coi[i]->to->parent->name, coi[i]->to->name);
	}

	for (int i=0;i<nrco;i++)
	{
		c->connect(coo[i]->from->parent->name, coo[i]->from->name,
					conout[i]->name, conout[i]->con[0]->name);
	}

	free(conin);
	free(conout);
	free(mo);
	free(co);

	// position view port
	c->setViewAll();

	return c;
}







	//-------------------------------- midi ------------------------------------

#ifdef CSMOD_USE_MIDI

void CSmodContainer::onMidiIn(DWORD data1, DWORD /*data2*/)
{
	if ((parent)&&(!parent->running)) return;

	onMidiInChangeScr = true;

	unsigned char
		cmd = data1 & 0xf0,
		//chan = data1 & 0x0f,
		d1 = (data1>>8) & 0xff,
		d2 = (data1>>16) & 0xff;

	// CONTROL CHANGE
	if (cmd == 0xb0)
	{
		midi_controlChange(d1,d2);

		/* NRPN sequence:
			CC 99 = controller number MSB
			CC 98 = controller number LSB
			CC 6 = value MSB
			CC 38 = value LSB
			// 2^14 = 16384 possible values
		*/
		switch (nrpnSequence)
		{
			case -1: if (d1==99) { nrpnSequence++; nrpnNumber = d2<<7; } break;
			case 0: if (d1==98) { nrpnSequence++; nrpnNumber |= d2; } else nrpnSequence = -1; break;
			case 1: if (d1==6) { nrpnSequence++; nrpnValue = d2<<7; } else nrpnSequence = -1; break;
			case 2:
				if (d1==38) { nrpnValue |= d2; midi_control14Change(nrpnNumber, nrpnValue); }
				nrpnSequence = -1;
			break;
		}
	}
	else
	// NOTE ON/OFF
	if (cmd == 0x90) midi_noteOn(d1,d2);


	//if (cmd == 0x90) printf("key on\n");
	//printf("%p %p\n",data1,data2);
}


void CSmodContainer::midi_panic()
{
	// reset midi stuff
	for (int i=0;i<CSMOD_MAX_NOTE;i++)
	{
		noteOn[i] = 0;
	}
	nrpnSequence = -1;
	lastNoteMono = 0;
	lastNoteVelMono = 0;
	lastCCNumber = 0;
	onMidiInChangeScr = false;
	onMidiControlChangeScr = false;
	onMidiNoteChangeScr = false;
}

void CSmodContainer::midi_noteOn(int noteNr, int velocity)
{
	onMidiNoteChangeScr = true;

	if ((noteNr<0)||(noteNr>127)) return;

	if (velocity>0)
	{
		noteOn[noteNr] = velocity;

		lastNoteMono = noteNr;
		lastNoteVelMono = (csfloat)velocity/127;
		lastNoteMonoChanged = true;

	}
	else
	{
		noteOn[noteNr] = 0;
		lastNoteVelMono = 0.0;
	}

	// pass to noteInPoly modules
	for (int i=0;i<nr;i++)
	{
		CSmodule_NoteInPoly *m = dynamic_cast<CSmodule_NoteInPoly*>(module[i]);
		if (m) m->noteOn(noteNr, velocity);

		// pass to subcontainers
		CSmodule_Container *cm = dynamic_cast<CSmodule_Container*>(module[i]);
		if (cm) cm->container->midi_noteOn(noteNr, velocity);
	}
}


void CSmodContainer::midi_controlChange(int controller, int value)
{
	onMidiControlChangeScr = true;
	lastCCNumber = controller;

	controllerValue[controller] = (csfloat)value / 127;

	// pass to controllerIn modules
	for (int i=0;i<nr;i++)
	{
		CSmodule_ControllerIn *m = dynamic_cast<CSmodule_ControllerIn*>(module[i]);
		if (m) m->controlChange(controller, value);

		// pass to subcontainers
		CSmodule_Container *cm = dynamic_cast<CSmodule_Container*>(module[i]);
		if (cm) cm->container->midi_controlChange(controller, value);
	}
}


void CSmodContainer::midi_control14Change(int controller, int value)
{
	onMidiControlChangeScr = true;
	lastCCNumber = controller;

	controller14Value[controller] = (csfloat)value / 16383;

	// pass to controllerIn modules
	for (int i=0;i<nr;i++)
	{
		CSmodule_ControllerIn *m = dynamic_cast<CSmodule_ControllerIn*>(module[i]);
		if (m) m->control14Change(controller, value);

		// pass to subcontainers
		CSmodule_Container *cm = dynamic_cast<CSmodule_Container*>(module[i]);
		if (cm) cm->container->midi_control14Change(controller, value);
	}

}


#endif // #ifdef CSMOD_USE_MIDI


	//---------------------------- calculation ---------------------------------

void CSmodContainer::setSampleRate(int newSampleRate)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::setSampleRate(%d)\n", newSampleRate);
	#endif
	for (int i=0;i<nr;i++)
	{
		module[i]->setSampleRate(newSampleRate);

		// sub containers
		CSmodule_Container *mc =
			dynamic_cast<CSmodule_Container*>(module[i]);
		if (mc) mc->container->setSampleRate(newSampleRate);
	}
}


void CSmodContainer::start()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::start()\n");
	#endif

	setStart(this);
	stepLow();
}

void CSmodContainer::setStart(CSmodContainer *c)
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::setStart()\n");
	#endif

	// check all modules
	for (int i=0;i<c->nr;i++)
	{
		CSmodule_StartGate *m =
			dynamic_cast<CSmodule_StartGate*>(c->module[i]);
		if (m) m->start();

		// check sub containers
		CSmodule_Container *mc =
			dynamic_cast<CSmodule_Container*>(c->module[i]);
		if (mc) setStart(mc->container);
	}

#ifdef CSMOD_USE_MIDI
    midi_panic();
#endif
}



// 392 - 400
// 156
// 5 ins/outs - 625
//		490 w/o updateInputs
//		520 w/o updateOutputs
//		520 w/o step
//      600 w/o connections
//-----
// 7000
//   5000 w/o connections
//   4000 w/o step
//   3000 w/o updateInputs
//-----
// 4500
//   600 steps only
//	 4100 w/o connections
//	 1700 w/o updateOutputs
//
void CSmodContainer::step()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::step()\n");
	#endif

	// clear output sample buffer
	if (this==rootContainer)
	{
		float *os = outputSample;
		for (int i=0;i<CSMOD_MAX_CHAN;i++)
			*os++ = 0.0;
	}


	// reset module inputs
	// !! faster update since 2011/02/28 ;)
	int k=0;
	for (int i=0;i<nrValueUpdateList;i++)
	{
		*valueUpdateList[k] = *valueUpdateList[k+1];
		k+=2;
	}

	// transport connections
	for (int i=0;i<nrCon;i++) if (ready && con[i]->ready)
	{
		con[i]->to->value +=
			con[i]->from->value;
	}

	// step modules
	for (int i=0;i<nr;i++) if (ready && module[i]->ready)
	{
		//printf("step module : %s\n",module[i]->name);
		module[i]->isStep = true;
		module[i]->step();
		module[i]->isStep = false;

		module[i]->updateOutputs();
	}

#ifdef CSMOD_USE_MIDI
	// reset midi stuff
	lastNoteMonoChanged = false;
	/*
	lastNoteGate = -1;
	for (int i=0;i<CSMOD_MAX_NOTE;i++)
		noteGate[i] = -1;
		*/
#endif
}


void CSmodContainer::stepLow()
{
	#ifdef CSMOD_DEBUGE
	printf("CSmodContainer::stepLow()\n");
	#endif

	// update module inputs
	for (int i=0;i<nr;i++) if (ready && module[i]->ready)
	{
		module[i]->stepLow();
	}
}




void CSmodContainer::getCalculationTime()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::getCalculationTime()\n");
	#endif

	if ((parent)&&(parent->running))
		{ printf("must stop audio process to messure time.\n"); return; }

	printf("MESSURED TIME IN CONTAINER ");
	if (containerModule)
		printf("\"%s\"\n", containerModule->uname);
	else
		printf(">ROOT<\n");

	int nrSteps = 96000;

	printf("%24s%17s%17s\n","module","ms per sample","samples per sec");

	DWORD t1, t2;

	float smps=0;

	for (int i=0;i<nr;i++)
	{
		// for each module

		t1 = timeGetTime();

		// do couple of steps
		module[i]->stepLow();
		for (int k=0;k<nrSteps;k++)
		{
			module[i]->step();
		}

		t2 = timeGetTime();

		float mps = (float)max((DWORD)1, t2-t1) / nrSteps;
		float sps = 1000.0 / mps;

		printf("%24s%17f%17ld\n",
			module[i]->uname,
			mps,
			(unsigned long int)sps);

		smps += mps;
	}

	printf("--%22s%17f%17ld\n","sum:",smps,(unsigned long int)(1000.0 / smps));

	// check sub containers
	for (int i=0;i<nr;i++)
	{
		CSmodule_Container *mc =
			dynamic_cast<CSmodule_Container*>(module[i]);
		if (mc) mc->container->getCalculationTime();
	}

}

















//------------------------------ G F X ----------------------------------------------------

void CSmodContainer::drawConnection(CSconnection *con)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::drawConnection(0x%p)\n", con);
	#endif

	float th = max((float)1, 2.0f*zoom_);
	fl_color(connectionColor);

	if (selCable==con) {
		// cable in focus
		th = max((float)2,3.0f*zoom_); fl_color(connectionSelColor);
		}
	fl_line_style(FL_SOLID, th);

	bool doDrawNormal = true;
	if ((dragBundle==con->from))
	{
		// draw dispositioned connection that is currently dragged
		fl_color(connectionDragColor);
		fl_line_style(FL_DASH, max(1, (int)(2*zoom_)));
		fl_line(
		conx*zoom_+offx_,cony*zoom_+offy_,
		(con->to->x+con->to->parent->x)*zoom_+offx_, (con->to->y+con->to->parent->y)*zoom_+offy_ );
		doDrawNormal = (Fl::event_state()&FL_ALT);
	} else
	if ((dragBundle==con->to))
	{
		// draw dispositioned connection
		fl_color(connectionDragColor);
		fl_line_style(FL_DASH, max(1, (int)(2*zoom_)));
		fl_line(
		(con->from->x+con->from->parent->x)*zoom_+offx_, (con->from->y+con->from->parent->y)*zoom_+offy_,
		conx*zoom_+offx_,cony*zoom_+offy_ );
		doDrawNormal = (Fl::event_state()&FL_ALT);
	}

	if (doDrawNormal)
	{
		// draw normal connection
		fl_line(
		(con->from->x+con->from->parent->x)*zoom_+offx_, (con->from->y+con->from->parent->y)*zoom_+offy_,
		(con->to->x+con->to->parent->x)*zoom_+offx_, (con->to->y+con->to->parent->y)*zoom_+offy_ );
	}
	fl_line_style(FL_SOLID);

}

void CSmodContainer::draw(int width, int height)
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::draw()\n");
	#endif

	// remember the window size
	curWidth = width;
	curHeight = height;

	#ifdef CSMOD_GLGUI

		// fill background
		gl_color(backgroundColor);
		gl_rectf(0,0,width,height);


		// draw modules
		for (int i=0;i<nr;i++)
			module[i]->draw(offx_,offy_,zoom_);

	#else

		// fill background
		fl_color((active)? backgroundColor:backgroundInactiveColor);
		fl_rectf(0,0,width,height);

        // draw some grid
		fl_color(backgroundGridColor);
		float gs = 50*zoom_;
		float k = MODULO((offx_+width/2.0)/2.0, gs);
		for (float i=k;i<width;i+=gs) fl_line(i,0,i-height/32,height);
		k = MODULO((offy_+height/2.0)/2.0, gs);
		for (float i=k;i<height;i+=gs) fl_line(0,i,width,i+width/32);

		if (!doCableBehind)
		{
			// draw modules
			for (int i=0;i<nr;i++)
				module[i]->draw(offx_,offy_,zoom_);

			// draw connections
			for (int i=0;i<nrCon;i++) drawConnection(con[i]);
		}
		else
		// or other way around
		{
			// draw connections
			for (int i=0;i<nrCon;i++) drawConnection(con[i]);
			// draw modules
			for (int i=0;i<nr;i++)
				module[i]->draw(offx_,offy_,zoom_);
		}

		// show dragged connection
		if (dragCon)
		{
			fl_color(connectionDragColor);
			fl_line_style(FL_DASH, max(1, (int)(2*zoom_)));
			fl_line(
				(dragCon->x+dragCon->parent->x)*zoom_+offx_, (dragCon->y+dragCon->parent->y)*zoom_+offy_,
				conx*zoom_+offx_, cony*zoom_+offy_ );
			fl_line_style(FL_SOLID);
		}

		// show selection area
		if (selArea)
		{
			fl_color(selAreaColor);
			fl_line_style(FL_DASH, 2);
			fl_xyline( offx_+zoom_*selAreaX1, offy_+zoom_*selAreaY1, offx_+zoom_*selAreaX2 );
			fl_xyline( offx_+zoom_*selAreaX1, offy_+zoom_*selAreaY2, offx_+zoom_*selAreaX2 );
			fl_yxline( offx_+zoom_*selAreaX1, offy_+zoom_*selAreaY1, offy_+zoom_*selAreaY2 );
			fl_yxline( offx_+zoom_*selAreaX2, offy_+zoom_*selAreaY1, offy_+zoom_*selAreaY2 );
			fl_line_style(FL_SOLID);
		}

		// cable's value
		if (selCable)
		{
			// get min/max value
			selCableMin = min(selCableMin, selCable->from->value);
			selCableMax = max(selCableMax, selCable->from->value);

			fl_font(FL_HELVETICA, 12);
			char v[256];
			if (selCableMin==selCableMax)
				snprintf(v,255,"%g",selCable->from->value);
			else
				snprintf(v,255,"%g (%g - %g)",
					selCable->from->value, selCableMin, selCableMax);
			fl_color(FL_BLACK);
			fl_draw(v, lmx+1, lmy+1);
			fl_color(FL_CYAN);
			fl_draw(v, lmx, lmy);
			// print connected modules if out of screen
			int y = lmy-12;

			if ( ! CS_POINT_IN_BOX(
					offx_+zoom_*(selCable->to->x+selCable->to->parent->x),
					offy_+zoom_*(selCable->to->y+selCable->to->parent->y),
					0,0,width,height) )
			{
				snprintf(v,255, "->%s.%s", selCable->to->parent->uname, selCable->to->uname);
				fl_color(FL_BLACK);
				fl_draw(v, lmx+1, y+1);
				fl_color(FL_CYAN);
				fl_draw(v, lmx, y);
				y -= 12;
			}
			if ( ! CS_POINT_IN_BOX(
					offx_+zoom_*(selCable->from->x+selCable->from->parent->x),
					offy_+zoom_*(selCable->from->y+selCable->from->parent->y),
					0,0,width,height) )
			{
				snprintf(v,255, "%s.%s->", selCable->from->parent->uname, selCable->from->uname);
				fl_color(FL_BLACK);
				fl_draw(v, lmx+1, y+1);
				fl_color(FL_CYAN);
				fl_draw(v, lmx, y+1);
			}

		}
    #ifdef CSMOD_USE_MIDI
		// midi in
		if (onMidiInChangeScr)
		{
			fl_color(FL_DARK_GREEN);
			fl_rectf(width-12,2,10,10);
			onMidiInChangeScr = false;
		}
		if (onMidiControlChangeScr)
		{
			fl_color(FL_DARK_GREEN);
			fl_rectf(width-32,2,18,10);

			fl_color(FL_WHITE);
			fl_font(FL_HELVETICA, 9);
			char ccnum[32];	sprintf(ccnum,"%d",lastCCNumber);
			fl_draw(ccnum,width-30,10);
			onMidiControlChangeScr = false;
		}
		if (onMidiNoteChangeScr)
		{
			fl_color(FL_DARK_CYAN);
			fl_rectf(width-44,2,10,10);
			onMidiNoteChangeScr = false;
		}

		/*
		fl_color(FL_WHITE);
		char od[100]; sprintf(od, "%d %d",offx_,offy_);
		fl_draw(od,20,20);
		*/
    #endif // #ifdef CSMOD_USE_MIDI

    #endif // CSMOD_GLGUI

	CSpopup_draw();
}


void CSmodContainer::drawExternalWindows()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::drawExternalWindows()\n");
	#endif

	if (!active) return;

	// update the external windows
	if ((parent)&&(parent->showExternalWindows))
	for (int i=0;i<nr;i++)
	{
		if ((module[i]->window)&&(module[i]->doDrawWindow))
		{
			//if (!module[i]->window->visible())
				//module[i]->window->show();
			//else
			if (module[i]->window->visible())
				module[i]->window->redraw();
		}
		if (module[i]->doCallDrawWindow)
			module[i]->drawWindow();

		// call sub-containers
		CSmodule_Container *mc = dynamic_cast<CSmodule_Container*>(module[i]);
		if (mc) mc->container->drawExternalWindows();
	}
}



CSmoduleList *CSmodContainer::getExternalWindowModules()
{
	#ifdef CSMOD_DEBUGG
	printf("CSmodContainer::getExternalWindowModules()\n");
	#endif

	CSmoduleList *mods = getNewModuleList();

	// search the external windows
	for (int i=0;i<this->nr;i++)
	{
		if (module[i]->window)
		{
			addModuleList(mods, module[i]);
		}

		// call sub-containers
		CSmodule_Container *mc = dynamic_cast<CSmodule_Container*>(module[i]);
		if (mc)
			{
			CSmoduleList *m = mc->container->getExternalWindowModules();
			if (m->nr>0) addModuleList(mods, m);

			freeModuleList(m);
			}
	}

	return mods;
}
