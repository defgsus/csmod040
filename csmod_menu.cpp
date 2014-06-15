#include <stdio.h>
#include "csmod_menu.h"

CSmodPopUp * CSactivePopUp = 0;

CSmodPopUp::CSmodPopUp()
{
	nrItem = 0;
	nrItemAlloc = CSMOD_POPUP_MIN_ITEM;
	item = (CSmodPopUpItem*) calloc(nrItemAlloc, sizeof(CSmodPopUpItem));
	lh = 20;
}



CSmodPopUp::~CSmodPopUp()
{
	for (int i=0;i<nrItem;i++)
	{
		free(item[i].caption);
	}
	if (item) free(item);
}


void CSmodPopUp::clear()
{
	nrItem = 0;
}

void CSmodPopUp::addItem(const char *caption, void *ptr, void *userData1, void *userData2)
{
	if (nrItem>=nrItemAlloc)
	{
		nrItemAlloc += CSMOD_POPUP_MIN_ITEM;
		item = (CSmodPopUpItem*) realloc(item, nrItemAlloc * sizeof(CSmodPopUpItem));
	}

	item[nrItem].caption = (char*) calloc(strlen(caption)+1, 1);
	strcpy(item[nrItem].caption, caption);

	item[nrItem].ptr = ptr;
	item[nrItem].userData1 = userData1;
	item[nrItem].userData2 = userData2;

	nrItem++;
}

void CSmodPopUp::doPopUp(int x,int y)
{
	ox = x;
	oy = y;
	visible = 1;

	w = 100;
	h = nrItem*lh;

	CSactivePopUp = this;
}

void CSmodPopUp::hide()
{
	visible = 0;
	if (CSactivePopUp==this) CSactivePopUp = 0;
}

bool CSmodPopUp::onMouseDown(int x, int y, int mk)
{
	if ((x<ox)||(x>ox+w)||(y<oy)||(y>oy+h)) return false;

	itemFocus = (y-oy) / lh;
	if (itemFocus<0) itemFocus = 0;
	if (itemFocus>=nrItem) itemFocus = nrItem-1;

	doClick(itemFocus);
	return true;
}

bool CSmodPopUp::onMouseMove(int x, int y)
{
	if ((x<ox)||(x>ox+w)||(y<oy)||(y>oy+h)) return false;

	itemFocus = (y-oy) / lh;
	if (itemFocus<0) itemFocus = 0;
	if (itemFocus>=nrItem) itemFocus = nrItem-1;

	return true;
}

bool CSmodPopUp::onMouseUp(int x, int y)
{
	return false;
}

bool CSmodPopUp::onKeyDown(int k)
{
	bool wantKey=true;
	switch (k)
	{
		case FL_Up:
			itemFocus--;
			if (itemFocus<0) itemFocus = 0;
			break;
		case FL_Down:
			itemFocus++;
			if (itemFocus>=nrItem) itemFocus = nrItem-1;
			break;
		case FL_Enter:
			if ((itemFocus>=0) && (itemFocus<nrItem))
				doClick(itemFocus);
			break;
		case FL_Escape:
			visible = false;
			break;
		default:
			wantKey = false;
	}
	return wantKey;
}


void CSmodPopUp::doClick(int itemFocus)
{
	hide();

	if (item[itemFocus].userData1)
	{
		// callback function
		void (*fp)(void*,void*);
		fp = (void (*)(void*,void*))(item[itemFocus].ptr);
		fp(item[itemFocus].userData1,item[itemFocus].userData2);
		return;
	}

	if (item[itemFocus].ptr)
	{
		// call other popup
		CSmodPopUp *pu = (CSmodPopUp*)( item[itemFocus].ptr );
		hide();
		pu->doPopUp(ox+w,oy+itemFocus*lh);
	}

}





void CSmodPopUp::draw()
{
	if (!visible) return;

	// background
	fl_color(0);
	fl_rectf(ox,oy,w,h);

	// items
	for (int i=0;i<nrItem;i++)
	{
		int y = oy+i*lh;

		if (itemFocus==i)
		{
			fl_color(FL_DARK_GREEN);
			fl_rectf(ox,y,w,lh);
		}
		fl_color(FL_GRAY);
		fl_xyline(ox, y, ox+w);
		fl_color(FL_WHITE);
		fl_draw(item[i].caption, ox+5, y+lh-2);
	}
}









///////////////////////////// GLOBALS ////////////////////////////////////////////////////////

void CSpopup_draw()
{
	if (!CSactivePopUp) return;
	CSactivePopUp->draw();
}

bool CSpopup_onMouseDown(int x, int y, int mk)
{
	if (!CSactivePopUp) return false;
	return CSactivePopUp->onMouseDown(x,y,mk);
}

bool CSpopup_onMouseMove(int x, int y)
{
	if (!CSactivePopUp) return false;
	return CSactivePopUp->onMouseMove(x,y);
}

bool CSpopup_onMouseUp(int x, int y)
{
	if (!CSactivePopUp) return false;
	return CSactivePopUp->onMouseUp(x,y);
}

bool CSpopup_onKeyDown(int k)
{
	if (!CSactivePopUp) return false;
	return CSactivePopUp->onKeyDown(k);
}
