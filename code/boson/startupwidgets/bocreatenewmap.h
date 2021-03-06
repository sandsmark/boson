/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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

#ifndef BOCREATENEWMAP_H
#define BOCREATENEWMAP_H

#include <qstring.h>

class BosonGroundTheme;

/**
 * @short Used by the editor to create a @ref QByteArray containing the new map.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoCreateNewMap
{
public:
	BoCreateNewMap();
	~BoCreateNewMap();

	void setSize(unsigned int width, unsigned int height);
	void setGroundTheme(BosonGroundTheme* theme);
	void setGroundFilling(unsigned int f);
	void setName(const QString& name);
	void setPlayerCount(unsigned int c);

	/**
	 * Create a new map, according to the settings in the widget (such as
	 * width/height, groundtype, ...)
	 **/
	QByteArray createNewMap();

protected:
	QByteArray createNewLocalPlayerScript() const;
	QByteArray createNewAIScript(unsigned int playerId) const;
	QByteArray createEmptyEventListenerXML() const;

private:
	unsigned int mWidth;
	unsigned int mHeight;
	QString mMapName;
	BosonGroundTheme* mGroundTheme;
	unsigned int mGroundFilling;
	unsigned int mPlayerCount;
};

#endif

