/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef __KGAMEDIALOGBOSONCONFIG_H__
#define __KGAMEDIALOGBOSONCONFIG_H__

#include <kgame/kgamedialogconfig.h>

class Player;

/**
 * This is the main widget of the newgame dialog. Here the user can choose a
 * map, a scenario, a network or computer players and even his species.
 *
 * Please note that this is quite a complex class which must interact very much
 * with @ref Boson! We need to ensure that as soon as one player changed a value
 * in the widget it is transferred over network to <em>all</em> players!
 * For example if a player changes the map the new map MUST be transferred to
 * all players so that they know the map was changed.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGameDialogBosonConfig : public KGameDialogGeneralConfig
{
	Q_OBJECT
public:
	KGameDialogBosonConfig(QWidget* parent = 0);
	virtual ~KGameDialogBosonConfig();

signals:
	void signalStartGame();

	void signalPlayFieldChanged(const QString& fileName);
	
	void signalSpeciesChanged(const QString& species);
	void signalTeamColorChanged(const QColor& color);

public slots:
	void slotPlayFieldChanged(int index);

	void slotSpeciesChanged(Player*);
	void slotTeamColorChanged(Player*);

	/**
	 * When this slot is called a remote player has selected another
	 * playField. We should display it now.
	 * @param playField name/identifier of the newly selected playField.
	 **/
	void slotPlayFieldChanged(const QString& playField);

protected slots:
	void slotStartGame();
	void slotSpeciesChanged(int index);
	void slotTeamColorChanged(int index);

protected:
	void regenerateColors();
	void addColor(const QColor& c);

private:
	class KGameDialogBosonConfigPrivate;
	KGameDialogBosonConfigPrivate* d;
};
#endif
