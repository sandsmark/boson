/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef EDITORCOMMANDFRAME_H
#define EDITORCOMMANDFRAME_H

#include "bosoncommandframebase.h"
#include "../global.h"

class Unit;
class UnitBase;
class Facility;
class Player;
class BosonOrderButton;
class BoSelection;
class BosonTiles;

class KPlayer;

/**
 * @short The frame where you can order units
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class EditorCommandFrame : public BosonCommandFrameBase
{
	Q_OBJECT
public:

	EditorCommandFrame(QWidget* parent);
	~EditorCommandFrame();

	void setTileSet(BosonTiles* t);

	/**
	 * @param owner necessary for units (mobile and facilities) only.
	 **/
	void placeCells(CellType type);

	void placeMobiles(Player* owner);
	void placeFacilities(Player* owner);

public slots:
	virtual void slotSetButtonsPerRow(int b);

protected:
	/**
	 * Set the plugins. See @ref BosonCommandFrameBase::setSelectedUnit
	 *
	 * We might use plugins for unit-configurations, e.g. to change the
	 * initial health of a unit.
	 *
	 * Currently not used.
	 * @param unit The selected unit
	 **/
	virtual void setSelectedUnit(Unit* unit);

	/**
	 * See @ref BosonCommandFrameBase::showUnitActions
	 *
	 * We might use this for configuration widget, but i'd prefer plugins -
	 * see @ref setSelectedUnit.
	 **/
	virtual void showUnitActions(Unit*);

	/**
	 * See @ref BosonCommandFrameBase::setProduction
	 *
	 * We might use this for configuration widget, but i'd prefer plugins -
	 * see @ref setSelectedUnit.
	 **/
	virtual void setProduction(Unit* unit);


	/**
	 * See @ref BosonCommandFrameBase::checkUpdateTimer. Additionally all
	 * plugins are checked, whether they want to be updated.
	 **/
	virtual bool checkUpdateTimer() const;

protected slots:
	void slotUpdate();

private:
	void init();

private:
	class EditorCommandFramePrivate;
	EditorCommandFramePrivate* d;
};

#endif
