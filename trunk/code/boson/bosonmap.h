#ifndef __BOSONMAPDOM_H__
#define __BOSONMAPDOM_H__

#include <qstring.h>
#include <qdatastream.h>

class Cell;
class UnitBase;
class Boson;
class QDomElement;
class QStringList;

class BosonMapPrivate;

/**
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
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonMap
{
public:
	BosonMap();
	BosonMap(const QString& fileName);
	~BosonMap();

	/**
	 * @return vertical cell count
	 **/
	int height() const { return mMapHeight; }
	
	/**
	 * @return Horizonatal cell count
	 **/
	int width() const { return mMapWidth; }

	int maxPlayers() const;
	unsigned int minPlayers() const;

	/**
	 * @return All valid map files. The list contains the .desktop files.
	 **/
	static QStringList availableMaps();

	/**
	 * Load the specified map from a file
	 * @param fileName the absolute filename of the map file.
	 **/
	bool loadMap(const QString& fileName);
	
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

	/**
	 * Save the map to a file. By default this creates an XML file. But you
	 * can also create a binary file. While an XML file is human readable a
	 * binary file is smaller and faster parsed by boson. 
	 *
	 * Please note that the map file is only read <em>once</em> from file
	 * per game, so speed does not matter much (icon loading takes far more
	 * time).
	 * @param fileName The filename of the new map. Must be absolute.
	 * @param binary If false this creates an XML file (much better
	 * readable). If true a binary file is created.
	 **/
	bool saveMap(const QString& fileName, bool binary = false);


	/**
	 * @return The (hardcoded) default map
	 **/
	static QString defaultMap();

	/**
	 * @return TRUE if the current map is valid i.e. can be transmitted
	 * savely using @ref saveMapGeo
	 **/
	bool isValid() const;

	Cell* cell(int x, int y) const;

	void changeCell(int x, int y, int groundType, unsigned char b);

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

private:
	void init();

private:
	BosonMapPrivate* d;
	int mMapWidth;
	int mMapHeight;
};

#endif
