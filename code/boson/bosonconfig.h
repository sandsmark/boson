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
#ifndef __BOSONCONFIG_H__
#define __BOSONCONFIG_H__

#include <qstring.h>

class KConfig;

#define boConfig BosonConfig::bosonConfig()

/**
 * Boson has two different types of config entries, you can find both of them in
 * BosonConfig.
 * 
 * The first type is stored in this class - you can read it with the usual KDE
 * get functions (like @ref sound and @ref music). These entries can be saved to
 * the config file using @ref save. As the value is stored in BosonConfig you
 * need an object to access them. You probably want to use @ref bosonConfig, or
 * just boConfig (which is a #define just like kapp is a #define for @ref
 * KApplication::kApplication).
 *
 * The second type is not stored in BosonConfig. You can read the initial value
 * using BosonConfig::readFooBar() and finally save the value using
 * BosonConfig::saveFooBar(). You should not need to access these values all
 * over Boson but just at a single point. Examples for this type of config
 * entries are @ref readChatFramePosition and @ref readGameSpeed. You don't need
 * a BosonConfig object for this as the read/save methods are static.
 * @short Global configuration class for Boson
 *
 * All values which are stored in BosonConfig can be read from the config file
 * using @ref reset and stored using @ref save. Note that @reset is called on
 * construction but @ref save is <em>not</em> called on destruction!
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonConfig
{
public:
	BosonConfig(KConfig* conf = 0);
	~BosonConfig();

	static BosonConfig* bosonConfig() { return mBosonConfig; }
	
	static void initBosonConfig();

	static QString readLocalPlayerName(KConfig* conf = 0);
	static void saveLocalPlayerName(const QString& name, KConfig* conf = 0);

	static void saveGameSpeed(int speed, KConfig* conf = 0);
	static int readGameSpeed(KConfig* conf = 0);

	static void saveCommandFramePosition(int pos, KConfig* conf = 0);
	static int readCommandFramePosition(KConfig* conf = 0);

	static void saveChatFramePosition(int pos, KConfig* conf = 0);
	static int readChatFramePosition(KConfig* conf = 0);

	bool sound() const;
	bool music() const;

	void save(KConfig* conf = 0);
	void reset(KConfig* conf = 0);

protected:
	bool readSound(KConfig* conf);
	void saveSound(bool sound, KConfig* conf);

	bool readMusic(KConfig* conf);
	void saveMusic(bool sound, KConfig* conf);

private:
	static BosonConfig* mBosonConfig;
	
	class BosonConfigPrivate;
	BosonConfigPrivate* d;
};

#endif
