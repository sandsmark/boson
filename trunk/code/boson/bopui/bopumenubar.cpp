/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// note the copyright above: this is LGPL!

// AB: most code has been stolen from plib 1.8.3, src/pui/puMenuBar.cxx

#include "bopumenubar.h"

#include <bodebug.h>

#include <stdlib.h>
#include <string.h>

void bopuMenuBar_drop_down_the_menu (puObject *b)
{
 // Inputs:  b = pointer to the button on the menu which has been pushed
 // p = pointer to the popup menu which is b's submenu

 puPopupMenu *p = (puPopupMenu *) b -> getUserData () ;

  if ( b -> getIntegerValue () )
  {
/*
  SJBL Test hack.
*/
    puDeactivateWidget () ;

    p->reveal () ;   // Reveal the submenu

    // If the parent floats in its own window, and the submenu drops off the window,
    // expand the window to fit.

#if 1
    puGroup *parent = p -> getParent () ;

    if ( ( parent != parent -> getParent () ) && parent -> getFloating () )  // DEPRECATED! -- we need to replace this code.
    {
      int temp_window = puGetWindow () ;
      puSetWindow ( parent -> getWindow () ) ;

      puBox *par_box = parent -> getBBox () ;
      puBox *cur_box = p -> getBBox () ;
      int x_min = (cur_box->min[0] < 0) ? par_box->min[0] + cur_box->min[0] : par_box->min[0] ;
      int x_max = (par_box->max[0] > par_box->min[0] + cur_box->max[0]) ?
                                    par_box->max[0] : par_box->min[0] + cur_box->max[0] ;
      int y_min = (cur_box->min[1] < 0) ? par_box->min[1] + cur_box->min[1] : par_box->min[1] ;
      int y_max = (par_box->max[1] > par_box->min[1] + cur_box->max[1]) ?
                                    par_box->max[1] : par_box->min[1] + cur_box->max[1] ;
      int x_siz, y_siz;
      puGetWindowSize ( &x_siz, &y_siz ) ;
      if ( x_siz < (x_max - x_min) ) x_siz = x_max - x_min ;    // Adjust the present size
      if ( y_siz < (y_max - y_min) ) y_siz = y_max - y_min ;
      puSetWindowSize ( x_siz, y_siz ) ;

      x_min = par_box->min[0] - x_min ;
      y_min = y_siz - ( par_box->max[1] - par_box->min[1] ) ;

      /* If the parent window is SUPPOSED to be stuck at the top of the screen, move it. 
         - JCJ 6 June 2002 */
      if (parent -> getVStatus () == 1)
        parent -> setPosition ( x_min, y_min ) ;

      puSetWindow ( temp_window ) ;
    }
#endif
  }
  else
    p->hide () ;

  for ( puObject *child = b -> getParent () -> getFirstChild () ;
        child != NULL ; child = child -> getNextObject() )
  {
    if (( child -> getType() & PUCLASS_BUTTON    ) != 0 && child != b ) child -> clrValue () ;
    if (( child -> getType() & PUCLASS_POPUPMENU ) != 0 && child != p ) child -> hide     () ;
  }

  // Move the popup menu to the last item in the "dlist" so it is drawn last
  // (in front of everything else).

  puMoveToLast ( p );
}


class bopuMenuBarPrivate
{
public:
	bopuMenuBarPrivate()
	{
	}

};

bopuMenuBar::bopuMenuBar()
	: puInterface(0, 0)
{
 d = new bopuMenuBarPrivate;
 type |= PUCLASS_MENUBAR;
}

bopuMenuBar::~bopuMenuBar()
{
 delete d;
}

// don't forget to call close() on the returned menu once you added the items!
puPopupMenu* bopuMenuBar::addSubMenu(puPopupMenu* parent, const char* str)
{
 int w, h ;
 if (parent) {
	parent->getSize ( &w, &h ) ;
 } else {
	// toplevel menu
	getSize ( &w, &h ) ;
 }

 puOneShot *b = new puOneShot(w + 10, 0, str);
 b->setStyle(PUSTYLE_SPECIAL_UNDERLINED);
 b->setColourScheme(colour[PUCOL_FOREGROUND][0],
		colour[PUCOL_FOREGROUND][1],
		colour[PUCOL_FOREGROUND][2],
		colour[PUCOL_FOREGROUND][3]);
 b->setCallback(bopuMenuBar_drop_down_the_menu);
 b->setActiveDirn(PU_UP_AND_DOWN);

 puPopupMenu *p = new puPopupMenu(w + 10, 0);

 b->setUserData(p);
 return p;
}

void bopuMenuBar::closeSubMenu(puPopupMenu* p)
{
 p->close();
 recalc_bbox();
}

void bopuMenuBar::addMenuItem(puPopupMenu* p, const char* text, puCallback cb, void* userData)
{
 if (userData) {
	p->add_item(text, cb, userData);
 } else {
	p->add_item(text, cb);
 }
}

void bopuMenuBar::add_submenu(const char *str, char *items[], puCallback _cb[],
                              void *_user_data[])
{
 puPopupMenu* p = addSubMenu(str);
 for (int i = 0; items[i] != NULL; i++) {
	addMenuItem(p, items[i], _cb[i], _user_data ? _user_data[i] : 0);
 }
 closeSubMenu(p);
}

void bopuMenuBar::close (void)
{
 puInterface::close();

 if (dlist == NULL) {
	return;
 }

 int width = 0;
 puObject* ob;

 /*
   Use alternate objects - which gets the puOneShot/puPopupMenu pairs
 */

 for (ob = dlist; ob != NULL; ob = ob->getNextObject()) {
	int w, h;

	/* Reposition the button so it looks nice */

	ob->getSize(&w, &h);
	ob->setPosition(width, 0);
	ob = ob->getNextObject();

	/* Reposition the submenu so it sits under the button */

	int w2, h2;
	ob->getSize(&w2, &h2);
	ob->setPosition(width, -h2);

	/* Next please! */
	width += w;
 }

 recalc_bbox ();
}


