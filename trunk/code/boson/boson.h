#ifndef __BOSON_H__
#define __BOSON_H__

#include <kgame/kgame.h>

class Player;
class QCanvas;
class Unit;
class Facility;

class BosonPrivate;
class Boson : public KGame
{
	Q_OBJECT
public:
	enum PropertyIds {
		IdGameSpeed = 10000, // dont wanna #include <kgameproperty.h> - better: KGamePropertyBase::IdUser+...
		IdNextUnitId = 10001
	};
	
	Boson(QObject* parent);
	Boson(QObject* parent, const QString& fileName);
	~Boson();

	void setCanvas(QCanvas*); 
	
	void quitGame();
	void startGame();

	int gameSpeed() const;
	bool isServer() const;

	virtual KPlayer* createPlayer(int rtti, int io, bool isVirtual);

	Unit* createUnit(int unitType, Player* owner); // public for Player::load

public slots:
	void slotSetGameSpeed(int speed);

	/**
	 * Doesn't actually add a unit to the game but sends a message to the
	 * network. The actual adding is done in @ref slotNetworkData
	 * @param unitType The type of the unit (see @ref UnitProperties::typeId) to be added
	 * @param x The x-coordinate (on the canvas) of the unit
	 * @param y The y-coordinate (on the canvas) of the unit
	 * @param owner The owner of the new unit.
	 **/
	void slotSendAddUnit(int unitType, int x, int y, Player* owner);

	void slotAdvanceComputerPlayers();

signals:
	/**
	 * Start a scenario. This should be done after loading map and scenario
	 * (a scenario is loaded first, <em>then</em> started). It is
	 * implemented using @ref KGame::sendMessage as the map must be loaded
	 * this way as well.
	 **/
	void signalStartScenario();

	/**
	 * @param unit The unit to be added
	 * @param x x-coordinate of the unit on the canvas
	 * @param y y-coordinate of the unit on the canvas
	 **/
	void signalAddUnit(Unit* unit, int x, int y);

	/**
	 * Order the canvas to call @ref QCanvas::advance
	 **/
	void signalAdvance();

	void signalInitMap(const QByteArray&);

protected:
	virtual bool playerInput(QDataStream& stream, KPlayer* player);

	unsigned long int nextUnitId();

	/**
	 * @param unitId The unit to search for
	 * @param searchIn The player to search the unit in. 0 for all players
	 **/
	Unit* findUnit(unsigned long int unitId, Player* searchIn) const;

	bool constructUnit(Facility* factory, int unitType, int x, int y);

protected slots:
	/**
	 * A network message arrived. Most game logic stuff is done here as
	 * nearly all functions just send a network request instead of doing the
	 * task theirselfes. This way we ensure that a task happens on
	 * <em>all</em> clients.
	 **/
	void slotNetworkData(int msgid, const QByteArray& buffer, Q_UINT32 receiver, Q_UINT32 sender);

	/**
	 * Send an advance message. When this is received by the clients @ref
	 * QCanvas::advance ist called.
	 **/
	void slotSendAdvance();

	void slotSave(QDataStream& stream);
	void slotLoad(QDataStream& stream);

	void slotReplacePlayerIO(KPlayer* player, bool* remove);

	void slotPlayerJoinedGame(KPlayer*);
	void slotPlayerLeftGame(KPlayer*);

private:
	BosonPrivate* d;
};

#endif
