/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BOSONSTARTWIDGETBASE_H
#define BOSONSTARTWIDGETBASE_H

#include <qwidget.h>

class BosonPlayField;
class Player;
class KPlayer;

class BosonStartWidgetBase : public QWidget
{
	Q_OBJECT
public:
	BosonStartWidgetBase(QWidget* parent);
	virtual ~BosonStartWidgetBase();

	/**
	 * @return The identifier of the currently selected playfield.
	 **/
	const QString& playFieldIdentifier() const { return mMapId; }

	/**
	 * Send the new playfield identifier through network, so that all
	 * clients can display the new playfield.
	 *
	 * Note that this method can safely be called directly, as it cares
	 * about network (in contrary to @ref slotPlayFieldChanged).
	 **/
	void sendPlayFieldChanged(const QString& identifier);

public slots:
	/**
	 * Called when the player changes the current map/playfield. This will
	 * send the new map/playfield (identifier) through network, so that 
	 * the changed map is displayed in the widget of all clients.
	 * See also @ref sendPlayFieldChanged and @ef slotPlayFieldChanged
	 *
	 * Note that you can safely call this directly. It cares about network.
	 **/
	virtual void slotSendPlayFieldChanged(int index);

	/**
	 * Called when user clicks on "Start game" button (see also @ref
	 * slotStartGameClicked)
	 * This widget should then be hidden and game should be started. Must be
	 * implemented by derived classes.
	 *
	 * You should call @ref sendNewGame at the end of your implementation.
	 **/
	virtual void slotStart() = 0;

	/**
	 * Should be used to start the game. Note that this will send a message
	 * indicating that start game was clicked and once it arrives again @ref
	 * slotStart will be called.
	 **/
	virtual void slotStartGameClicked();

signals:
	/**
	 * Emitted when the player clicks on cancel. The widget should get
	 * hidden now. (back to welcome widget)
	 **/
	void signalCancelled();

protected:
	virtual void setCurrentPlayField(BosonPlayField* field) = 0;

	/**
	 * Start a new game. This should send a message out, so that on the game
	 * starts on all clients (do this for the editor, too!). Derived
	 * classes also might do some final changes, e.g. apply all changes to
	 * the textfields in the widget (e.g. player name).
	 **/
	virtual void sendNewGame() = 0;

protected slots:
	/**
	 * Called when the ADMIN changes the map. See @ref
	 * Boson::signalPlayFieldChanged.
	 *
	 * Note that this gets called after a <em>network</em> message! Do
	 * <em>not</em> call this directly!
	 **/
	void slotPlayFieldChanged(const QString& playFieldIdentifier);

private:
	void initKGame();
	void initPlayFields();

private:
	QString mMapId;
};

#endif

