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

/**
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
	void signalMapChanged(const QString& fileName);
	void signalScenarioChanged(const QString& fileName);
	void signalSpeciesChanged(const QString& species);

public slots:
	void slotMapChanged(int index);
	void slotMapChanged(const QString& mapIdentifier);
	void slotScenarioChanged(const QString& scenarioIdentifier);

protected slots:
	void slotStartGame();
	void slotScenarioChanged(int index);
	void slotSpeciesChanged(int index);

private:
	class KGameDialogBosonConfigPrivate;
	KGameDialogBosonConfigPrivate* d;
};
#endif
