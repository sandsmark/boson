/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
class bofixed;
template<class T> class QValueList;
template<class T> class BoVector2;
template<class T> class BoVector3;
template<class T> class BoVector4;
typedef BoVector2<float> BoVector2Float;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoVector3<float> BoVector3Float;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoVector4<float> BoVector4Float;
typedef BoVector4<bofixed> BoVector4Fixed;

#define boConfig BosonConfig::bosonConfig()

class BoConfigEntry
{
public:
	enum Type {
		Bool = 0,
		Int = 1,
		UInt = 2,
		Double = 3,
		String = 4,
		IntList = 5,
		Color = 6
	};
public:
	/**
	 * @param saveConfig If TRUE (the default) this value will be saved by
	 * @ref BosonConfig::save to the config file. If it is FALSE, the value
	 * will not be saved (may be useful for debugging options).
	 **/
	BoConfigEntry(BosonConfig* parent, const QString& key, bool saveConfig = true);
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

	virtual int type() const = 0;

	const QString& key() const { return mKey; }

	bool saveConfig() const
	{
		return mSaveConfig;
	}

private:
	QString mKey;
	bool mSaveConfig;
};

//AB: we can't use a template because of KConfig::readEntry()
class BoConfigBoolEntry : public BoConfigEntry
{
public:
	BoConfigBoolEntry(BosonConfig* parent, const QString& key, bool defaultValue, bool saveConfig = true);
	virtual ~BoConfigBoolEntry() {}

	bool value() const { return mValue; }
	void setValue(bool v) { mValue = v; }

	virtual void save(KConfig* conf);
	virtual void load(KConfig* conf);

	virtual int type() const { return Bool; }

private:
	bool mValue;
};

class BoConfigIntEntry : public BoConfigEntry
{
public:
	BoConfigIntEntry(BosonConfig* parent, const QString& key, int defaultValue, bool saveConfig = true);
	virtual ~BoConfigIntEntry() {}

	unsigned int value() const { return mValue; }
	void setValue(int v) { mValue = v; }

	virtual void save(KConfig* conf);
	virtual void load(KConfig* conf);

	virtual int type() const { return Int; }

private:
	int mValue;
};

class BoConfigUIntEntry : public BoConfigEntry
{
public:
	BoConfigUIntEntry(BosonConfig* parent, const QString& key, unsigned int defaultValue, bool saveConfig = true);
	virtual ~BoConfigUIntEntry() {}

	unsigned int value() const { return mValue; }
	void setValue(unsigned int v) { mValue = v; }

	virtual void save(KConfig* conf);
	virtual void load(KConfig* conf);

	virtual int type() const { return UInt; }

private:
	unsigned int mValue;
};

class BoConfigDoubleEntry : public BoConfigEntry
{
public:
	BoConfigDoubleEntry(BosonConfig* parent, const QString& key, double defaultValue, bool saveConfig = true);
	virtual ~BoConfigDoubleEntry() {}

	double value() const { return mValue; }
	void setValue(double v) { mValue = v; }

	virtual void save(KConfig* conf);
	virtual void load(KConfig* conf);

	virtual int type() const { return Double; }

private:
	double mValue;
};

class BoConfigStringEntry : public BoConfigEntry
{
public:
	BoConfigStringEntry(BosonConfig* parent, const QString& key, QString defaultValue, bool saveConfig = true);
	virtual ~BoConfigStringEntry() {}

	const QString& value() const { return mValue; }
	void setValue(const QString& v) { mValue = v; }

	virtual void save(KConfig* conf);
	virtual void load(KConfig* conf);

	virtual int type() const { return String; }

private:
	QString mValue;
};

class BoConfigColorEntry : public BoConfigEntry
{
public:
	BoConfigColorEntry(BosonConfig* parent, const QString& key, const QColor& defaultValue, bool saveConfig = true);
	virtual ~BoConfigColorEntry() {}

	QColor value() const;
	void setValue(unsigned int rgb ) { mRGBValue = rgb; }
	void setValue(const QColor& v);

	virtual void save(KConfig* conf);
	virtual void load(KConfig* conf);

	virtual int type() const { return Color; }

private:
	unsigned int mRGBValue;
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

	static BosonConfig* bosonConfig();

	/**
	 * Set a function that will be called immediately after @ref
	 * BoGlobalObjectBase::loadObject() has returned successfully.
	 *
	 * You can use this to do some initialization for other global objects,
	 * that will get constructed after the BosonConfig object, such as
	 * applying the --nosound parameter (will get used by @ref BosonMusic
	 * constructor)
	 **/
	static void setPostInitFunction(void (*func)());

	/**
	 * Read and return a @ref BoVector3. Will return @p aDefault if @p key
	 * is not found.
	 **/
	static BoVector3Float readBoVector3FloatEntry(const KConfig* cfg, const QString& key, const BoVector3Float& aDefault);
	static BoVector3Fixed readBoVector3FixedEntry(const KConfig* cfg, const QString& key, const BoVector3Fixed& aDefault);

	/**
	 * @overload
	 *
	 * Just the same as the above one, but it uses BoVector3(0, 0, 0) as
	 * default. We don't use a default value in the parameter, as we would
	 * have to include bo3dtools.h for that.
	 **/
	static BoVector3Float readBoVector3FloatEntry(const KConfig* cfg, const QString& key);
	static BoVector3Fixed readBoVector3FixedEntry(const KConfig* cfg, const QString& key);

	static BoVector4Float readBoVector4FloatEntry(const KConfig* cfg, const QString& key, const BoVector4Float& aDefault);
	static BoVector4Float readBoVector4FloatEntry(const KConfig* cfg, const QString& key);
	static BoVector4Fixed readBoVector4FixedEntry(const KConfig* cfg, const QString& key, const BoVector4Fixed& aDefault);
	static BoVector4Fixed readBoVector4FixedEntry(const KConfig* cfg, const QString& key);

	static void writeEntry(KConfig* cfg, const QString& key, const BoVector3Float& value);
	static void writeEntry(KConfig* cfg, const QString& key, const BoVector3Fixed& value);
	static void writeEntry(KConfig* cfg, const QString& key, const BoVector4Float& value);
	static void writeEntry(KConfig* cfg, const QString& key, const BoVector4Fixed& value);


	static QString readLocalPlayerName(KConfig* conf = 0);
	static void saveLocalPlayerName(const QString& name, KConfig* conf = 0);

	static QString readComputerPlayerName(KConfig* conf = 0);
	static void saveComputerPlayerName(const QString& name, KConfig* conf = 0);

	static QColor readLocalPlayerColor(KConfig* conf = 0);
	static void saveLocalPlayerColor(const QColor& color, KConfig* conf = 0);

	static QString readLocalPlayerMap(KConfig* conf = 0);
	static void saveLocalPlayerMap(const QString& id, KConfig* conf = 0);

	static QString readEditorMap(KConfig* conf = 0);
	static void saveEditorMap(const QString& id, KConfig* conf = 0);

	static bool readEditorCreateNewMap(KConfig* conf = 0);
	static void saveEditorCreateNewMap(bool createnew, KConfig* conf = 0);

	static void saveGameSpeed(int speed, KConfig* conf = 0);
	static int readGameSpeed(KConfig* conf = 0);

// below we have config options that are stored in this class (and saved in
// save())
public:
	void setUnitSoundActivated(UnitSoundEvent e, bool activated);
	bool unitSoundActivated(UnitSoundEvent e) const;


	/**
	 * Add a dynamic entry to BosonConfig. A dynamic entry does not have a
	 * dedicated set/get function, but rather uses @ref setBoolValue,
	 * @ref value and friends.
	 *
	 * You should not use this for extremely time critical config entries
	 * (time critical means it is used several dozen times per second at
	 * least), as at least one @ref QDict lookup is involved for nearly
	 * every operation.
	 *
	 * For all non-time critical config entries this can be very handy, as
	 * you don't need to modify bosonconfig.h (and therefore don't have to
	 * recompile everything).
	 * @param conf The @ref KConfig object that is looked into for whether
	 * the key already has a value.
	 **/
	void addDynamicEntry(BoConfigEntry* entry, KConfig* conf = 0);

	/**
	 * Convenience method for @ref addDynamicEntry
	 **/
	BoConfigBoolEntry* addDynamicEntryBool(const QString& configKey, bool defaultValue = false, bool saveConfig = true);
	BoConfigIntEntry* addDynamicEntryInt(const QString& configKey, int defaultValue = 0, bool saveConfig = true);
	BoConfigUIntEntry* addDynamicEntryUInt(const QString& configKey, unsigned int defaultValue = 0, bool saveConfig = true);
	BoConfigDoubleEntry* addDynamicEntryDouble(const QString& configKey, double defaultValue = 0.0, bool saveConfig = true);
	BoConfigStringEntry* addDynamicEntryString(const QString& configKey, QString defaultValue = QString::null, bool saveConfig = true);
	BoConfigColorEntry* addDynamicEntryColor(const QString& configKey, QColor defaultValue, bool saveConfig = true);
	BoConfigIntListEntry* addDynamicEntryIntList(const QString& configKey, const QValueList<int>& defaultValue, bool saveConfig = true);

	/**
	 * @return TRUE if a dynamic entry with @p key was added with @ref
	 * addDynamicEntry, otherwise FALSE.
	 **/
	bool hasKey(const QString& key) const;

	/**
	 * Set the value of the config entry @p key (see @ref value) to @p
	 * value. This operates on @ref BoConfigBoolEntry objects.
	 *
	 * Note that an overloaded "setValue()" method for this task is not
	 * provided, because while programming it is only a minor difference
	 * between setUIntValue() and setIntValue(), but as they resolve to two
	 * different classes in BosonConfig it is a major difference here. I
	 * believe this is not a problem for us.
	 **/
	void setBoolValue(const QString& key, bool value);

	/**
	 * Same as  @ref setBoolValue, but for @ref BoConfigIntEntry objects.
	 **/
	void setIntValue(const QString& key, int value);

	/**
	 * Same as  @ref setBoolValue, but for @ref BoConfigUIntEntry objects.
	 **/
	void setUIntValue(const QString& key, unsigned int value);

	/**
	 * Same as  @ref setBoolValue, but for @ref BoConfigDoubleEntry objects.
	 **/
	void setDoubleValue(const QString& key, double value);

	/**
	 * Same as  @ref setBoolValue, but for @ref BoConfigStringEntry objects.
	 **/
	void setStringValue(const QString& key, const QString& value);

	/**
	 * Same as  @ref setBoolValue, but for @ref BoConfigColorEntry objects.
	 **/
	void setColorValue(const QString& key, const QColor& value);

	/**
	 * Same as  @ref setBoolValue, but for @ref BoConfigIntListEntry objects.
	 **/
	void setIntListValue(const QString& key, const QValueList<int>& v);

	/**
	 * @return The (dynamic) entry for @p key, or NULL if no such key was
	 * ever added using @ref addDynamicEntry.
	 **/
	BoConfigEntry* value(const QString& key) const;

	/**
	 * @return The value of the entry @p key, or @p _default no entry of the
	 * correct type exists.
	 **/
	bool boolValue(const QString& key, bool _default = false) const;

	/**
	 * @return The value of the entry @p key, or @p _default no entry of the
	 * correct type exists.
	 **/
	int intValue(const QString& key, int _default = 0) const;

	/**
	 * @return The value of the entry @p key, or @p _default no entry of the
	 * correct type exists.
	 **/
	unsigned int uintValue(const QString& key, unsigned int _default = 0) const;

	/**
	 * @return The value of the entry @p key, or @p _default no entry of the
	 * correct type exists.
	 **/
	const QString& stringValue(const QString& key, const QString& _default = QString::null) const;

	/**
	 * @return The value of the entry @p key, or @p _default no entry of the
	 * correct type exists.
	 **/
	double doubleValue(const QString& key, double _default = 0.0) const;

	/**
	 * @return The value of the entry @p key, or @ref Qt::black if no entry
	 * of the correct type exists.
	 **/
	QColor colorValue(const QString& key) const;

	/**
	 * @return The value of the entry @p key, or @p _default if no entry of
	 * the correct type exists.
	 **/
	QColor colorValue(const QString& key, const QColor& _default) const;

	/**
	 * @return The value of the entry @p key, or an empty list if no entry
	 * of the correct type exists.
	 **/
	QValueList<int> intListValue(const QString& key) const;

	/**
	 * @return The value of the entry @p key, or @p _default if no entry of
	 * the correct type exists.
	 **/
	QValueList<int> intListValue(const QString& key, const QValueList<int>& _default) const;


public:
	void save(bool editor = false, KConfig* conf = 0);
	void reset(KConfig* conf = 0);

	void addConfigEntry(BoConfigEntry*);

	/**
	 * Loads list of unsinged long int's from KConfig (which only supports loading
	 * list of _int's_)
	 **/
	static QValueList<unsigned long int> readUnsignedLongNumList(const KConfig* cfg, const QString key);
	static void writeUnsignedLongNumList(KConfig* cfg, const QString key, QValueList<unsigned long int> list);

	/**
	 * Loads list of float's from KConfig
	 **/
	static QValueList<float> readFloatNumList(const KConfig* cfg, const QString key);
	static void writeFloatNumList(QValueList<float> list, KConfig* cfg, const QString key);


private:
	class BosonConfigPrivate;
	BosonConfigPrivate* d;
};

#endif
