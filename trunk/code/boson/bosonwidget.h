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
#ifndef __BOSONWIDGET_H__
#define __BOSONWIDGET_H__

#include <qwidget.h>
#include <qdatastream.h>

class KPlayer;
class KGameIO;
class KGameMouseIO;
class QKeyEvent;
class KGamePropertyBase;
class KToolBar;

class BosonCanvas;
class BosonCommandFrame;
class BosonBigDisplay;
class Unit;
class Player;

/**
 * This is the actual main widget of boson for both, the game and the editor
 * mode. The widget also contains most of the important objects, like the @ref 
 * KGame object (see @ref Boson). 
 *
 * This widget conists of 3 sub-widgets:
 * @li a @ref BosonBigDisplay. This is the actual game view. Here the user can
 * click on units, move units, ...
 * @li a @ref BosonMiniMap. Well, this is just the mini map (wow ;))
 * @li a @ref BosonCommandFrame. The frame where unit can be ordered, the
 * selected unit is displayed and so on.
 *
 * The @ref BosonCommandFrame is currently a quite tricky part as the frame
 * differs heavily between game and editor mode. Maybe it will become two
 * classes one day, but the basic structure will stay.
 * 
 * BosonWidget is responsible for connecting all of these widgets and objects
 * together, which is mostly done on constructing. The editor specific parts are
 * being initialized in @ref startEditor, the game specific parts are being
 * initialized by the new game dialog.
 *
 * All game specific stuff should be done in other classes - e.g. visual stuff
 * (click on a unit) in @ref BosonBigDisplay, constructing in @ref
 * BosonCommandFrame and so on. These classes should emit signals which get
 * connected by BosonWidget to the necessary slots - probably mainly to @ref
 * Boson.
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonWidget : public QWidget 
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 **/
	BosonWidget(QWidget* parent);

	/**
	 * Default Destructor
	 **/
	virtual ~BosonWidget();

	void startGame();
	void startEditor();

	void saveConfig(bool editor = false);

	void zoom(const QWMatrix&);

	void addEditorCommandFrame(QWidget* parent);
	void addGameCommandFrame(QWidget* parent);

	/**
	 * The Mini Map belongs logically to BosonWidget. But we are now using
	 * QToolBars as parent for the minimap and this is initialized
	 * elsewhere. So we add reparent and show the minimap as soon as
	 * reparentMiniMap is called with the "real" parent.
	 **/
	void reparentMiniMap(QWidget* parent);

	bool sound() const;
	bool music() const;

	/**
	 * Used by the editor: either display or hide all units. You usually
	 * don't want to see the units when you edit the map, but sometimes
	 * (*g*) you need to see the units when you design a scenario...
	 **/
	void displayAllItems(bool display);

	bool isModified() const;
	void setModified(bool);

	void setShowChat(bool s);

public slots:
	void slotDebug();
	void slotNewGame();
	void slotGamePreferences();
	void slotEndGame();

	void slotLoadMap(const QString& map);
	void slotLoadScenario(const QString& scenario);
	void slotChangeLocalPlayer(int playerNumber);

	/**
	 * Called by @ref EditorTop, the map editor, when the construction frame
	 * shall be changed (mobile -> facilities or the other way round)
	 **/
	void slotEditorConstructionChanged(int index);

	void slotEditorSaveMap(const QString& fileName);
	void slotEditorSaveScenario(const QString& fileName);

	void slotToggleSound();
	void slotToggleMusic();

	void slotSetCommandButtonsPerRow(int b);

	/**
	 * Unfogs the map for the specified player
	 * @param player The player that shall see the map or NULL for all
	 * players
	 **/
	void slotUnfogAll(Player* player = 0);

	void slotSplitViewHorizontal();
	void slotSplitViewVertical();
	void slotRemoveActiveView(); // TODO

signals:
	void signalPlayerJoinedGame(KPlayer* p); // used by the map editor
	void signalPlayerLeftGame(KPlayer* p); // used by the map editor

	/**
	 * Emitted when a new tileset shall be loaded. This is usually emitted
	 * only once (at program startup), at least currently.
	 *
	 * The editor should load the new tileset and probably also update all
	 * cells on the screen. Currently tileSet is always "earth.png"
	 **/
	void signalEditorLoadTiles(const QString& tileSet);

	/**
	 * Emitted when the number of units of the local player changes.
	 **/
	void signalMobilesCount(int mobileUnits);

	/**
	 * Emitted when the number of units of the local player changes.
	 **/
	void signalFacilitiesCount(int facilities);

	void signalMineralsUpdated(int);
	void signalOilUpdated(int);

	void signalGameStarted();

	void signalMoveCommandFrame(int);

protected slots:
	void slotChatFramePosition(int);
	void slotCommandFramePosition(int);
	
	void slotStartScenario();
	void slotSendChangeSpecies(const QString& species);
	void slotSendChangeTeamColor(const QColor& color);

	void slotUnfog(int x, int y);
	void slotFog(int x, int y);

	void slotPlayerPropertyChanged(KGamePropertyBase*, KPlayer*);

	void slotInitFogOfWar();

	void slotNotEnoughMinerals(Player*);
	void slotNotEnoughOil(Player*);

	void slotChangeCursor(int mode);

protected:
	void addChatSystemMessage(const QString& fromName, const QString& text);
	
	void sendChangeTeamColor(Player* player, const QColor& color);
	void changeSpecies(const QString& species);
	void addLocalPlayer();

	void addDummyComputerPlayer(const QString& name); // used by editor only

	virtual void changeLocalPlayer(Player* p);
	virtual void keyReleaseEvent(QKeyEvent* e);

	void quitGame();

	/**
	 * Delete an existing @ref BosonMap object and create a new one. You
	 * will have to call @ref BosonMap::loadMap before using it!
	 **/
	void recreateMap();

	void recreateLayout(int chatFramePos);

	void addBigDisplay();
	void addMiniMap();

	void addMouseIO(BosonBigDisplay*);

protected slots:
	void slotPlayerJoinedGame(KPlayer* p);
	void slotArrowScrollChanged(int speed);
	void slotAddUnit(Unit* unit, int x, int y);
	void slotRemoveUnit(Unit* unit);
	void slotStartGame();

	void slotReceiveMap(const QByteArray& map);

	void slotAddComputerPlayer(Player*);

private:
	void init();
	void initChat();

private:
	class BosonWidgetPrivate;
	BosonWidgetPrivate* d;
};

#endif // __BOSONWIDGET_H__
