#ifndef __BOSONMAP_H__
#define __BOSONMAP_H__

#include <qstring.h>
#include <qdatastream.h>

class Cell;
class Unit;
class Boson;
class KGamePropertyHandler;

class BosonMapPrivate;

/**
 * This class represents a Boson map file (*.bpf). Use @ref loadMap to load the
 * file and you get the min/max players of the map (currently both have the same
 * value) as well as all of the units/facilities.
 **/
class BosonMap
{
public:
	enum PropertyId {
		IdMaxPlayers = 0,
		IdMinPlayers = 1,
		IdMapHeight = 2,
		IdMapWidth = 3,
		IdWorldName = 4 //TODO: i18n
	};

	BosonMap();
	BosonMap(const QString& fileName);
	~BosonMap();

	int height() const;
	int width() const;
	int maxPlayers() const;
	unsigned int minPlayers() const;
	QString worldName() const;

	/**
	 * Load the specified map from a file
	 * @param fileName the absolute filename of the map file.
	 **/
	bool loadMap(const QString& fileName);
	
	/**
	 * Read the map geo from stream. This only reads map size, playercount,
	 * cells and something like this. This does <em>not</em> read the units
	 * of the player. Usually you will transmit the geo parts of a map (see
	 * @ref saveMapGeo) to all clients but only the server loads the units
	 * which will be added.
	 * @param stream The stream to read from
	 **/
	bool loadMapGeo(QDataStream& stream);

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

	bool saveMap(const QString& fileName);

	/**
	 * @return The (hardcoded) default map
	 **/
	static QString defaultMap();

	/**
	 * @return TRUE if the current map geo is valid i.e. can be transmitted
	 * savely using @ref saveMapGeo
	 **/
	bool isValidGeo() const;

	KGamePropertyHandler* dataHandler() const;

	Cell* cell(int x, int y) const;

protected:
	bool loadCell(QDataStream& stream, int& groundType, unsigned char& b);
	void saveCell(QDataStream& stream, Cell* cell);

	/**
	 * Load the entire map from a stream. Used mainly by @ref loadMap.
	 *
	 * You will rather want to use @ref loadMapGeo
	 **/
	bool loadCompleteMap(QDataStream& stream);

	/**
	 * Read the magic string from stream.
	 * @return TRUE if the magic string matches the expected value.
	 * Otherwise FALSE (not a boson map file)
	 **/
	bool verifyMap(QDataStream& stream);

private:
	void init();

private:
	BosonMapPrivate* d;
};

#endif
