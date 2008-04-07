/*
    This file is part of the Boson game
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOSONUNITVIEW_H
#define BOSONUNITVIEW_H

#include "bosonorderbutton.h"
//Added by qt3to4:
#include <QPixmap>

class Unit;

class BosonUnitViewPrivate;
/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUnitView : public BosonOrderButton
{
	Q_OBJECT
public:
	BosonUnitView();
	~BosonUnitView();

	/**
	 * @param unit The unit to be shown or 0 for none
	 **/
	virtual void setUnit(Unit* unit);

public slots:
	void slotUnitChanged(Unit*);

protected:

        /**
	 * Set the big overview pixmap.
	 * @param p The pixmap to be displayed. 0 for none.
	 **/
	void setOverview(QPixmap* p);

	void hideAll();
	void showGeneral();

	virtual void displayUnitPixmap(quint32 unitType, const Player* owner);
	using BosonOrderButton::displayUnitPixmap;

private:
	BosonUnitViewPrivate* d;
};

#endif
