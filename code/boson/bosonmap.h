/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONMAPDOM_H
#define BOSONMAPDOM_H

#include <qobject.h>

class Cell;
class UnitBase;
class Boson;
class QDomElement;
class QStringList;
class QDataStream;
class BosonTiles;

/**
 * DOC is OBSOLETE
 * 
 * This class represents a Boson map file (*.bpf). Use @ref loadMap to load the
 * file and you get the min/max players of the map (currently both have the same
 * value) as well as all of the units/facilities.
 *
 * BosonMap supports loading of binary files as well as XML files. The XML file
 * can also be compressed using gzip (automatically done by @ref saveMap). The
 * binary file is faster to read and smaller than the XML file (if it is
 * <em>not</em> compressed). A compressed XML file is usually even smaller.
 *
 * Boson uses internally a binary format to send information over the network.
 * Therefore the binary loading methods are also used to load the XML file. The
 * XML file is first parsed completely and stored in a @ref QDomDocument object.
 * This object is then queried and all information are streamed using @ref
 * QDataStream. These streames are now read by BosonMap to actually load the
 * file. This concept might be a little bit confusing but this way we ensure
 * that changes in the binary format and/or XML format result in change of the
 * other format as well, i.e. we don't have broken formats around (ideally at
 * least)
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonMap : public QObject
{
	Q_OBJECT
public:
	BosonMap(QObject* parent = 0);
	~BosonMap();

	/**
	 * @return vertical cell count
	 **/
	inline unsigned int height() const { return mMapHeight; }
	
	/**
	 * @return Horizonatal cell count
	 **/
	inline unsigned int width() const { return mMapWidth; }

	int maxPlayers() const;
	unsigned int minPlayers() const;

	bool loadMap(QDomElement& node);
	
	/**
	 * Read the map geo from stream. This only reads map size, playercount
	 * and something like this. Use @ref loadCells to load the cells.
	 *
	 * This does <em>not</em> read the units
	 * of the player. Usually you will transmit the geo parts of a map (see
	 * @ref saveMapGeo) to all clients but only the server loads the units
	 * which will be added.
	 **/
	bool loadMapGeo(QDataStream& stream);

	/**
	 * Load the cells from the stream.
	 * @param stream The stream to read from
	 **/
	bool loadCells(QDataStream& stream);

	/**
	 * Save the map geo into stream. This creates a stream in the format
	 * used by @ref loadMapGeo. You can use this to send the map geo to
	 * another client.
	 *
	 * Note that this doesn't add all the units of the player but just the
	 * basic settings of a map! The units should be loaded by the server
	 * only.
	 * @param stream The stream to write to
	 **/
	bool saveMapGeo(QDataStream& stream);
	bool saveCells(QDataStream& stream);

	bool saveMap(QDomElement& node);

	/**
	 * @return TRUE if the current map is valid i.e. can be transmitted
	 * savely using @ref saveMapGeo
	 **/
	bool isValid() const;

	Cell* cell(int x, int y) const;

	/**
	 * You should rather use @ref cell ! If you use this, you need to ensure
	 * yourself, that all cells you want ta access are actually valid.
	 * @return The internal cell array
	 **/
	inline Cell* cells() const { return mCells; }

	void setModified(bool m) { mModified = m; }
	bool modified() const { return mModified; }

	/**
	 * Tiles are loaded externally, so they need to be placed into the map.
	 *
	 * See BosonCanvas, where tile loading takes place (currently)
	 **/
	void setTileSet(BosonTiles* t);
	BosonTiles* tileSet() const { return mTiles; }

	/**
	 * Load the tileset - see @ref BosonTiles
	 *
	 * Note that the actual loading happens in @ref slotLoadTiles using
	 * a @ref QTimer::singleShot. This gives us a non-blocking UI as we can
	 * use @ref QApplication::processEvents
	 * @param tiles currently always "earth.png", currently
	 * @param withtimer whether to use timer
	 **/
	void loadTiles(const QString& tiles, bool withtimer = true);

public slots:
	void slotChangeCell(int x, int y, int groundType, unsigned char b);

signals:
	void signalTileSetChanged(BosonTiles*);

	void signalTilesLoading(int);
	void signalTilesLoaded();

protected:
	bool loadCell(QDataStream& stream, int& groundType, unsigned char& b);

	void saveCell(QDataStream& stream, int groundType, unsigned char b);

	bool saveMapGeo(QDomElement&);
	bool saveCells(QDomElement&);
	bool saveCell(QDomElement&, int x, int y, Cell* cell);

	bool loadMapGeo(QDomElement&);
	bool loadCells(QDomElement&);
	bool loadCell(QDomElement&, int& x, int& y, int& groundType, unsigned char& b);


	bool loadMap(const QByteArray& buffer, bool binary);

	/**
	 * Read the magic string from stream.
	 * @return TRUE if the magic string matches the expected value.
	 * Otherwise FALSE (not a boson map file)
	 **/
	bool verifyMap(QDataStream& stream);

	/**
	 * Write the validity string to the stream. @ref verifyMap reads this
	 * string, so a binary map loaded from a file should first contain the
	 * validity string/header.
	 **/
	void saveValidityHeader(QDataStream& stream);

protected slots:
	/**
	 * Load the tileset that has been specified using @ref loadTiles. We use
	 * this slot to provide a non-blocking tile loading.
	 **/
	void slotLoadTiles();

private:
	void init();

private:
	class BosonMapPrivate;
	BosonMapPrivate* d;
	Cell* mCells;
	bool mModified;

	unsigned int mMapWidth;
	unsigned int mMapHeight;

	BosonTiles* mTiles;
};

#endif
