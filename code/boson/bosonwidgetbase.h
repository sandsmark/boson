/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONWIDGETBASE_H
#define BOSONWIDGETBASE_H

#include <kxmlguiclient.h>

#include <qwidget.h>

class KPlayer;
class KGamePropertyBase;
class KDockWidget;
class QDataStream;
class QDomElement;

class BosonCursor;
class BosonBigDisplay;
class BosonBigDisplayBase;
class Unit;
class Player;
class BoDisplayManager;
class Boson;
class BosonPlayField;
class OptionsDialog;
class BosonLocalPlayerInput;
class BosonItem;
class PlayerIO;

/**
 * This is the actual main widget of boson for the game
 *
 * [obsolete docs got deleted]
 *
 * All game specific stuff should be done in other classes - e.g. visual stuff
 * (click on a unit) in @ref BosonBigDisplay, constructing in @ref
 * BosonCommandFrame and so on. These classes should emit signals which get
 * connected by BosonWidgetBase to the necessary slots - probably mainly to @ref
 * Boson.
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 */
class BosonWidgetBase : public QWidget, virtual public KXMLGUIClient
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 **/
	BosonWidgetBase(QWidget* parent);

	/**
	 * Default Destructor
	 **/
	virtual ~BosonWidgetBase();


	/**
	 * Set the displaymanager. The displaymanager will be reparened to this
	 * widget, but ownership is <em>NOT</em> taken.
	 *
	 * I repeat: ownership is <em>NOT</em> taken! This means you MUST delete
	 * the displaymanager manually, Qt will NOT delete this, as we took
	 * ownership but don't delete here!
	 **/
	void setDisplayManager(BoDisplayManager* displayManager);

	inline BoDisplayManager* displayManager() const { return mDisplayManager; }

	/**
	 * @param playFieldId See @ref Top::slotStartGame
	 **/
	void initGameMode();

private:
	BoDisplayManager* mDisplayManager;
};

#endif
