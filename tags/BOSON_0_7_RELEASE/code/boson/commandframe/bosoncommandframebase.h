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
#ifndef BOSONCOMMANDFRAMEBASE_H
#define BOSONCOMMANDFRAMEBASE_H

#include <qframe.h>
#include <qvaluelist.h>
#include "../global.h"

class Unit;
class UnitBase;
class Facility;
class Player;
class BosonOrderButton;
class BoSelection;
class BoUnitDisplayBase;
class BosonOrderWidget;

class KPlayer;
class QVBox;
class QScrollView;


/**
 * @short The frame where you can order units
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonCommandFrameBase : public QFrame
{
	Q_OBJECT
public:

	BosonCommandFrameBase(QWidget* parent);
	~BosonCommandFrameBase();

	/**
	 * @param p The player whose units can be produced here.
	 **/
	void setLocalPlayer(Player* p);
	Player* localPlayer() const;

	/**
	 * Ok, I know this function is a hack. Since we use QToolBar as parent
	 * of BosonCommandFrameBase we need to do some ugly things - like
	 * reparenting the minimap. It is done only once, immediately after
	 * constructing the command frame.
	 *
	 * Seriously it does not even belong here. It belongs <em>next</em> to
	 * BosonCommandFrameBase onto the QToolBar. But hey - it doesn't hurt here
	 * and it's an easy solution for the background pixmap :-)
	 **/
	void reparentMiniMap(QWidget* map);

	/**
	 * @internal
	 * Used by BoUnitDisplayBase only. 
	 **/
	void addUnitDisplayWidget(BoUnitDisplayBase*);

	/**
	 * @return The parent widget for the unit actions (move, attack,
	 * stop...). You need to show this explicitly, as it is hidden by
	 * default (as its not used by the editor).
	 **/
	QVBox* unitActionsBox() const;

	/**
	 * The "plugin widget".
	 * @return The parent box for the @ref BoUnitDisplayBase widgets. You
	 * can use another parent but this should be used, if possible
	 **/
	QVBox* unitDisplayBox() const;

	/**
	 * @return The leader of the current selection (see @ref
	 * BosonBigDisplayBase::selection). All unit specific items will be
	 * displayed for this unit.
	 **/
	Unit* selectedUnit() const { return mSelectedUnit; }

public slots:
	/**
	 * Derived classes should apply this value also to the unit actions
	 * widget (see @ref showUnitActions)
	 **/
	virtual void slotSetButtonsPerRow(int b);

	/**
	 * Should be called when the production of the factory changes, i.e. is
	 * stopped/paused or started.
	 **/
	void slotUpdateProduction(Unit* factory);

	void slotSelectionChanged(BoSelection*);

	void slotUpdateProductionOptions();

protected:
	/**
	 * Clear everything that displays currently selected items/units. E.g.
	 * clear the unitview (i.e. the big image on the top) and the list of
	 * selected units.
	 **/
	virtual void clearSelection();

	/**
	 * Set the @ref selectedUnit and display the unit in the unit view (the
	 * image on the top).
	 *
	 * Derived classes should reimplement (and call) this. Here e.g. all
	 * plugin widgets (construction progress, harvester filling, ...) should
	 * be shown and updated.
	 **/
	virtual void setSelectedUnit(Unit* unit);

	/**
	 * In game mode this should display the order buttons if @p unit has a
	 * production plugin.
	 **/
	virtual void setProduction(Unit* unit) = 0;

	/**
	 * Display the unit actions. Mostly used by game mode - there we display
	 * the move, attack, ... buttons.
	 *
	 * <em>Might</em> be useful for the editor - we could provide buttons to
	 * configure the unit (health, ...) here.
	 *
	 * Note that an implementation must hide the widget if unit is NULL!
	 **/
	virtual void showUnitActions(Unit* unit) = 0;

	/**
	 * @return The widget where the selected units are displayed, as well as
	 * the order buttons (in case a single unit is selected and has a
	 * production plugin)
	 **/
	BosonOrderWidget* selectionWidget() const;

	/**
	 * Construct and return a @ref QScrollView that should be used as parent
	 * for the placement items (i.e. cell placement, unit placement)
	 **/
	QScrollView* addPlacementView();

signals:
	/**
	 * Emitted when a unit should be produced.
	 * @param factory The factory producing the unit. Usually equal to the
	 * selected unit. Is NULL in editor mode
	 * @param owner The local player. See @ref setLocalPlayer. Note that
	 * this is (in game mode) the same as factory->owner()
	 **/
	void signalProduce(ProductionType type, unsigned long int id, UnitBase* factory, KPlayer* owner);
	void signalStopProduction(ProductionType type, unsigned long int id, UnitBase* factory, KPlayer* owner);

	void signalPlaceUnit(unsigned long int unitType, Player* owner);

	/**
	 * @param groundType The tile number. See @ref BosonTiles::tile to get
	 * the actual pixmap.
	 **/
	void signalPlaceCell(int groundType);

	/**
	 * Emitted when user clicks on action button (e.g move)
	 */
	void signalAction(int actionType);

	/**
	 * This unit should become the only selected unit. See @ref
	 * BosonOrderButton::signalSelectUnit
	 **/
	void signalSelectUnit(Unit* unit);

protected slots:
	/**
	 * Game mode only. Emit @ref signalProduce.
	 **/
	void slotProduce(ProductionType type, unsigned long int id);

	/**
	 * Game mode only. Emit @ref signalStopProduction.
	 **/
	void slotStopProduction(ProductionType type, unsigned long int id);

	/**
	 * Editor mode only. Emit @ref signalPlaceUnit.
	 **/
	void slotPlaceUnit(ProductionType, unsigned long int unitType);

	virtual void slotUpdate();

protected:
	virtual void resizeEvent(QResizeEvent*);

	/**
	 * Hide all plugin widgets (harvester filling, construction progress,
	 * ...), i.e. all @ref BoUnitDisplayBase widgets.
	 **/
	void hidePluginWidgets();

	void startStopUpdateTimer();
	/**
	 * @return TRUE if the update timer should be started, otherwise FALSE
	 * (i.e. it will be stopped)
	 **/
	virtual bool checkUpdateTimer() const;

private:
	class BosonCommandFrameBasePrivate;
	BosonCommandFrameBasePrivate* d;
	Unit* mSelectedUnit;
};

/**
 * @short Base class for plugin widgets (e.g. construction progress, harvester
 * filling, ...
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUnitDisplayBase : public QWidget
{
public:
	BoUnitDisplayBase(BosonCommandFrameBase* frame, QWidget* parent);
	~BoUnitDisplayBase();

	/**
	 * @return TRUE if the widget gets displayed (shown), otherwise FALSE.
	 **/
	bool showUnit(Unit* unit)
	{
		if (unit && display(unit)) {
			show();
			mUpdateTimer = true;
			return true;
		}
		hide();
		mUpdateTimer = false;
		return false;
	}

	bool updateTimer() const
	{
		return mUpdateTimer && useUpdateTimer();
	}

	BosonCommandFrameBase* cmdFrame() const { return mCommandFrame; }

protected:
	/**
	 * Show the specified unit. Note that unit can't be NULL.
	 * @return TRUE if the widget should get displayed (shown), otherwise
	 * FALSE.
	 **/
	virtual bool display(Unit* unit) = 0;

	/**
	 * Make this return false if the derived class doesn't want to use an
	 * update timer.
	 **/
	virtual bool useUpdateTimer() const { return true; }

private:
	BosonCommandFrameBase* mCommandFrame;
	bool mUpdateTimer;
};

#endif
