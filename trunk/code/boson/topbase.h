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
#ifndef TOPBASE_H
#define TOPBASE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <kmainwindow.h>
#include <ktoolbar.h>

class KPlayer;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonCommandBar : public KToolBar
{
	Q_OBJECT
public:
	BosonCommandBar(QMainWindow* parent, QMainWindow::ToolBarDock dock);
	virtual ~BosonCommandBar();

protected:
	virtual void mousePressEvent(QMouseEvent* e);
	
signals:
	void signalButtonsPerRow(int);
	
private:
	enum {
		CONTEXT_LEFT = 0,
		CONTEXT_RIGHT = 1,
		CONTEXT_FLAT = 2,
		CONTEXT_BUTTONS_ROW = 10
	};

private:
	KPopupMenu* contextMenu();


private:
	KPopupMenu* mContext;
	
};

class BosonWidget;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class TopBase : public KMainWindow
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 **/
	TopBase();

	/**
	 * Default Destructor
	 **/
	virtual ~TopBase();

protected:
	/**
	 * This function is called when it is time for the app to save its
	 * properties for session management purposes.
	 **/
	void saveProperties(KConfig *);

	/**
	 * This function is called when this app is restored.  The KConfig
	 * object points to the session management config file that was saved
	 * with @ref saveProperties
	 **/
	void readProperties(KConfig *);

	BosonWidget* bosonWidget() const { return mBosonWidget; }
	QFrame* commandFrame() const;

	/**
	 * Initialize the command frame - currently implemented as a @ref
	 * KToolBar. Note that this needs to be called after @ref initKAction
	 **/
	void initCommandFrame();

protected slots:
	virtual void slotPlayerJoinedGame(KPlayer*);
	virtual void slotPlayerLeftGame(KPlayer*);
	void slotConfigureKeys();

private slots:
	void slotZoom(int index);

	void slotShowToolbar();
	void slotShowStatusbar();
	void slotShowChat();
	void slotConfigureToolbars();
	void slotMoveCommandFrame(int pos);
	void slotDebugMode(int);

	void slotDebugPlayer(int);

private:
	void initStatusBar();

	/**
	 * Initialize th KActions which are shared by both, editor and game.
	 **/
	void initKAction();

private:
	class TopBasePrivate;
	TopBasePrivate* d;

	BosonWidget* mBosonWidget;
};

#endif
