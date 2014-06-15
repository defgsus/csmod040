/**
 *
 * CS MODULAR EDITOR MENU CLASSES AND FUNCTIONS
 *
 *
 * @version 2010/12/19 just started
 * @author def.gsus-
 *
 */
#ifndef CSMOD_MENU_H_INCLUDED
#define CSMOD_MENU_H_INCLUDED

#include <stdlib.h>
#include <string.h>

#include "FL/Fl.H"
#include "FL/Fl_Draw.H"
#include "FL/Enumerations.H"

//////////////////////////// CSmodPopUp //////////////////////////////////////////////////////

#define CSMOD_POPUP_MIN_ITEM 64

struct CSmodPopUpItem
{
	char
		*caption;
	void
		*ptr,
		*userData1,
		*userData2;

};

/**
	a popup menu
	*/
class CSmodPopUp
{
	public:

	/** list of item pointers */
	CSmodPopUpItem
		*item;

	int
		/** nr of items in menu */
		nrItem,
		/** length of item list */
		nrItemAlloc,

		/** visible/active */
		visible,
		/** screen x offset */
		ox,
		/** screen y offset */
		oy,

		/** width */
		w,
		/** height */
		h,
		/** line height */
		lh,

		/** the item that has mouse focus */
		itemFocus;




	/** constructor */
	CSmodPopUp ();

	/** destructor */
	~CSmodPopUp ();

	/** delete all items */
	void clear();

	/** add an item to list,
		if userData1 == 0 then ptr must be a CSmodPopUp and will be called when clicked,
		if userData1 != 0 ptr will be used as a function(void *userData1, void *userData2) */
	void addItem(const char *caption, void *ptr = 0, void *userData1 = 0, void *userData2 = 0);

	/** show the menu */
	void doPopUp(int x, int y);

	/** make it disappear */
	void hide();

	/** call to aknowledge mouse down, returns true if somethings changed */
	bool onMouseDown(int x, int y, int mk);

	/** call to aknowledge mouse move event, returns true if somethings changed */
	bool onMouseMove(int x, int y);

	/** call to aknowledge mouse up, returns true if somethings changed */
	bool onMouseUp(int x, int y);

	/** call to aknowledge key events, returns true if somethings changed */
	bool onKeyDown(int k);

	/** click an item */
	void doClick(int itemFocus);

	/** draw the menu */
	void draw();
};





///////////////////////////// GLOBALS ////////////////////////////////////////////////////////

static CSmodPopUp
	/** the current active menu, or nil */
	*CSactivePopUp = 0;


/** draw the active popup */
void CSpopup_draw();

/** call to aknowledge mouse down, returns true if somethings changed */
bool CSpopup_onMouseDown(int x, int y, int mk);

/** call to aknowledge mouse move event, returns true if somethings changed */
bool CSpopup_onMouseMove(int x, int y);

/** call to aknowledge mouse up, returns true if somethings changed */
bool CSpopup_onMouseUp(int x, int y);

/** call to aknowledge key event */
bool CSpopup_onKeyDown(int k);


#endif // CSMOD_MENU_H_INCLUDED
