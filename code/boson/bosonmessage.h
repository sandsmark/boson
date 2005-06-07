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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef BOSONMESSAGE_H
#define BOSONMESSAGE_H

#include "bosonmessageids.h"
#include "bomath.h"
#include "bo3dtools.h"

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
};

class BosonMessageMovePlaceUnit : public BosonMessage
{
public:
	BosonMessageMovePlaceUnit();
	BosonMessageMovePlaceUnit(Q_UINT32 unitType, Q_UINT32 owner, const BoVector2Fixed& pos);

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
};

class BosonMessageMoveChangeTexMap : public BosonMessage
{
public:
	BosonMessageMoveChangeTexMap();

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
	BosonMessageMoveChangeTexMap(const QValueVector<Q_UINT32>& cellCornersX,
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

class BosonMessageMoveChangeHeight : public BosonMessage
{
public:
	BosonMessageMoveChangeHeight();
	BosonMessageMoveChangeHeight(
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

class BosonMessageMoveDeleteItems : public BosonMessage
{
public:
	BosonMessageMoveDeleteItems();
	BosonMessageMoveDeleteItems(Q_UINT32 count, const QValueList<Q_ULONG>& items);

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);
	virtual int messageId() const
	{
		return BosonMessageIds::MoveDeleteItems;
	}

public:
	Q_UINT32 mCount;
	QValueList<Q_ULONG> mItems;
};

#endif

