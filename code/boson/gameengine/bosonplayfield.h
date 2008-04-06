/*
    This file is part of the Boson game
    Copyright (C) 2002-2008 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONPLAYFIELD_H
#define BOSONPLAYFIELD_H

#include <qobject.h>

class BosonMap;
class Boson;
class BPFFile;
class BPFDescription;
class BPFPreview;

class QImage;
class QDomDocument;
class QDomElement;
class QDataStream;
template<class T, class T2> class QMap;

// FIXME: do we really want to store rules and winning conditions here?
//        -> both may depend on players and units that are in the game (e.g.
//           "unit ID xyz cannot be destroyed"), so it rather belongs to the
//           BosonCanvas?!
//           -> if we store rules+winning condition in BosonCanvas, then this
//              class is useless. it provides .bpf file loading (which does not
//              belong here anyway) and playfield description/name/comment
//              (which could also be stored in the map or somewhere else) only.
//              the rest is stored in BosonMap
/**
 * A playfield is a "level" in boson. It consists of a map (see @ref BosonMap
 * and @ref map),
 * a description of the playfield (see @ref description) and rules (which may
 * e.g. limit what units may be used on this playfield - not yet implemented)
 * and winning conditions (not yet implemented).
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonPlayField : public QObject
{
	Q_OBJECT
public:
	BosonPlayField(QObject* parent = 0);
	~BosonPlayField();

	BosonMap* map() const { return mMap; }
	const BPFDescription* description() const;

	/**
	 * This simply deletes the map. This may be useful for game starting, as
	 * the ADMIN already has the map, but all other players don't. So the
	 * ADMIN might send the map over network and delete his local copy, so
	 * that it is ensured the entire loading code is the same on all
	 * clients, i.e. without a map until it is received.
	 **/
	void deleteMap();

	void quit();

	QString playFieldName() const;
	QString playFieldComment() const;

	/**
	 * Load the playfield from @p files to this object.
	 *
	 * See @ref BPFLoader on how to retrieve these virtual @p files.
	 **/
	bool loadPlayFieldFromFiles(const QMap<QString, QByteArray>& files);

	/**
	 * This saves this @ref BosonPlayField object into a set of virtual
	 * files that are stored into @p destFiles.
	 **/
	bool savePlayFieldToFiles(QMap<QString, QByteArray>& destFiles);

	////////////////////////////////////////////////
	//////////////////// Methods for the editor only
	////////////////////////////////////////////////
	/**
	 * See also @ref BosonMap::importHeightMapImage
	 *
	 * This method is meant for the editor only.
	 **/
	bool importHeightMapImage(const QImage& image);

	/**
	 * See also @ref BosonMap::saveHeightMapImage
	 *
	 * This method is meant for the editor only.
	 *
	 * @return a @ref QByteArray with the heightmap encoded as a .png  image. You
	 * can write this directly to the disk or use a @ref QImage constructor
	 * that takes a @ref QByteArray.
	 **/
	QByteArray exportHeightMap() const;

	/**
	 * See also @ref BosonMap::saveTexMapImage
	 *
	 * This method is meant for the editor only.
	 *
	 * @return a @ref QByteArray with the texmap encoded as a .png  image. You
	 * can write this directly to the disk or use a @ref QImage constructor
	 * that takes a @ref QByteArray.
	 **/
	QByteArray exportTexMap(unsigned int texture) const;

	/**
	 * Make @p map the new map of this playfield. Called by the editor
	 * only.
	 **/
	void changeMap(BosonMap* map);

	/**
	 * Set a @ref BPFDescription object that is meant to replace the
	 * original @ref description object from @ref preview.
	 *
	 * This method is meant to be used in the editor only.
	 **/
	void setModifiedDescription(BPFDescription* description);

	// AB: not really supported currently (2008/03/22)!
	bool modified() const;
	//////////////////////////////////////////////////////
	//////////////////// Methods for the editor only (end)
	//////////////////////////////////////////////////////
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

	/**
	 * @return A list of all playfields (.bpf files) that are found on this
	 * computer. This does not check whether they got already loaded.
	 *
	 * You usually should not use this method, but rather use @ref
	 * BosonData::playField instead.
	 **/
	static QStringList findAvailablePlayFields();

protected:
	bool loadMapFromFiles(const QMap<QString, QByteArray>& files);

	QString saveDescriptionToFile() const;
	QByteArray saveMapPreviewPNGToFile() const;

	static QStringList findAvailableCampaigns();
	static QStringList findPlayFieldsOfCampaign(const QString& campaign);

private:
	BosonMap* mMap;
	BPFDescription* mDescription;
};

#endif
