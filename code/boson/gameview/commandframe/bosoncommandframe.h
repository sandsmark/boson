/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONCOMMANDFRAME_H
#define BOSONCOMMANDFRAME_H

#include "../../boufo/boufowidget.h"

class Player;
class PlayerIO;
class Unit;
class BoSelection;
class BoUnitDisplayBase;
class BoSpecificAction;
class BosonGroundTheme;
class UnitProperties;
class UpgradeProperties;

class BosonCommandFramePrivate;
class BosonCommandFrame : public BoUfoWidget
{
	Q_OBJECT
public:
	BosonCommandFrame();
	~BosonCommandFrame();

	void setGameMode(bool game);

	void setLocalPlayerIO(PlayerIO* p);
	PlayerIO* localPlayerIO() const;

	BoSelection* selection() const;

	/**
	 * @internal
	 * Used by @ref BoUnitDisplayBase
	 **/
	void addUnitDisplayWidget(BoUnitDisplayBase* w);


	void placeGround();
	void placeMobiles(PlayerIO* io);
	void placeFacilities(PlayerIO* io);

	const QPoint* cursorRootPos() const;
	void setCursorRootPos(const QPoint* pos);

signals:
	/**
	 * This unit should become the only selected unit. See @ref
	 * BosonOrderButton::signalSelectUnit
	 **/
	void signalSelectUnit(Unit*);

	/**
	 * Emitted when user clicks on action button (e.g move). Also used for
	 * the placement preview, when the player clicks on a constructed
	 * facility and wants it to be placed on the map.
	 */
	void signalAction(const BoSpecificAction&);

	// AB: maybe use PlayerIO instead of Player parameter
	void signalPlaceUnit(unsigned int unitType, Player* owner);

	/**
	 * @param textureCount See @ref BosonGroundTheme::textureCount
	 * @param alpha An array (of size @þ textureCount) defining how much of
	 * every texture should get displayed. 255 is maximum, 0 is nothing.
	 **/
	void signalPlaceGround(unsigned int textureCount, unsigned char* alpha);


public slots:
	void slotUpdate();
	void slotUpdateProduction(quint32 id);
	void slotUpdateProduction(Unit*);
	void slotUpdateProductionOptions();
	void slotUpdateSelection();
	void slotSelectionChanged(BoSelection*);
	void slotProduce(const BoSpecificAction&);
	void slotConstructionCompleted(quint32 facilityId);
	void slotUnitDestroyed(quint32 id);

	/**
	 * Editor mode only. Emit @ref signalPlaceUnit.
	 **/
	void slotPlaceUnit(const BoSpecificAction& action);

	/**
	 * Editor mode only. Emit @ref signalPlaceGround.
	 **/
	void slotPlaceGround(unsigned int textureCount, unsigned char* alpha);

	void slotSetGroundTheme(BosonGroundTheme* theme);

	void slotUnitTypeHighlighted(const PlayerIO* player, const UnitProperties* prop);
	void slotTechnologyHighlighted(const PlayerIO* player, const UpgradeProperties* prop);

protected:
	/**
	 * Clear everything that displays currently selected items/units. E.g.
	 * clear the unitview (i.e. the big image on the top) and the list of
	 * selected units.
	 **/
	void clearSelection();

	/**
	 * Set the @ref selectedUnit and display the unit in the unit view (the
	 * image on the top).
	 *
	 * Here e.g. all plugin widgets (construction progress, harvester
	 * filling, ...) should be shown and updated.
	 **/
	void setSelectedUnit(Unit*);

	/**
	 * @return The leader of the current selection (see @ref
	 * BosonBigDisplayBase::selection). All unit specific items will be
	 * displayed for this unit.
	 **/
	Unit* selectedUnit() const { return mSelectedUnit; }

	/**
	 * In game mode this should display the order buttons if @p unit has a
	 * production plugin.
	 **/
	void setProduction(Unit* unit);

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
	bool checkUpdateTimer() const;

	/**
	 * Display the unit actions. Mostly used by game mode - there we display
	 * the move, attack, ... buttons.
	 *
	 * <em>Might</em> be useful for the editor - we could provide buttons to
	 * configure the unit (health, ...) here.
	 **/
	void showUnitActions(Unit* unit);

	void showPluginWidgetsForUnit(Unit*);

protected slots:
	void slotUpdateUnitConfig();

private:
	void initUnitView();
	void initUnitActions();
	void initUnitDisplayBox();
	void initGamePlugins();
	void initEditorPlugins();
	void initSelectionWidget();
	void initPlacementWidget();
	void initUnitInfo();

private:
	BosonCommandFramePrivate* d;
	Unit* mSelectedUnit;
};


/**
 * @short Base class for plugin widgets (e.g. construction progress, harvester
 * filling, ...
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUnitDisplayBase : public BoUfoWidget
{
public:
	BoUnitDisplayBase(BosonCommandFrame* frame);
	~BoUnitDisplayBase();

	void setAvailableInGame(bool g) { mAvailableInGame = g; }
	bool availableInGame() const { return mAvailableInGame; }
	void setAvailableInEditor(bool g) { mAvailableInEditor = g; }
	bool availableInEditor() const { return mAvailableInEditor; }

	/**
	 * Set the current game mode
	 **/
	void setGameMode(bool game) { mGameMode = game; }

	/**
	 * @return TRUE if the widget gets displayed (shown), otherwise FALSE.
	 **/
	bool showUnit(Unit* unit);

	bool updateTimer() const
	{
		return mUpdateTimer && useUpdateTimer();
	}

	BosonCommandFrame* cmdFrame() const { return mCommandFrame; }

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

	bool isAvailable() const
	{
		if (mGameMode && availableInGame() ||
				!mGameMode && availableInEditor()) {
			return true;
		}
		return false;
	}

private:
	BosonCommandFrame* mCommandFrame;
	bool mUpdateTimer;
	bool mAvailableInGame;
	bool mAvailableInEditor;
	bool mGameMode;
};

#endif
