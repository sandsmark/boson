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

#ifndef BOSONSTARTGAMEWIDGET_H
#define BOSONSTARTGAMEWIDGET_H

#include <qwidget.h>

class BosonStartupNetwork;
class BosonPlayField;
class Player;
class KPlayer;
class KGamePropertyBase;

class BosonStartGameWidgetPrivate;
class BosonStartGameWidget : public QWidget
{
	Q_OBJECT
public:
	BosonStartGameWidget(BosonStartupNetwork* interface, QWidget* parent);
	~BosonStartGameWidget();

	/**
	 * See @ref BosonNewGameWidget::slotAddAIPlayer
	 **/
	void addAIPlayer();

public slots:
	/**
	 * Called when user clicks on "Start game" button. Sends a message to
	 * @ref Boson which emits a signal, which @ref slotStart is connected to
	 **/
	virtual void slotStartGameClicked();

	/**
	 * This is used to enable or disable some widgets when user's admin 
	 * status changes
	 **/
	void slotSetAdmin(bool);

signals:
	void signalShowNetworkOptions();
//	void signalSetLocalPlayer(Player* player);

	/**
	 * Emitted when the player clicks on cancel. The widget should get
	 * hidden now. (back to welcome widget)
	 **/
	void signalCancelled();

protected slots:
	void slotSetLocalPlayer(Player*);

protected:
	BosonStartupNetwork* networkInterface() const { return mNetworkInterface; }
private:
	BosonStartGameWidgetPrivate* d;
	BosonStartupNetwork* mNetworkInterface;
};

#endif

