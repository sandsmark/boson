#ifndef __BOSON_H__
#define __BOSON_H__

#include <kgame/kgame.h>

class Player;
class QCanvas;
class VisualUnit;

class BosonPrivate;
class Boson : public KGame
{
	Q_OBJECT
public:
	enum PropertyIds {
//		IdGameSpeed = KGamePropertyBase::IdUser + 1
		IdGameSpeed = 10000, // dont wanna #include <kgameproperty.h>
		IdNextUnitId = 10001
	};
	
	Boson(QObject* parent);
	Boson(QObject* parent, const QString& fileName);
	~Boson();

	
	void quitGame();
	bool isServer() const;
	void startGame();

	void setCanvas(QCanvas*); // not nice - used for adding a unit only

	virtual KPlayer* createPlayer(int rtti, int io, bool isVirtual);

	int gameSpeed() const;

	VisualUnit* createUnit(int unitType, Player* owner); // public for Player::load

public slots:
	void slotSetGameSpeed(int speed);

	void slotConstructUnit(int unitType, VisualUnit* facility, Player* owner);
	void slotConstructUnit(int unitType, int x, int y, Player* owner);

signals:
	/**
	 * @param unit The unit to be added
	 * @param x x-coordinate of the unit on the canvas
	 * @param y y-coordinate of the unit on the canvas
	 **/
	void signalAddUnit(VisualUnit* unit, int x, int y);

	/**
	 * Order the canvas to call @ref QCanvas::advance
	 **/
	void signalAdvance();

	void signalInitMap(const QByteArray&);

protected:
	bool playerInput(QDataStream& stream, KPlayer* player);

	unsigned long int nextUnitId();

	/**
	 * @param unitId The unit to search for
	 * @param searchIn The player to search the unit in. 0 for all players
	 **/
	VisualUnit* findUnit(unsigned long int unitId, Player* searchIn) const;

protected slots:
	/**
	 * Doesn't actually add a unit to the game but sends a message to the
	 * network. The actual adding is done in @ref slotNetworkData
	 * @param unitType The type of the unit (see @ref UnitProperties::typeId) to be added
	 * @param x The x-coordinate (on the canvas) of the unit
	 * @param y The y-coordinate (on the canvas) of the unit
	 * @param owner The owner of the unit. Can either be
	 * 0..BOSON_MAX_PLAYERS-1 or KPlayer style ids. I recommend to use
	 * @ref KPlayer::id if possible
	 **/
	void slotAddUnit(int unitType, int x, int y, int owner);
	void slotLoadUnit(int unitType, unsigned long int id, Player* owner); // doesn't call setId()

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

//	void slotCreateUnit(VisualUnit*& unit, int unitType, Player* owner);

	void slotSave(QDataStream& stream);
	void slotLoad(QDataStream& stream);

	void slotReplacePlayerIO(KPlayer* player, bool* remove);

private:
	BosonPrivate* d;
};

#endif
