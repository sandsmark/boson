/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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
class KPlayer;

class BosonMenuInputDataPrivate;
class BosonMenuInputData : public QObject
{
	Q_OBJECT
public:
	BosonMenuInputData(BoUfoActionCollection* parent);
	~BosonMenuInputData();

	void initUfoActions(bool gameMode);

	void resetDefaults();

signals:
	void signalScroll(int direction);
	void signalRotateLeft();
	void signalRotateRight();
	void signalZoomIn();
	void signalZoomOut();
	void signalToggleStatusbar();
	void signalToggleStatusbar(bool);
	void signalToggleSound();
	void signalToggleMusic();
	void signalToggleFullScreen(bool);
	void signalToggleChatVisible();
	void signalChangeMaxProfilingEventEntries();
	void signalChangeMaxProfilingAdvanceEntries();
	void signalChangeMaxProfilingRenderingEntries();
	void signalProfiling();
	void signalDebugKGame();
	void signalBoDebugLogDialog();
	void signalSleep1s();
	void signalResetViewProperties();
	void signalGrabScreenshot();
	void signalGrabProfiling();
	void signalSetGrabMovie(bool);
	void signalSetShowResources(bool);
	void signalSetDebugMapCoordinates(bool);
	void signalSetDebugPFData(bool);
	void signalSetDebugShowCellGrid(bool);
	void signalSetDebugMatrices(bool);
	void signalSetDebugItemWorks(bool);
	void signalSetDebugCamera(bool);
	void signalSetDebugRenderCounts(bool);
	void signalSetDebugWireFrames(bool);
	void signalSetDebugBoundingBoxes(bool);
	void signalSetDebugFPS(bool);
	void signalSetDebugAdvanceCalls(bool);
	void signalSetDebugTextureMemory(bool);
	void signalSetEnableColorMap(bool);
	void signalSetDebugMode(int);
	void signalDebugKillPlayer(Q_UINT32 playerId);
	void signalDebugModifyMinerals(Q_UINT32 playerId, int amount);
	void signalDebugModifyOil(Q_UINT32 playerId, int amount);
	void signalToggleCheating(bool);
	void signalUnfogAll();
	void signalDumpGameLog();
	void signalEditConditions();
	void signalShowGLStates();
	void signalReloadMeshRenderer();
	void signalReloadGroundRenderer();
	void signalShowLight0Widget();
	void signalCrashBoson();
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
	BosonMenuInput();
	virtual ~BosonMenuInput();

	virtual void initIO(KPlayer*);

	virtual int rtti() const { return RTTI; }
};

#endif

