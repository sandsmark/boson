#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <kgame/kplayer.h>

#include <qcolor.h>

class QCanvasPixmapArray;
class VisualUnit;
class SpeciesTheme;
class UnitProperties;


class PlayerPrivate;
class Player : public KPlayer
{
	Q_OBJECT
public:
	Player();
	virtual ~Player();

	/**
	 * @return A @ref QCanvasPixmapArray for the specified unit (see @ref
	 * UnitProperties::typeId) of the species of this player
	 **/
	QCanvasPixmapArray* pixmapArray(int unitType) const;

	void loadTheme(const QString& species, const QRgb& teamColor);

	void addUnit(VisualUnit* unit);
	void unitDestroyed(VisualUnit* unit);
	SpeciesTheme* speciesTheme() const;

	VisualUnit* findUnit(unsigned long int unitId) const;

	virtual bool load(QDataStream& stream);
	virtual bool save(QDataStream& stream);

	/**
	 * @return <em>All</em> units of this player. Please don't use this as
	 * it is very unclean. This is meant for KGameUnitDebug only.
	 **/
	QPtrList<VisualUnit> allUnits() const;

	/**
	 * Called by @ref VisualUnit to sync the positions of the units on
	 * several clients. Shouldn't be necessary if it was implemented
	 * cleanly!!
	 **/
	void sendStopMoving(VisualUnit* unit);

	/**
	 * Convenience method for theme()->unitProperties()
	 **/
	const UnitProperties* unitProperties(int unitType) const;

signals:
	void signalCreateUnit(VisualUnit*& unit, int unitType, Player* owner); // obsolete
	void signalLoadUnit(int unitType, unsigned long int id, Player* owner);

	void signalUnitChanged(VisualUnit* unit);

public slots:
	void slotUnitPropertyChanged(KGamePropertyBase* prop);
	void slotNetworkData(int msgid, const QByteArray& msg, Q_UINT32 sender, KPlayer*);

private:
	PlayerPrivate* d;
};

#endif
