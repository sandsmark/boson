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

#include "global.h"
#include <qstring.h>

class QColor;
class KConfig;
class BosonConfig;
template<class T> class QValueList;

#define boConfig BosonConfig::bosonConfig()


class BoConfigEntry
{
public:
	BoConfigEntry(BosonConfig* parent, const QString& key);
	virtual ~BoConfigEntry();

	/**
	 * Set the current group.
	 * By default just conf->setGroup("Boson")
	 *
	 * Replace this to create config entries e.g. for the editor only which
	 * go to a separate group.
	 **/
	virtual void activate(KConfig* conf);

	virtual void save(KConfig* conf) = 0;
	virtual void load(KConfig* conf) = 0;

	const QString& key() const { return mKey; }

private:
	QString mKey;
};

//AB: we can't use a template because of KConfig::readEntry()
class BoConfigBoolEntry : public BoConfigEntry
{
public:
	BoConfigBoolEntry(BosonConfig* parent, const QString& key, bool defaultValue);
	virtual ~BoConfigBoolEntry() {}

	bool value() const { return mValue; }
	void setValue(bool v) { mValue = v; }

	virtual void save(KConfig* conf);
	virtual void load(KConfig* conf);

private:
	bool mValue;
};

class BoConfigIntEntry : public BoConfigEntry
{
public:
	BoConfigIntEntry(BosonConfig* parent, const QString& key, int defaultValue);
	virtual ~BoConfigIntEntry() {}

	unsigned int value() const { return mValue; }
	void setValue(int v) { mValue = v; }

	virtual void save(KConfig* conf);
	virtual void load(KConfig* conf);

private:
	int mValue;
};


class BoConfigUIntEntry : public BoConfigEntry
{
public:
	BoConfigUIntEntry(BosonConfig* parent, const QString& key, unsigned int defaultValue);
	virtual ~BoConfigUIntEntry() {}

	unsigned int value() const { return mValue; }
	void setValue(unsigned int v) { mValue = v; }

	virtual void save(KConfig* conf);
	virtual void load(KConfig* conf);

private:
	unsigned int mValue;
};

class BoConfigDoubleEntry : public BoConfigEntry
{
public:
	BoConfigDoubleEntry(BosonConfig* parent, const QString& key, double defaultValue);
	virtual ~BoConfigDoubleEntry() {}

	double value() const { return mValue; }
	void setValue(double v) { mValue = v; }

	virtual void save(KConfig* conf);
	virtual void load(KConfig* conf);

private:
	double mValue;
};

class BoConfigStringEntry : public BoConfigEntry
{
public:
	BoConfigStringEntry(BosonConfig* parent, const QString& key, QString defaultValue);
	virtual ~BoConfigStringEntry() {}

	const QString& value() const { return mValue; }
	void setValue(const QString& v) { mValue = v; }

	virtual void save(KConfig* conf);
	virtual void load(KConfig* conf);

private:
	QString mValue;
};

class BoConfigIntListEntry; // forwarding, since i dont want to #include <qvaluelist.h>

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
 * using @ref reset and stored using @ref save. Note that @ref reset is called on
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

	static QString readComputerPlayerName(KConfig* conf = 0);
	static void saveComputerPlayerName(const QString& name, KConfig* conf = 0);

	static QColor readLocalPlayerColor(KConfig* conf = 0);
	static void saveLocalPlayerColor(const QColor& color, KConfig* conf = 0);

	static QString readLocalPlayerMap(KConfig* conf = 0);
	static void saveLocalPlayerMap(const QString& id, KConfig* conf = 0);

	static void saveGameSpeed(int speed, KConfig* conf = 0);
	static int readGameSpeed(KConfig* conf = 0);

	static void saveCursorMode(CursorMode mode, KConfig* conf = 0);
	static CursorMode readCursorMode(KConfig* conf = 0);

	static void saveCursorDir(const QString& dir, KConfig* conf = 0);
	static QString readCursorDir(KConfig* conf = 0);

	void setSound(bool s) { mSound->setValue(s); }
	bool sound() const { return mSound->value(); }
	void setMusic(bool m) { mMusic->setValue(m); }
	bool music() const { return mMusic->value(); }
	void setRMBMove(bool m) { mRMBMove->setValue(m); }
	bool rmbMove() const { return mRMBMove->value(); }
	void setMMBMove(bool m) { mMMBMove->setValue(m); }
	bool mmbMove() const { return mMMBMove->value(); }
	void setShowMenubarInGame(bool s) { mShowMenubarInGame->setValue(s); }
	bool showMenubarInGame() const { return mShowMenubarInGame->value(); }
	void setShowMenubarOnStartup(bool s) { mShowMenubarOnStartup->setValue(s); }
	bool showMenubarOnStartup() const { return mShowMenubarOnStartup->value(); }
	void setArrowKeyStep(unsigned int k) { mArrowKeyStep->setValue(k); }
	unsigned int arrowKeyStep() const { return mArrowKeyStep->value(); }
	void setUpdateInterval(unsigned int i) { mUpdateInterval->setValue(i); }
	unsigned int updateInterval() const { return mUpdateInterval->value(); }
	void setMiniMapScale(double s) { mMiniMapScale->setValue(s); }
	double miniMapScale() const { return mMiniMapScale->value(); }
	void setMiniMapZoom(double z) { mMiniMapZoom->setValue(z); }
	double miniMapZoom() const { return mMiniMapZoom->value(); }
	void setCommandButtonsPerRow(int b) { mCommandButtonsPerRow->setValue(b); }
	int commandButtonsPerRow() const { return mCommandButtonsPerRow->value(); }
	void setChatScreenRemoveTime(unsigned int s) { mChatScreenRemoveTime->setValue(s); }
	unsigned int chatScreenRemoveTime() const { return mChatScreenRemoveTime->value(); }
	void setChatScreenMaxItems(int max) { mChatScreenMaxItems->setValue(max); }
	int chatScreenMaxItems() const { return mChatScreenMaxItems->value(); }
	void setModelTexturesMipmaps(bool enable) { mModelTexturesMipmaps->setValue(enable); }
	bool modelTexturesMipmaps() const { return mModelTexturesMipmaps->value(); }
	int magnificationFilter() const { return mMagnificationFilter->value(); }
	void setMagnificationFilter(int f) { mMagnificationFilter->setValue(f); }
	int minificationFilter() const { return mMinificationFilter->value(); }
	void setMinificationFilter(int f) { mMinificationFilter->setValue(f); }
	int mipmapMinificationFilter() const { return mMipmapMinificationFilter->value(); }
	void setMipmapMinificationFilter(int f) { mMipmapMinificationFilter->setValue(f); }
	void setAlignSelectionBoxes(bool enable) { mAlignSelectionBoxes->setValue(enable); }
	bool alignSelectionBoxes() const { return mAlignSelectionBoxes->value(); }

	void setUnitSoundActivated(UnitSoundEvent e, bool activated);
	bool unitSoundActivated(UnitSoundEvent e) const;

	/**
	 * @param m How "sensitive" the edge is. I.e. the number the cursor must
	 * be in range of m pixels to an edge of the window. 0 to disable
	 **/
	void setCursorEdgeSensity(unsigned int m) { mCursorEdgeSensity->setValue(m); }
	unsigned int cursorEdgeSensity() const { return mCursorEdgeSensity->value(); }

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

	void addConfigEntry(BoConfigEntry*);

	/**
	 * Loads list of unsinged long int's from KConfig (which only supports loading
	 * list of _int's_)
	 **/
	static QValueList<unsigned long int> readUnsignedLongNumList(KConfig* cfg, QString key);

	/**
	 * Loads list of float's from KConfig
	 **/
	static QValueList<float> readFloatNumList(KConfig* cfg, QString key);

	static void writeUnsignedLongNumList(KConfig* cfg, QString key, QValueList<unsigned long int> list);


protected:
	int readCommandButtonsPerRow(KConfig* conf);
	void saveCommandButtonsPerRow(KConfig* conf);

private:
	static BosonConfig* mBosonConfig;
	
	class BosonConfigPrivate;
	BosonConfigPrivate* d;

	// note that ALL BoConfigEntry objects must be new'ed in the
	// BosonConfig c'tor !
	BoConfigBoolEntry* mSound;
	BoConfigBoolEntry* mMusic;
	BoConfigBoolEntry* mMMBMove;
	BoConfigBoolEntry* mRMBMove;
	BoConfigBoolEntry* mShowMenubarInGame;
	BoConfigBoolEntry* mShowMenubarOnStartup;
	BoConfigBoolEntry* mModelTexturesMipmaps;
	BoConfigIntEntry* mCommandButtonsPerRow;
	BoConfigIntEntry* mChatScreenMaxItems;
	BoConfigUIntEntry* mChatScreenRemoveTime;
	BoConfigUIntEntry* mArrowKeyStep;
	BoConfigUIntEntry* mCursorEdgeSensity;
	BoConfigUIntEntry* mUpdateInterval;
	BoConfigDoubleEntry* mMiniMapScale;
	BoConfigDoubleEntry* mMiniMapZoom;
	BoConfigIntListEntry* mUnitSoundsDeactivated;
	BoConfigIntEntry* mMagnificationFilter;
	BoConfigIntEntry* mMinificationFilter;
	BoConfigIntEntry* mMipmapMinificationFilter;
	BoConfigBoolEntry* mAlignSelectionBoxes;

	// NOT stored to config file!
	bool mDisableSound;
	DebugMode mDebugMode;
};

#endif
