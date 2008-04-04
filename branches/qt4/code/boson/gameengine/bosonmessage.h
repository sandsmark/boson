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
#include <qvaluelist.h>
#include <qvaluevector.h>

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
	Q_INT8 mUndo;
	Q_INT8 mRedo;
};

class BosonMessageEditorMovePlaceUnit : public BosonMessageEditorMove
{
public:
	BosonMessageEditorMovePlaceUnit();
	BosonMessageEditorMovePlaceUnit(Q_UINT32 unitType, Q_UINT32 owner, const BoVector2Fixed& pos, const bofixed& rotation);

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MovePlaceUnit;
	}

public:
	Q_UINT32 mUnitType;
	Q_UINT32 mOwner;
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
	BosonMessageEditorMoveChangeTexMap(const QValueVector<Q_UINT32>& cellCornersX,
			const QValueVector<Q_UINT32>& cellCornersY,
			const QValueVector<Q_UINT32>& cellCornersTexCount,
			const QValueVector< QValueVector<Q_UINT32> > cellCornerTextures,
			const QValueVector< QValueVector<Q_UINT8> > cellCornerAlpha);

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveChangeTexMap;
	}

public:
	QValueVector<Q_UINT32> mCellCornersX;
	QValueVector<Q_UINT32> mCellCornersY;
	QValueVector<Q_UINT32> mCellCornersTextureCount;
	QValueVector< QValueVector<Q_UINT32> > mCellCornerTextures;
	QValueVector< QValueVector<Q_UINT8> > mCellCornerAlpha;
};

class BosonMessageEditorMoveChangeHeight : public BosonMessageEditorMove
{
public:
	BosonMessageEditorMoveChangeHeight();
	BosonMessageEditorMoveChangeHeight(
			const QValueVector<Q_UINT32> cellCornersX,
			const QValueVector<Q_UINT32> cellCornersY,
			const QValueVector<bofixed> cellCornersHeight
			);

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveChangeHeight;
	}

public:
	QValueVector<Q_UINT32> mCellCornersX;
	QValueVector<Q_UINT32> mCellCornersY;
	QValueVector<bofixed> mCellCornersHeight;
};

class BosonMessageEditorMoveDeleteItems : public BosonMessageEditorMove
{
public:
	BosonMessageEditorMoveDeleteItems();
	BosonMessageEditorMoveDeleteItems(const QValueList<Q_ULONG>& items);

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveDeleteItems;
	}

public:
	QValueList<Q_ULONG> mItems;
};

class BosonMessageEditorMoveUndoPlaceUnit : public BosonMessageEditorMove
{
public:
	BosonMessageEditorMoveUndoPlaceUnit() : BosonMessageEditorMove()
	{
		setUndo();
	}
	BosonMessageEditorMoveUndoPlaceUnit(Q_ULONG unit, const BosonMessageEditorMovePlaceUnit& message);

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
			const QValueList<BosonMessageEditorMovePlaceUnit*>& units,
			const QValueList<QString>& unitsData,
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
	QValueList<BosonMessageEditorMovePlaceUnit*> mUnits;
	QValueList<QString> mUnitsData;
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
	BosonMessageMoveMove(bool isAttack, const BoVector2Fixed& pos, const QValueList<Q_ULONG>& items);

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveMove;
	}

public:
	Q_INT8 mIsAttack;
	BoVector2Fixed mPos;
	QValueList<Q_ULONG> mItems;
};

class BosonMessageMoveAttack : public BosonMessage
{
public:
	BosonMessageMoveAttack() : BosonMessage() {}
	BosonMessageMoveAttack(Q_ULONG attackedUnitId, const QValueList<Q_ULONG>& items)
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
	Q_ULONG mAttackedUnitId;
	QValueList<Q_ULONG> mItems;
};

class BosonMessageMoveStop : public BosonMessage
{
public:
	BosonMessageMoveStop() : BosonMessage() {}
	BosonMessageMoveStop(const QValueList<Q_ULONG>& items)
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
	QValueList<Q_ULONG> mItems;
};

class BosonMessageMoveMine : public BosonMessage
{
public:
	BosonMessageMoveMine() : BosonMessage() {}
	BosonMessageMoveMine(Q_ULONG harvester, Q_ULONG resourceMine)
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
	Q_ULONG mHarvesterId;
	Q_ULONG mResourceMineId;
};

class BosonMessageMoveRefine: public BosonMessage
{
public:
	BosonMessageMoveRefine() : BosonMessage() {}
	BosonMessageMoveRefine(Q_UINT32 refineryOwner, Q_ULONG refineryId, const QValueList<Q_ULONG> items)
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
	Q_UINT32 mRefineryOwner;
	Q_ULONG mRefineryId;
	QValueList<Q_ULONG> mItems;
};

// is a TODO
class BosonMessageMoveRepair : public BosonMessage
{
public:
	BosonMessageMoveRepair() : BosonMessage() {}
	BosonMessageMoveRepair(Q_ULONG unit)
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
	Q_ULONG mUnit;
};

class BosonMessageMoveProduce : public BosonMessage
{
public:
	BosonMessageMoveProduce() : BosonMessage() {}
	BosonMessageMoveProduce(Q_UINT32 produceType, Q_UINT32 owner, Q_ULONG factoryId, Q_UINT32 type)
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
	Q_UINT32 mProduceType;
	Q_UINT32 mOwner;
	Q_ULONG mFactoryId;
	Q_UINT32 mType;
};

class BosonMessageMoveProduceStop : public BosonMessage
{
public:
	BosonMessageMoveProduceStop() : BosonMessage() {}
	BosonMessageMoveProduceStop(Q_UINT32 produceType, Q_UINT32 owner, Q_ULONG factoryId, Q_UINT32 type)
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
	Q_UINT32 mProduceType;
	Q_UINT32 mOwner;
	Q_ULONG mFactoryId;
	Q_UINT32 mType;
};

class BosonMessageMoveBuild : public BosonMessage
{
public:
	BosonMessageMoveBuild() : BosonMessage() {}
	BosonMessageMoveBuild(Q_UINT32 produceType, Q_UINT32 owner, Q_ULONG factoryId, const BoVector2Fixed& pos)
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
	Q_UINT32 mProduceType;
	Q_UINT32 mOwner;
	Q_ULONG mFactoryId;
	BoVector2Fixed mPos;
};

class BosonMessageMoveFollow : public BosonMessage
{
public:
	BosonMessageMoveFollow() : BosonMessage() {}
	BosonMessageMoveFollow(Q_UINT32 followUnitId, const QValueList<Q_ULONG>& items)
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
	Q_UINT32 mFollowUnitId;
	QValueList<Q_ULONG> mItems;
};

class BosonMessageMoveEnterUnit : public BosonMessage
{
public:
	BosonMessageMoveEnterUnit() : BosonMessage() {}
	BosonMessageMoveEnterUnit(Q_UINT32 enterUnitId, const QValueList<Q_ULONG>& items)
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
	Q_UINT32 mEnterUnitId;
	QValueList<Q_ULONG> mItems;
};

class BosonMessageMoveLayMine : public BosonMessage
{
public:
	BosonMessageMoveLayMine() : BosonMessage() {}
	BosonMessageMoveLayMine(const QValueList<Q_ULONG>& units, const QValueList<Q_ULONG>& weapons)
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
	QValueList<Q_ULONG> mUnits;
	QValueList<Q_ULONG> mWeapons;
};

class BosonMessageMoveDropBomb : public BosonMessage
{
public:
	BosonMessageMoveDropBomb() : BosonMessage() {}
	BosonMessageMoveDropBomb(const BoVector2Fixed& pos, const QValueList<Q_ULONG>& units, const QValueList<Q_ULONG>& weapons)
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
	QValueList<Q_ULONG> mUnits;
	QValueList<Q_ULONG> mWeapons;
};

class BosonMessageMoveTeleport : public BosonMessage
{
public:
	BosonMessageMoveTeleport() : BosonMessage() {}
	BosonMessageMoveTeleport(Q_ULONG unitId, Q_UINT32 owner, const BoVector2Fixed& pos)
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
	Q_ULONG mUnitId;
	Q_UINT32 mOwner;
	BoVector2Fixed mPos;
};

class BosonMessageMoveRotate : public BosonMessage
{
public:
	BosonMessageMoveRotate() : BosonMessage() {}
	BosonMessageMoveRotate(Q_ULONG unitId, Q_UINT32 owner, const bofixed& rotate)
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
	Q_ULONG mUnitId;
	Q_UINT32 mOwner;
	bofixed mRotate;
};

#endif

