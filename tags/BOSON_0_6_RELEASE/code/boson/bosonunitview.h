/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef BOSONUNITVIEW_H
#define BOSONUNITVIEW_H

#include "bosoncommandwidget.h"

class Unit;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUnitView : public BosonCommandWidget
{
	Q_OBJECT
public:
	BosonUnitView(QWidget* parent);
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

	virtual void displayUnitPixmap(int unitType, Player* owner);

private:
	class BosonUnitViewPrivate;
	BosonUnitViewPrivate* d;
};

#endif
