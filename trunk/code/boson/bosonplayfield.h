/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONPLAYFIELD_H
#define BOSONPLAYFIELD_H

#include <qobject.h>

class BosonMap;
class BosonScenario;
class Boson;
class BPFFile;
class BPFDescription;

class KArchiveDirectory;
class KTar;
class KArchiveFile;
class QDomDocument;
class QDomElement;
class QDataStream;
template<class T> class QDict;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonPlayField : public QObject
{
	Q_OBJECT
public:
	BosonPlayField(QObject* parent = 0);
	~BosonPlayField();

	/**
	 * Preload all playFields into memory. Note that this will consume pretty much
	 * memory, so once a map is started all other maps should be cleared
	 * again.
	 **/
	static void preLoadAllPlayFields();

	/**
	 * Clear all preloaded playfields (see @ref preLoadAllPlayFields)
	 **/
	static void clearAllPreLoadedPlayFields();

	static BosonPlayField* playField(const QString& identifier);

	/**
	 * Load all data from the playField. Needs to be done only once - when
	 * the map gets started. You may want to use @ref preLoadPlayField
	 * instead, which loads the important data only.
	 *
	 * @ref preLoadPlayField is sufficient for displaying description and
	 * winning conditions and so on of a game. You need to load it
	 * completely for starting a playField only.
	 * @param identifier The filename without suffix. As returned by @ref
	 * availablePlayFields.
	 *
	 * @param file The fileName of the playField. Can be QString::null if
	 * @ref preLoadPlayField has been called already, i.e. @ref isPreLoaded
	 * is true.
	 **/
	bool loadPlayField(const QString& file);

	bool importHeightMapImage(const QImage& image);

	/**
	 * Load the important data (description for example) from the playField.
	 * Use @ref loadPlayField to load <em>all</em> data. preLoadPlayField is
	 * much faster than @ref loadPlayField.
	 **/
	bool preLoadPlayField(const QString& identifier);

	/**
	 * Remember to call @ref applyScenario first! The @ref
	 * BosonMap is updated automatically whenever a cell is added, but the
	 * unit list is not.
	 **/
	bool savePlayField(const QString& fileName);

	void applyScenario(Boson* boson);

	/**
	 * @return The default playfield which is meant to be used if the player
	 * didn't select a playfield at all
	 **/
	static QString defaultPlayField();

	/**
	 * @return The identifiers of all playfields that are currently
	 * available.
	 **/
	static QStringList availablePlayFields();

	/**
	 * @return The i18n'ed name of the specified playfield
	 **/
	static QString playFieldName(const QString& identifier);
	static QString playFieldComment(const QString& identifier);

	/**
	 * This simply deletes the map. This may be useful for game starting, as
	 * the ADMIN already has the map, but all other players don't. So the
	 * ADMIN might send the map over network and delete his local copy, so
	 * that it is ensured the entire loading code is the same on all
	 * clients, i.e. without a map until it is received.
	 **/
	void deleteMap();
	
	bool savePlayFieldForRemote(QDataStream& stream);
	bool loadPlayFieldFromRemote(QDataStream& stream);

	BosonMap* map() const { return mMap; }
	BosonScenario* scenario() const { return mScenario; }
	BPFDescription* description() const { return mDescription; }
	const QString& identifier() const { return mIdentifier; }

	void quit();

	bool modified() const;

	/**
	 * Make @p map the new map of this playfield. Called by the editor
	 * only.
	 **/
	void changeMap(BosonMap* map);

	/**
	 * Make @p s the new scenario of this playfield. Called by the editor
	 * only.
	 **/
	void changeScenario(BosonScenario* s);

	QString playFieldName() const;
	QString playFieldComment() const;

	/**
	 * @return Whether the the playfield has been completely loaded. This is
	 * the case when @ref loadPlayField has been called (<em>not</em> @ref
	 * preLoadPlayField)
	 **/
	bool isLoaded() const { return mLoaded; }

	/**
	 * @return Whether the the playfield has been pre-loaded. This is
	 * the case when @ref preLoadPlayField has been called. Note that this
	 * happens in @ref loadPlayField as well.
	 **/
	bool isPreLoaded() const { return mPreLoaded; }

	/**
	 * This emulates that loading has been completed. Useful when we are
	 * creating new maps. Do <em>not</em> use this of you are not creating a
	 * new map!
	 **/
	void finalizeLoading();

signals:
	/**
	 * Emitted when the map changes. Note that this can even be 0!
	 **/
	void signalNewMap(BosonMap*);

protected:
	bool loadDescriptionFromFile(const QByteArray& xml);
	bool loadMapFromFile(const QByteArray& xml, const QByteArray& heightMapImage, const QByteArray& texMap);
	bool loadScenarioFromFile(const QByteArray& xml);

	QString saveDescriptionToFile();
	QByteArray saveMapToFile();
	QByteArray saveTexMapToFile();
	QString saveScenarioToFile();

	bool saveMap(QDataStream& stream);
	bool saveDescription(QDataStream& stream);

	/**
	 * Load a @ref BosonMap from stream. Create a new BosonMap object if it
	 * does not yet exist, otherwise load from stream but also compare the
	 * stream with our locally present @ref BosonMap. Note that there must
	 * be no difference as we should have been the one who sent the stream!!
	 *
	 * But if there is a difference by any reason the network stream will be
	 * used instead of our locally present one.
	 *
	 * @return FALSE if the format was invalid. Note that the map will be
	 * broken then!
	 **/
	bool loadMap(QDataStream& stream);
	bool loadDescription(QDataStream& stream);

private:
	static void initStatic();

private:
	BosonMap* mMap;
	BosonScenario* mScenario;
	BPFDescription* mDescription;
	QString mIdentifier; // AB: this is not yet fully implemented - e.g. it isn't changed when saving or changing the map. should be the filename (see BPFFile::identifier())
	bool mLoaded;
	bool mPreLoaded;
	BPFFile* mFile;

	static QDict<BosonPlayField>* mPlayFields;
};

#endif
