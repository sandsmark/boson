/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef __BOSONUNITVIEW_H__
#define __BOSONUNITVIEW_H__

#include <qframe.h>

class Unit;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUnitView : public QFrame
{
	Q_OBJECT
public:
	BosonUnitView(QWidget* parent);
	~BosonUnitView();

	/**
	 * @param unit The unit to be shown or 0 for none
	 **/
	void setUnit(Unit* unit);

protected:

        /**
	 * Set the big overview pixmap.
	 * @param p The pixmap to be displayed. 0 for none.
	 **/
	void setOverview(QPixmap* p);


	void hideAll();
	void hideMobile();
	void hideFacility();
	void showGeneral();
	void showMobile();
	void showFacility();

private:
	class BosonUnitViewPrivate;
	BosonUnitViewPrivate* d;
};

#endif
