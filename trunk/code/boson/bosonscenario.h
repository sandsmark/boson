#ifndef __BOSONSCENARIO_H__
#define __BOSONSCENARIO_H__

#include <qstring.h>
#include <qdatastream.h>

class Unit;
class Boson;
class Player;
class KGamePropertyHandler;

class BosonScenarioPrivate;

class BosonScenario
{
public:
	enum PropertyId {
		IdMaxPlayers = 0,
		IdMinPlayers = 1,
		IdMapHeight = 2,
		IdMapWidth = 3,
		IdWorldName = 4 //TODO: i18n
	};

	BosonScenario();
	BosonScenario(const QString& fileName);
	~BosonScenario();

	int maxPlayers() const;
	unsigned int minPlayers() const;

//	QString worldName() const; // TODO: scenario name

	/**
	 * Load the specified scenario from a file. Note that this is just about
	 * the units - you have to load the map separately using @ref BosonMap!
	 * @param fileName the absolute filename of the map file.
	 **/
	// TODO: which map can be used with this scenario??
	bool loadScenario(const QString& fileName);
	
	/**
	 * @return The (hardcoded) default map
	 **/
	static QString defaultScenario();

	/**
	 * Add the units of this player to boson. See @ref Boson::slotSendAddUnit
	 * @param player Player number. 0..maxPlayers() 
	 **/
	void addPlayerUnits(Boson* boson, int playerNumber);

	/**
	 * Add all available player units to the game. This is like
	 * calling @ref addPlayerUnits for all players (0..maxPlayers()).
	 **/
	void startScenario(Boson* boson);

	KGamePropertyHandler* dataHandler() const;

	bool isValid() const;

protected:
	/**
	 * Add unit to the game. See also @ref Boson::slotSendAddUnit
	 **/
	void addUnit(Boson* boson, Player* owner, int unitType, int x, int y);

	/**
	 * Load the scenario from a stream. 
	 **/
	bool loadScenario(QDataStream& stream);

	bool loadUnits(QDataStream& stream, unsigned int playerNumber);

	/**
	 * Read the magic string from stream.
	 * @return TRUE if the magic string matches the expected value.
	 * Otherwise FALSE (not a boson map file)
	 **/
	bool verifyScenario(QDataStream& stream); // TODO

private:
	void init();

private:
	BosonScenarioPrivate* d;
};

#endif
