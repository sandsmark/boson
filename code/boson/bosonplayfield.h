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
class Boson;
class BPFFile;
class BPFDescription;

class KArchiveDirectory;
class KTar;
class KArchiveFile;
class QDomDocument;
class QDomElement;
class QDataStream;
template<class T, class T2> class QMap;

/**
 * CLass that provides information about the playfield, such as map size and
 * winning conditions.
 *
 * These information can be displayed on the startup widgets.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonPlayFieldInformation
{
public:
	BosonPlayFieldInformation();
	~BosonPlayFieldInformation();

	unsigned int mapWidth() const
	{
		return mMapWidth;
	}
	unsigned int mapHeight() const
	{
		return mMapHeight;
	}

	unsigned int minPlayers() const
	{
		return mMinPlayers;
	}
	int maxPlayers() const
	{
		return mMaxPlayers;
	}

	bool loadInformation(BPFFile* file);

protected:
	bool loadPlayersInformation(const QByteArray& xml);
	bool loadMapInformation(const QByteArray& xml);

private:
	unsigned int mMapWidth;
	unsigned int mMapHeight;
	// AB: add description data ?

	unsigned int mMinPlayers;
	int mMaxPlayers;
};


/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonPlayField : public QObject
{
	Q_OBJECT
public:
	BosonPlayField(QObject* parent = 0);
	~BosonPlayField();

	bool loadFromDiskToFiles(QMap<QString, QByteArray>& destFiles);
	QByteArray loadFromDiskToStream(QMap<QString, QByteArray>* destFiles = 0);

	bool savePlayFieldToFiles(QMap<QString, QByteArray>& destFiles);

	/**
	 * Stream all (virtual) files in @p files.
	 **/
	static QByteArray streamFiles(const QMap<QString, QByteArray>& files);
	static bool unstreamFiles(QMap<QString, QByteArray>& destFiles, const QByteArray& buffer);

	/**
	 * Load the important data (description for example) from the playField.
	 * Use @ref loadPlayField to load <em>all</em> data. preLoadPlayField is
	 * much faster than @ref loadPlayField.
	 **/
	bool preLoadPlayField(const QString& identifier);

	/**
	 * Load the playfield from @p files to this object.
	 **/
	bool loadPlayField(const QMap<QString, QByteArray>& files);

	bool importHeightMapImage(const QImage& image);

	/**
	 * @return a @ref QByteArray with the heightmap encoded as a .png  image. You
	 * can write this directly to the disk or use a @ref QImage constructor
	 * that takes a @ref QByteArray.
	 **/
	QByteArray exportHeightMap() const;

	/**
	 * @return a @ref QByteArray with the texmap encoded as a .png  image. You
	 * can write this directly to the disk or use a @ref QImage constructor
	 * that takes a @ref QByteArray.
	 **/
	QByteArray exportTexMap(unsigned int texture) const;

	bool savePlayField(const QString& fileName);

	// AB: do we still need these?
	bool savePlayFieldForRemote(QDataStream& stream) const;
	bool loadPlayFieldFromRemote(QDataStream& stream);

	/**
	 * Make @p map the new map of this playfield. Called by the editor
	 * only.
	 **/
	void changeMap(BosonMap* map);

	/**
	 * Make @p d the new description of this playfield. Called by the editor
	 * only.
	 **/
	void changeDescription(BPFDescription* d);

	BosonMap* map() const { return mMap; }
	BPFDescription* description() const { return mDescription; }
	const QString& identifier() const { return mIdentifier; }

	/**
	 * @return The @ref BosonPlayFieldInformation object of this playfield
	 * (always non-NULL). Use @ref isPreLoaded to find out whether it
	 * already contains useful information.
	 **/
	const BosonPlayFieldInformation* information() const
	{
		return mPlayFieldInformation;
	}

	/**
	 * This simply deletes the map. This may be useful for game starting, as
	 * the ADMIN already has the map, but all other players don't. So the
	 * ADMIN might send the map over network and delete his local copy, so
	 * that it is ensured the entire loading code is the same on all
	 * clients, i.e. without a map until it is received.
	 **/
	void deleteMap();

	void quit();

	bool modified() const;

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

public:
	/**
	 * Preload all playFields into memory. Note that this will consume pretty much
	 * memory, so once a map is started all other maps should be cleared
	 * again.
	 **/
	static bool preLoadAllPlayFields();

	/**
	 * Clear all preloaded playfields (see @ref preLoadAllPlayFields)
	 **/
	static void clearAllPreLoadedPlayFields();

	/**
	 * @return The default playfield which is meant to be used if the player
	 * didn't select a playfield at all
	 **/
	static QString defaultPlayField();

signals:
	/**
	 * Emitted when the map changes. Note that this can even be 0!
	 **/
	void signalNewMap(BosonMap*);

protected:
	bool loadDescriptionFromFile(const QByteArray& xml);
	bool loadMapFromFile(const QByteArray& xml, const QByteArray& heightMapImage, const QByteArray& texMap);

	QString saveDescriptionToFile();
	QByteArray saveMapToFile();
	QByteArray saveTexMapToFile();

	bool saveMap(QDataStream& stream) const;
	bool saveDescription(QDataStream& stream) const;

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

	/**
	 * @return A list of all playfields (.bpf files) that are found on this
	 * computer. This does not check whether they got already loaded.
	 **/
	static QStringList findAvailablePlayFields();

	static QStringList findAvailableCampaigns();
	static QStringList findPlayFieldsOfCampaign(const QString& campaign);

private:
	BosonMap* mMap;
	BosonPlayFieldInformation* mPlayFieldInformation;
	BPFDescription* mDescription;
	QString mIdentifier; // AB: this is not yet fully implemented - e.g. it isn't changed when saving or changing the map. should be the filename (see BPFFile::identifier())
	bool mLoaded;
	bool mPreLoaded;
	BPFFile* mFile;
	QString mFileName;
};

#endif
