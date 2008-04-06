/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONMESSAGE_H
#define BOSONMESSAGE_H

#include "bosonmessageids.h"
#include "../bomath.h"
#include "../bo3dtools.h"

#include <qnamespace.h>
#include <q3valuelist.h>
#include <q3valuevector.h>

class QDataStream;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonMessage
{
public:
	virtual ~BosonMessage()
	{
	}

	/**
	 * @return The message id that identifies this message. See @ref
	 * BosonMessageIds.
	 **/
	virtual int messageId() const = 0;
	virtual bool save(QDataStream& stream) const = 0;
	virtual bool load(QDataStream& stream) = 0;


	/**
	 * @internal
	 * Read the message ID from @p stream.
	 *
	 * Note that this is for internal use only.
	 **/
	virtual bool readMessageId(QDataStream& stream) const;

	bool copyTo(BosonMessage& copy) const;
};

class BosonMessageEditorMove : public BosonMessage
{
public:
	BosonMessageEditorMove();

	virtual bool readMessageId(QDataStream& stream) const;

	void setUndo()
	{
		mUndo = true;
	}
	void setRedo()
	{
		mRedo = true;
	}

	bool isUndo() const
	{
		return mUndo;
	}
	bool isRedo() const
	{
		return mRedo;
	}

	bool saveFlags(QDataStream&) const;
	bool loadFlags(QDataStream&);

	static BosonMessageEditorMove* newCopy(const BosonMessageEditorMove& message);

private:
	qint8 mUndo;
	qint8 mRedo;
};

class BosonMessageEditorMovePlaceUnit : public BosonMessageEditorMove
{
public:
	BosonMessageEditorMovePlaceUnit();
	BosonMessageEditorMovePlaceUnit(quint32 unitType, quint32 owner, const BoVector2Fixed& pos, const bofixed& rotation);

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MovePlaceUnit;
	}

public:
	quint32 mUnitType;
	quint32 mOwner;
	BoVector2Fixed mPos;
	bofixed mRotation;
};

class BosonMessageEditorMoveChangeTexMap : public BosonMessageEditorMove
{
public:
	BosonMessageEditorMoveChangeTexMap();

	/**
	 * A message consists of a number of cell corners (each x and y indices)
	 * and for every corner the number of textures (must be @ref
	 * BosonGroundTheme::groundTypeCount).
	 *
	 * For every of these textures there must be
	 * @li The texture index (each of 0..@ref
	 * BosonGroundTheme::groundTypeCount must be present)
	 * @li The alpha value for that texture
	 **/
	BosonMessageEditorMoveChangeTexMap(const Q3ValueVector<quint32>& cellCornersX,
			const Q3ValueVector<quint32>& cellCornersY,
			const Q3ValueVector<quint32>& cellCornersTexCount,
			const Q3ValueVector< Q3ValueVector<quint32> > cellCornerTextures,
			const Q3ValueVector< Q3ValueVector<quint8> > cellCornerAlpha);

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveChangeTexMap;
	}

public:
	Q3ValueVector<quint32> mCellCornersX;
	Q3ValueVector<quint32> mCellCornersY;
	Q3ValueVector<quint32> mCellCornersTextureCount;
	Q3ValueVector< Q3ValueVector<quint32> > mCellCornerTextures;
	Q3ValueVector< Q3ValueVector<quint8> > mCellCornerAlpha;
};

class BosonMessageEditorMoveChangeHeight : public BosonMessageEditorMove
{
public:
	BosonMessageEditorMoveChangeHeight();
	BosonMessageEditorMoveChangeHeight(
			const Q3ValueVector<quint32> cellCornersX,
			const Q3ValueVector<quint32> cellCornersY,
			const Q3ValueVector<bofixed> cellCornersHeight
			);

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveChangeHeight;
	}

public:
	Q3ValueVector<quint32> mCellCornersX;
	Q3ValueVector<quint32> mCellCornersY;
	Q3ValueVector<bofixed> mCellCornersHeight;
};

class BosonMessageEditorMoveDeleteItems : public BosonMessageEditorMove
{
public:
	BosonMessageEditorMoveDeleteItems();
	BosonMessageEditorMoveDeleteItems(const Q3ValueList<quint32>& items);

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveDeleteItems;
	}

public:
	Q3ValueList<quint32> mItems;
};

class BosonMessageEditorMoveUndoPlaceUnit : public BosonMessageEditorMove
{
public:
	BosonMessageEditorMoveUndoPlaceUnit() : BosonMessageEditorMove()
	{
		setUndo();
	}
	BosonMessageEditorMoveUndoPlaceUnit(quint32 unit, const BosonMessageEditorMovePlaceUnit& message);

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveUndoPlaceUnit;
	}

public:
	BosonMessageEditorMoveDeleteItems mDeleteUnit;
	BosonMessageEditorMovePlaceUnit mMessage;
};

class BosonMessageEditorMoveUndoDeleteItems : public BosonMessageEditorMove
{
public:
	BosonMessageEditorMoveUndoDeleteItems()
	{
		setUndo();
	}
	~BosonMessageEditorMoveUndoDeleteItems();

	/**
	 * @param units One PlaceUnit message per unit that was deleted. Note
	 * that ownership is NOT taken, you need to delete the pointers
	 * yourself.
	 * @param unitsData A string containing @ref BosonItem::saveAsXML
	 * @param message The original DeleteItems message.
	 **/
	BosonMessageEditorMoveUndoDeleteItems(
			const Q3ValueList<BosonMessageEditorMovePlaceUnit*>& units,
			const Q3ValueList<QString>& unitsData,
			const BosonMessageEditorMoveDeleteItems& message
	);

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveUndoDeleteItems;
	}

protected:
	void clearUnits();

public:
	Q3ValueList<BosonMessageEditorMovePlaceUnit*> mUnits;
	Q3ValueList<QString> mUnitsData;
	BosonMessageEditorMoveDeleteItems mMessage;
};

class BosonMessageEditorMoveUndoChangeHeight : public BosonMessageEditorMove
{
public:
	BosonMessageEditorMoveUndoChangeHeight() : BosonMessageEditorMove()
	{
		setUndo();
	}
	BosonMessageEditorMoveUndoChangeHeight(const BosonMessageEditorMoveChangeHeight& originalHeights, const BosonMessageEditorMoveChangeHeight& message);

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveUndoChangeHeight;
	}

public:
	BosonMessageEditorMoveChangeHeight mOriginalHeights;
	BosonMessageEditorMoveChangeHeight mMessage;
};

class BosonMessageMoveMove : public BosonMessage
{
public:
	BosonMessageMoveMove();
	BosonMessageMoveMove(bool isAttack, const BoVector2Fixed& pos, const Q3ValueList<quint32>& items);

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveMove;
	}

public:
	qint8 mIsAttack;
	BoVector2Fixed mPos;
	Q3ValueList<quint32> mItems;
};

class BosonMessageMoveAttack : public BosonMessage
{
public:
	BosonMessageMoveAttack() : BosonMessage() {}
	BosonMessageMoveAttack(quint32 attackedUnitId, const Q3ValueList<quint32>& items)
		: BosonMessage(),
		mAttackedUnitId(attackedUnitId),
		mItems(items)
	{
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveAttack;
	}

public:
	quint32 mAttackedUnitId;
	Q3ValueList<quint32> mItems;
};

class BosonMessageMoveStop : public BosonMessage
{
public:
	BosonMessageMoveStop() : BosonMessage() {}
	BosonMessageMoveStop(const Q3ValueList<quint32>& items)
		: BosonMessage(),
		mItems(items)
	{
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveStop;
	}

public:
	Q3ValueList<quint32> mItems;
};

class BosonMessageMoveMine : public BosonMessage
{
public:
	BosonMessageMoveMine() : BosonMessage() {}
	BosonMessageMoveMine(quint32 harvester, quint32 resourceMine)
		: BosonMessage(),
		mHarvesterId(harvester),
		mResourceMineId(resourceMine)
	{
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveMine;
	}

public:
	quint32 mHarvesterId;
	quint32 mResourceMineId;
};

class BosonMessageMoveRefine: public BosonMessage
{
public:
	BosonMessageMoveRefine() : BosonMessage() {}
	BosonMessageMoveRefine(quint32 refineryOwner, quint32 refineryId, const Q3ValueList<quint32> items)
		: BosonMessage(),
		mRefineryOwner(refineryOwner),
		mRefineryId(refineryId),
		mItems(items)
	{
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveRefine;
	}

public:
	quint32 mRefineryOwner;
	quint32 mRefineryId;
	Q3ValueList<quint32> mItems;
};

// is a TODO
class BosonMessageMoveRepair : public BosonMessage
{
public:
	BosonMessageMoveRepair() : BosonMessage() {}
	BosonMessageMoveRepair(quint32 unit)
		: BosonMessage(),
		mUnit(unit)
	{
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveRepair;
	}

public:
	quint32 mUnit;
};

class BosonMessageMoveProduce : public BosonMessage
{
public:
	BosonMessageMoveProduce() : BosonMessage() {}
	BosonMessageMoveProduce(quint32 produceType, quint32 owner, quint32 factoryId, quint32 type)
		: BosonMessage(),
		mProduceType(produceType),
		mOwner(owner),
		mFactoryId(factoryId),
		mType(type)
	{
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveProduce;
	}

public:
	quint32 mProduceType;
	quint32 mOwner;
	quint32 mFactoryId;
	quint32 mType;
};

class BosonMessageMoveProduceStop : public BosonMessage
{
public:
	BosonMessageMoveProduceStop() : BosonMessage() {}
	BosonMessageMoveProduceStop(quint32 produceType, quint32 owner, quint32 factoryId, quint32 type)
		: BosonMessage(),
		mProduceType(produceType),
		mOwner(owner),
		mFactoryId(factoryId),
		mType(type)
	{
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveProduceStop;
	}

public:
	quint32 mProduceType;
	quint32 mOwner;
	quint32 mFactoryId;
	quint32 mType;
};

class BosonMessageMoveBuild : public BosonMessage
{
public:
	BosonMessageMoveBuild() : BosonMessage() {}
	BosonMessageMoveBuild(quint32 produceType, quint32 owner, quint32 factoryId, const BoVector2Fixed& pos)
		: BosonMessage(),
		mProduceType(produceType),
		mOwner(owner),
		mFactoryId(factoryId),
		mPos(pos)
	{
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveBuild;
	}

public:
	quint32 mProduceType;
	quint32 mOwner;
	quint32 mFactoryId;
	BoVector2Fixed mPos;
};

class BosonMessageMoveFollow : public BosonMessage
{
public:
	BosonMessageMoveFollow() : BosonMessage() {}
	BosonMessageMoveFollow(quint32 followUnitId, const Q3ValueList<quint32>& items)
		: BosonMessage(),
		mFollowUnitId(followUnitId),
		mItems(items)
	{
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveFollow;
	}

public:
	quint32 mFollowUnitId;
	Q3ValueList<quint32> mItems;
};

class BosonMessageMoveEnterUnit : public BosonMessage
{
public:
	BosonMessageMoveEnterUnit() : BosonMessage() {}
	BosonMessageMoveEnterUnit(quint32 enterUnitId, const Q3ValueList<quint32>& items)
		: BosonMessage(),
		mEnterUnitId(enterUnitId),
		mItems(items)
	{
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveEnterUnit;
	}

public:
	quint32 mEnterUnitId;
	Q3ValueList<quint32> mItems;
};

class BosonMessageMoveLayMine : public BosonMessage
{
public:
	BosonMessageMoveLayMine() : BosonMessage() {}
	BosonMessageMoveLayMine(const Q3ValueList<quint32>& units, const Q3ValueList<quint32>& weapons)
		: BosonMessage(),
		mUnits(units),
		mWeapons(weapons)
	{
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveLayMine;
	}

public:
	Q3ValueList<quint32> mUnits;
	Q3ValueList<quint32> mWeapons;
};

class BosonMessageMoveDropBomb : public BosonMessage
{
public:
	BosonMessageMoveDropBomb() : BosonMessage() {}
	BosonMessageMoveDropBomb(const BoVector2Fixed& pos, const Q3ValueList<quint32>& units, const Q3ValueList<quint32>& weapons)
		: BosonMessage(),
		mPos(pos),
		mUnits(units),
		mWeapons(weapons)
	{
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveDropBomb;
	}

public:
	BoVector2Fixed mPos;
	Q3ValueList<quint32> mUnits;
	Q3ValueList<quint32> mWeapons;
};

class BosonMessageMoveTeleport : public BosonMessage
{
public:
	BosonMessageMoveTeleport() : BosonMessage() {}
	BosonMessageMoveTeleport(quint32 unitId, quint32 owner, const BoVector2Fixed& pos)
		: BosonMessage(),
		mUnitId(unitId),
		mOwner(owner),
		mPos(pos)
	{
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveTeleport;
	}

public:
	quint32 mUnitId;
	quint32 mOwner;
	BoVector2Fixed mPos;
};

class BosonMessageMoveRotate : public BosonMessage
{
public:
	BosonMessageMoveRotate() : BosonMessage() {}
	BosonMessageMoveRotate(quint32 unitId, quint32 owner, const bofixed& rotate)
		: BosonMessage(),
		mUnitId(unitId),
		mOwner(owner),
		mRotate(rotate)
	{
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveRotate;
	}

public:
	quint32 mUnitId;
	quint32 mOwner;
	bofixed mRotate;
};

#endif

