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
#ifndef BOSONCOMMANDFRAME_H
#define BOSONCOMMANDFRAME_H

#include "bosoncommandframebase.h"

class Unit;
class UnitBase;
class Facility;
class Player;
class BosonOrderButton;
class BoSelection;
class BoUnitDisplayBase;

class KPlayer;

/**
 * @short The frame where you can order units
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonCommandFrame : public BosonCommandFrameBase
{
	Q_OBJECT
public:

	BosonCommandFrame(QWidget* parent);
	~BosonCommandFrame();

public slots:
	virtual void slotSetButtonsPerRow(int b);

protected:
	/**
	 * Mostly sets the plugin widget, such as the construction progress bar.
	 * See also @ref BosonCommandFrameBase::setSelectedUnit.
	 * @param unit The selected unit
	 **/
	virtual void setSelectedUnit(Unit* unit);

	/**
	 * Display the production options (i.e. whatever the @p unit can
	 * produce) in the @ref selectionWidget.
	 *
	 * Note: a NULL unit is valid, but will hide all order buttons, i.e.
	 * don't call this if there are multiple units selected, as they won't
	 * be shown anymore.
	 **/
	virtual void setProduction(Unit* unit);

	virtual void showUnitActions(Unit* unit);


	/**
	 * See @ref BosonCommandFrameBase::checkUpdateTimer. Additionally all
	 * plugins are checked, whether they want to be updated.
	 **/
	virtual bool checkUpdateTimer() const;

protected slots:
	/**
	 * If the selected unit is a facility that has not been constructed
	 * completely (see @ref Unit::isConstructionComplete) show how far the
	 * construction is currently.
	 **/
//	void slotShowConstructionProgress(Facility* fac);


	void slotUpdate();

private:
	void init();
	void initPlugins();

private:
	class BosonCommandFramePrivate;
	BosonCommandFramePrivate* d;
};

#endif
