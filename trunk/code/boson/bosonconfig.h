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
#ifndef BOSONCONFIG_H
#define BOSONCONFIG_H

#include <qstring.h>

#include "global.h"

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

	enum DebugMode {
		DebugNormal = 0, // no debugging
		DebugSelection = 1
	};

	static BosonConfig* bosonConfig() { return mBosonConfig; }
	
	static void initBosonConfig();

	static QString readLocalPlayerName(KConfig* conf = 0);
	static void saveLocalPlayerName(const QString& name, KConfig* conf = 0);

	static void saveGameSpeed(int speed, KConfig* conf = 0);
	static int readGameSpeed(KConfig* conf = 0);

	static void saveCommandFramePosition(CommandFramePosition pos, KConfig* conf = 0);
	static CommandFramePosition readCommandFramePosition(KConfig* conf = 0);

	static void saveChatFramePosition(ChatFramePosition pos, KConfig* conf = 0);
	static ChatFramePosition readChatFramePosition(KConfig* conf = 0);

	static void saveCursorMode(CursorMode mode, KConfig* conf = 0);
	static CursorMode readCursorMode(KConfig* conf = 0);

	static void saveCursorDir(const QString& dir, KConfig* conf = 0);
	static QString readCursorDir(KConfig* conf = 0);

	static void saveGroupMoveMode(GroupMoveMode mode, KConfig* conf = 0);
	static GroupMoveMode readGroupMoveMode(KConfig* conf = 0);

	void setSound(bool s);
	bool sound() const;
	void setMusic(bool m);
	bool music() const;
	void setCommandButtonsPerRow(int b);
	int commandButtonsPerRow() const;
	void setArrowKeyStep(unsigned int k);
	unsigned int arrowKeyStep() const;

	DebugMode debugMode() const;
	/**
	 * Change the debugging mode. Note that this isn't saved to the config,
	 * so this is lost on quit.
	 **/
	void setDebugMode(DebugMode m);

	/**
	 * Disable sound loading and playing. Note that this value is 
	 * <em>not</em> saved into the config file (it is a command line arg)!
	 **/
	void setDisableSound(bool d) { mDisableSound = d; }

	/**
	 * FALSE (default) if sound files should be loaded normally, otherwise
	 * TRUE.
	 **/
	bool disableSound() const { return mDisableSound; }

	void save(bool editor = false, KConfig* conf = 0);
	void reset(KConfig* conf = 0);

protected:
	bool readSound(KConfig* conf);
	void saveSound(KConfig* conf);

	bool readMusic(KConfig* conf);
	void saveMusic(KConfig* conf);

	int readCommandButtonsPerRow(KConfig* conf);
	void saveCommandButtonsPerRow(KConfig* conf);

	unsigned int readArrowKeyStep(KConfig* conf);
	void saveArrowKeyStep(KConfig* conf);

private:
	static BosonConfig* mBosonConfig;
	
	class BosonConfigPrivate;
	BosonConfigPrivate* d;

	bool mSound;
	bool mMusic;
	bool mDisableSound;
};

#endif
