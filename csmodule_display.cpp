#include "math.h"

#include "csmodcontainer.h"
#include "csmodule_display.h"


CSmodule_AbstractDisplay::~CSmodule_AbstractDisplay()
{
	if (img) free(img);
}

void CSmodule_AbstractDisplay::onAddModule()
{
	// check all other display modules in the patch
	if (parentRoot) checkAllServer(parentRoot);
}

void CSmodule_AbstractDisplay::init()
{
	oldBodyColor = bodyColor;
	oldBodySelColor = bodySelColor;

	if (!parent) return;

	// ---- image and window ----
	img = 0;
	setImage(CSMOD_INIT_EXTWIN_WIDTH, CSMOD_INIT_EXTWIN_HEIGHT);

	server = 0;
	server1 = 0;
	requestServerChange = isServer = isClient = false;

	connectToServer("");

	wasScreenUpdate = false;

	// ---- properties ----

	createNameProperty();
	createPropertyInt("w", "buffer width", CSMOD_INIT_EXTWIN_WIDTH, 1, 4096);
	createPropertyInt("h", "buffer height", CSMOD_INIT_EXTWIN_HEIGHT, 1, 4096);
	createPropertyString("serverstr", "bitmap server name", "");
	createPropertyString("clientstr", "connect to bitmap server", "");
}

void CSmodule_AbstractDisplay::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"clientstr")==0)
		connectToServer(prop->svalue);
	else
	if (strcmp(prop->name,"serverstr")==0)
	{
		isServer = strcmp(prop->svalue,"");
		onAddModule(); // re-check all other displays
	}
	else
	if (strcmp(prop->name,"w")==0) setImage(prop->ivalue, h);
	else
	if (strcmp(prop->name,"h")==0) setImage(w,prop->ivalue);
	else CSmodule::propertyCallback(prop);
}

void CSmodule_AbstractDisplay::setWindowCaption()
{
	if (window)
	{
		char wl[512];
		snprintf(wl, 512, "window %dx%d / buffer %dx%d",
			window->w(), window->h(), w,h);
		window->label(wl);
	}
}

void CSmodule_AbstractDisplay::setImage(int w,int h)
{
	setWindowCaption();

	CSwait();
	ready = false;

	this->w = w;
	this->h = h;
	if (img) free(img);
	img = (uchar*) calloc((w*h)<<2, sizeof(uchar));

	setWindowCaption();

	ready = true;
}

CSmodule_AbstractDisplay *CSmodule_AbstractDisplay::findServer(CSmodContainer *container, const char *servername)
{
	for (int i=0;i<container->nr;i++)
	{
		CSmodule_AbstractDisplay *m =
			dynamic_cast<CSmodule_AbstractDisplay*>(container->module[i]);
		if (m)
		{
			int ip = m->getPropertyIndex("serverstr");
			if (ip>=0)
			{
				if (!strcmp(servername, m->property[ip]->svalue))
					// found
					return m;
			}
		}

		// check sub-containers
		CSmodule_Container *mc =
			dynamic_cast<CSmodule_Container*>(container->module[i]);
		if (mc)
		{
			CSmodule_AbstractDisplay *mo =
				findServer(mc->container, servername);
			if (mo) return mo;
		}
	}
	return 0;
}

void CSmodule_AbstractDisplay::connectToServer(const char *serverName)
{
	bool beClient = strcmp("",serverName);

	server1 = 0;
	int cmode=0;
	if ((beClient)&&(parentRoot))
	{
		server1 = findServer(parentRoot, serverName);
		cmode = (server1!=0);
	}

	// make module red if server is not found but requested
	if ((beClient)&&(server1==0))
	{
		CSsetColor(bodyColor, 100,60,60);
		CSsetColor(bodySelColor, 120,70,80);
	}
	else
	{
		bodyColor = oldBodyColor;
		bodySelColor = oldBodySelColor;
	}

	if (window)
	{
		if (cmode==0)
		{
			doDrawWindow = true;
			doCallDrawWindow = false;
		}
		else
		{
			//window->hide(); FLTK only makes difficulties with window focus
			doDrawWindow = false;
			doCallDrawWindow = true;
		}
	}

	isClient = (cmode!=0);
	requestServerChange = (server1!=0);
}


void CSmodule_AbstractDisplay::checkAllServer(CSmodContainer *container)
{
	for (int i=0;i<container->nr;i++)
	{
		CSmodule_AbstractDisplay *m =
			dynamic_cast<CSmodule_AbstractDisplay*>(container->module[i]);
		if (m)
		{
			int ip = m->getPropertyIndex("clientstr");
			if (ip>=0)
				m->connectToServer(m->property[ip]->svalue);
		}
		// check sub-containers
		CSmodule_Container *mc =
			dynamic_cast<CSmodule_Container*>(container->module[i]);
		if ((mc)&&(mc->container))
		{
			checkAllServer(mc->container);
		}
	}
}


void CSmodule_AbstractDisplay::step()
{
	// change the server if nescessary
	if (requestServerChange)
	{
		server = server1;
		server1 = 0;
		requestServerChange = false;
	}

}


void CSmodule_AbstractDisplay::onResizeWindow(int newW, int newH)
{
	setImage(newW, newH);
}


void CSmodule_AbstractDisplay::addBuffer(const uchar *img1, unsigned int w1, unsigned int h1)
{
	int w2 = min(w,w1)<<2;
	int h2 = min(h,h1);
	for (int j=0;j<h2;j++)
	{
		int p1 = (j*w)<<2;
		int p2 = (j*w1)<<2;
		for (int i=0;i<w2;i++)
		{
			img[p1] = min(255, (int)img[p1] + img1[p2] );
			p1++;
			p2++;
		}
	}
}

