/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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
#ifndef BOSONMAINWIDGET_H
#define BOSONMAINWIDGET_H

#include "defines.h"
#include "bo3dtools.h"

#include "bosonufoglwidget.h"

class BosonCanvas;
class BosonCursor;
class Player;
class PlayerIO;
class Unit;
class UnitProperties;
class BoItemList;
class BosonItem;
class BoPixmapRenderer;
class BoLight;
class BoFontInfo;
class BosonScript;
class BoVisibleEffects;
class BosonMap;
class BosonEffect;
class BoSpecificAction;
class BoGLMatrices;
class BoRenderItem;
class BosonCursor;
class BoUfoWidget;
class BosonGameEngine;
class Boson;

class KGameChat;
class KGameIO;
class QDomElement;
template<class T> class QPtrList;
template<class T> class QValueVector;
class KCmdLineArgs;

class BosonMainWidgetPrivate;


/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonMainWidget : public BosonUfoGLWidget
{
	Q_OBJECT
public:
	BosonMainWidget(QWidget* parent, bool wantDirect = true);
	virtual ~BosonMainWidget();

	/**
	 * Must be called exactly once right after the c'tor
	 **/
	void initUfoGUI();

	void setGameEngine(BosonGameEngine*);


	/**
	 * Grab a frame for a movie. The returned @ref QByteArray contains
	 * everything that is necessary to display one frame. At the moment that
	 * is the whole screenshot, later we may use the positions of the units
	 * only or something similar.
	 **/
	QByteArray grabMovieFrame();

public slots:
	/**
	 * This extends the original @ref BosonGLWidget::slotUpdateGL by an FPS
	 * counter.
	 **/
	virtual void slotUpdateGL();

	/**
	 * Called when user clicks "start new game" button
	 * This shows BosonStartGameWidget from where you can start new game
	 **/
	void slotShowNewGamePage(KCmdLineArgs* args = 0);

	void slotShowStartEditorPage(KCmdLineArgs* args = 0);
	void slotShowLoadGamePage(KCmdLineArgs* args = 0);
	void slotLoadFromLog(const QString& logFile);

	void slotAddLocalPlayer();
	void slotResetGame();

	/**
	 * Starts a new game. Called when user clicks "Start game" button in
	 * BosonStartGameWidget
	 **/
	void slotStartNewGame();

	/**
	 * Ends current game and reinits all game data, so that a new game can
	 * be started.
	 * */
	void slotEndGame();

	void slotGameOver();

	void slotSetUpdateInterval(unsigned int ms);


protected:
	virtual void initializeGL();
	virtual void resizeGL(int w, int h);
	virtual void paintGL();

	void renderUfo();

	void grabMovieFrameAndSave();

	/**
	 * End the game. All relevant classes are deleted. You
	 * may want to call @ref reinitGame after @ref endGame so that you can
	 * start a new game now.
	 **/
	void endGame();

	/**
	 * Initialize all classes and member vars so that a new game can be
	 * started. Also show the welcome widget.
	 *
	 * Note that you <em>must</em> call @ref endGame before - otherwise
	 * you'll experience a <em>big</em> memory hole (and probably a lot of
	 * instability).
	 *
	 * You may want to call @ref slotGameOver instead, which calls both @ref
	 * endGame and @ref reinitGame
	 **/
	void reinitGame();

	bool changeLocalPlayer(Player* p);
	void saveConfig();

	void raiseWidget(BoUfoWidget*);

protected slots:
	void slotBosonObjectAboutToBeDestroyed(Boson*);
	void slotChangeLocalPlayer(Player* p);

	/**
	 * Cancenl the load/save widget and return to the game (if running), or
	 * the welcome widget.
	 **/
	void slotCancelLoadSave();

	void slotQuickloadGame();
	void slotQuicksaveGame();

	void slotShowSaveGamePage();

	void slotGameStarted();
	void slotStartingFailed();


	void slotEditorNewMap(const QByteArray&);

	void slotSaveExternalStuffAsXML(QDomElement& root);
	void slotLoadExternalStuffFromXML(const QDomElement& root);
	void slotAddChatSystemMessage(const QString& fromName, const QString& text, const Player* forPlayer);

	void slotSkipFrame();

	/**
	 * See @ref BosonCursorCollection::signalSetWidgetCursor
	 **/
	void slotSetWidgetCursor(BosonCursor* c);

	void slotPreferences();
	void slotPreferencesApply();
	void slotDebugUfoWidgets();
	void slotDebugTextures();
	void slotDebugModels();

	void slotStartupPreferredSizeChanged();


private:
	void init();

	void initBoson();
	void deleteBoson();

private:
	BosonMainWidgetPrivate* d;

};

#endif

