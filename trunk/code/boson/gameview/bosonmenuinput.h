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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef BOSONMENUINPUT_H
#define BOSONMENUINPUT_H

#include "kgame/kgameio.h"

class BoUfoActionCollection;
class Player;
class PlayerIO;
class KPlayer;
class BoGameCamera;

class BosonMenuInputDataPrivate;
class BosonMenuInputData : public QObject
{
	Q_OBJECT
public:
	BosonMenuInputData(BoUfoActionCollection* parent);
	~BosonMenuInputData();

	void initUfoActions(bool gameMode);

	void resetDefaults();

	void setLocalPlayerIO(PlayerIO* io);

signals:
	void signalScroll(int direction);
	void signalRotateLeft();
	void signalRotateRight();
	void signalZoomIn();
	void signalZoomOut();
	void signalToggleStatusbar();
	void signalToggleStatusbar(bool);
	void signalToggleChatVisible();
	void signalResetViewProperties();
	void signalSetGrabMovie(bool);
	void signalSetDebugMode(int);
	void signalDebugEditPlayerInputs(Player* p);
	void signalDebugKillPlayer(Q_UINT32 playerId);
	void signalDebugModifyMinerals(Q_UINT32 playerId, int amount);
	void signalDebugModifyOil(Q_UINT32 playerId, int amount);
	void signalToggleCheating(bool);
	void signalUnfogAll();
	void signalDumpGameLog();
	void signalEditConditions();
	void signalReloadMeshRenderer();
	void signalReloadGroundRenderer();
	void signalReloadGameViewPlugin();
	void signalShowLight0Widget();
	void signalDebugMemory();
	void signalEndGame();
	void signalQuit();
	void signalPreferences();

signals: // game actions
	void signalSaveGame();
	void signalCenterHomeBase();
	void signalSyncNetwork();
	void signalSelectSelectionGroup(int);
	void signalCreateSelectionGroup(int);
	void signalShowSelectionGroup(int);

signals: // editor actions
	void signalEditorSavePlayFieldAs();
	void signalEditorChangeLocalPlayer(Player*);
	void signalEditorShowPlaceFacilities();
	void signalEditorShowPlaceMobiles();
	void signalEditorShowPlaceGround();
	void signalEditorDeleteSelectedUnits();
	void signalEditorEditMapDescription();
	void signalEditorEditPlayerMinerals();
	void signalEditorEditPlayerOil();
	void signalEditorEditHeight(bool);
	void signalEditorImportHeightMap();
	void signalEditorExportHeightMap();
	void signalEditorImportTexMap();
	void signalEditorExportTexMap();
	void signalEditorUndo();
	void signalEditorRedo();


protected:
	void initUfoGameActions();
	void initUfoEditorActions();
	void createDebugPlayersMenu();
	void createEditorPlayerMenu();

	BoUfoActionCollection* actionCollection() const;

	void resetToggleAction(bool defaultValue, const QString& name);

protected slots:
	void slotDebugPlayer(int index);
	void slotEditorPlace(int index);
	void slotEditorChangeLocalPlayer(int index);
	void slotEditorChangeLocalPlayerHack();
	void slotChangeColorMap(int index);
	void slotUpdateColorMapsMenu();

private:
	BosonMenuInputDataPrivate* d;
};

class BosonMenuInput : public KGameIO
{
	Q_OBJECT
public:
	enum _RTTI {
		RTTI = 126
	};
public:
	BosonMenuInput(bool gameMode = true);
	virtual ~BosonMenuInput();

	virtual void initIO(KPlayer*);

	virtual int rtti() const { return RTTI; }

	/**
	 * This is a workaround for the fact that (currently)
	 * we don't allow including player.h in this file.
	 *
	 * We should somehow make sure, that the IO can access
	 * player()->playerIO()
	 **/
	void setPlayerIO(PlayerIO* io);
	PlayerIO* playerIO() const;

	void setCamera(BoGameCamera*);
	BoGameCamera* camera() const;

	void setActionCollection(BoUfoActionCollection* a);
	BoUfoActionCollection* actionCollection() const;

public slots:
	void slotEditorHasUndo(const QString&);
	void slotEditorHasRedo(const QString&);

signals:
	void signalScroll(int direction);
	void signalToggleStatusbar(bool);
	void signalToggleChatVisible();
	void signalResetViewProperties();
	void signalShowLight0Widget();
	void signalSelectSelectionGroup(int);
	void signalCreateSelectionGroup(int);
	void signalShowSelectionGroup(int);
	void signalPreferencesApply();
	void signalUpdateOpenGLSettings();
	void signalChangeCursor(int mode, const QString& cursorDir);
	void signalEndGame();
	void signalQuit();
	void signalSaveGame();
	void signalReloadGameViewPlugin();
	void signalDebugAddLocalPlayerInput();
	void signalDebugAddMenuInput();
	void signalEditorChangeLocalPlayer(Player*);
	void signalEditorShowPlaceFacilities();
	void signalEditorShowPlaceMobiles();
	void signalEditorShowPlaceGround();
	void signalEditorDeleteSelectedUnits();
	void signalEditorEditHeight(bool);
	void signalEditorUndo();
	void signalEditorRedo();

protected slots:
	void slotRotateLeft();
	void slotRotateRight();
	void slotZoomIn();
	void slotZoomOut();

	void slotSetDebugMode(int);
	void slotDebugEditPlayerInputs(Player* p);
	void slotDebugKillPlayer(Q_UINT32 playerId);
	void slotDebugModifyMinerals(Q_UINT32 playerId, int amount);
	void slotDebugModifyOil(Q_UINT32 playerId, int amount);
	void slotToggleCheating(bool);
	void slotUnfogAll(Player* pl = 0);
	void slotDumpGameLog();
	void slotEditConditions();
	void slotReloadMeshRenderer();
	void slotReloadGroundRenderer();
	void slotDebugMemory();
	void slotPreferences();
	void slotSetGrabMovie(bool);

	void slotCenterHomeBase();
	void slotSyncNetwork();
	void slotEndGame();

	void slotEditorSavePlayFieldAs();
	void slotEditorEditMapDescription();
	void slotEditorEditPlayerMinerals();
	void slotEditorEditPlayerOil();
	void slotEditorImportHeightMap();
	void slotEditorImportTexMap();
	void slotEditorExportHeightMap();
	void slotEditorExportTexMap();

private:
	bool mGameMode;
	BoGameCamera* mCamera;
	PlayerIO* mPlayerIO;
	BoUfoActionCollection* mActionCollection;
	BosonMenuInputData* mData;
};

#endif

