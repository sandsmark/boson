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

#include "bosonmessage.h"

#include <bodebug.h>

BosonMessageMovePlaceUnit::BosonMessageMovePlaceUnit()
	: BosonMessage()
{
}

BosonMessageMovePlaceUnit::BosonMessageMovePlaceUnit(Q_UINT32 unitType, Q_UINT32 owner, const BoVector2Fixed& pos)
	: BosonMessage(),
	mUnitType(unitType),
	mOwner(owner),
	mPos(pos)
{
}

bool BosonMessageMovePlaceUnit::save(QDataStream& stream) const
{
 stream << (Q_UINT32)BosonMessageIds::MoveEditor;
 stream << (Q_UINT32)messageId();
 stream << mUnitType;
 stream << mOwner;
 stream << mPos;
 return true;
}

bool BosonMessageMovePlaceUnit::load(QDataStream& stream)
{
 // AB: msgid and editor flag have been read already
 stream >> mUnitType;
 stream >> mOwner;
 stream >> mPos;
 return true;
}


BosonMessageMoveChangeTexMap::BosonMessageMoveChangeTexMap()
	: BosonMessage()
{
}

BosonMessageMoveChangeTexMap::BosonMessageMoveChangeTexMap(
		const QValueVector<Q_UINT32>& cellCornersX,
		const QValueVector<Q_UINT32>& cellCornersY,
		const QValueVector<Q_UINT32>& cellCornersTexCount,
		const QValueVector< QValueVector<Q_UINT32> > cellCornerTextures,
		const QValueVector< QValueVector<Q_UINT8> > cellCornerAlpha
		)
	: BosonMessage(),
	mCellCornersX(cellCornersX),
	mCellCornersY(cellCornersY),
	mCellCornersTextureCount(cellCornersTexCount),
	mCellCornerTextures(cellCornerTextures),
	mCellCornerAlpha(cellCornerAlpha)
{
}

bool BosonMessageMoveChangeTexMap::save(QDataStream& stream) const
{
 if (mCellCornersX.count() != mCellCornersY.count()
		 || mCellCornersX.count() != mCellCornersTextureCount.count()
		 || mCellCornersX.count() != mCellCornerTextures.count()
		 || mCellCornersX.count() != mCellCornerAlpha.count()) {
	boError() << k_funcinfo << "invalid cell counts" << endl;
	return false;
 }
 if (mCellCornersX.count() == 0) {
	boError() << k_funcinfo << "nothing to save" << endl;
	return false;
 }
 stream << (Q_UINT32)BosonMessageIds::MoveEditor;
 stream << (Q_UINT32)messageId();
 stream << (Q_UINT32)mCellCornersX.count();
 for (unsigned int i = 0; i < mCellCornersX.count(); i++) {
	stream << (Q_UINT32)mCellCornersX[i];
	stream << (Q_UINT32)mCellCornersY[i];
	stream << (Q_UINT32)mCellCornersTextureCount[i];

	if (mCellCornersTextureCount[i] != mCellCornerTextures[i].count()) {
		boError() << k_funcinfo << "invalid number of cellcorner textures" << endl;
		return false;
	}
	for (Q_UINT32 j = 0; j < mCellCornersTextureCount[i]; j++) {
		if (j != (mCellCornerTextures[i])[j]) {
			boError() << k_funcinfo << "invalid message parameters: textures must be ordered sequentially" << endl;
			return false;
		}
		stream << (Q_UINT32)((mCellCornerTextures[i])[j]);
		stream << (Q_UINT32)((mCellCornerAlpha[i])[j]);
	}
 }
 return true;
}

bool BosonMessageMoveChangeTexMap::load(QDataStream& stream)
{
 // AB: msgid and editor flag have been read already
 Q_UINT32 count;
 stream >> count;
 mCellCornersX.resize(count);
 mCellCornersY.resize(count);
 mCellCornersTextureCount.resize(count);
 mCellCornerTextures.resize(count);
 mCellCornerAlpha.resize(count);
 for (Q_UINT32 i = 0; i < count; i++) {
	Q_UINT32 x;
	Q_UINT32 y;
	Q_UINT32 texCount;
	stream >> x;
	stream >> y;
	stream >> texCount;
	if (texCount > 100) {
		boError() << k_funcinfo << "more than 100 textures? invalid!" << endl;
		return false;
	}
	mCellCornersX[i] = x;
	mCellCornersY[i] = y;
	mCellCornersTextureCount[i] = texCount;

	mCellCornerTextures[i].resize(texCount);
	mCellCornerAlpha[i].resize(texCount);

	for (Q_UINT32 j = 0; j < texCount; j++) {
		Q_UINT32 tex;
		Q_UINT8 alpha;
		stream >> tex;
		stream >> alpha;
		if (tex != j) {
			boError() << k_funcinfo << "textures must be sequentially ordered" << endl;
			return false;
		}
		(mCellCornerTextures[i])[j] = tex;
		(mCellCornerAlpha[i])[j] = alpha;
	}
 }
 return true;
}


BosonMessageMoveChangeHeight::BosonMessageMoveChangeHeight()
	: BosonMessage()
{
}

BosonMessageMoveChangeHeight::BosonMessageMoveChangeHeight(
		const QValueVector<Q_UINT32> cellCornersX,
		const QValueVector<Q_UINT32> cellCornersY,
		const QValueVector<bofixed> cellCornersHeight
		)
	: BosonMessage(),
	mCellCornersX(cellCornersX),
	mCellCornersY(cellCornersY),
	mCellCornersHeight(cellCornersHeight)
{
}

bool BosonMessageMoveChangeHeight::save(QDataStream& stream) const
{
 if (mCellCornersX.count() != mCellCornersY.count() ||
		mCellCornersX.count() != mCellCornersHeight.count()) {
	boError() << k_funcinfo << "invalid cell counts" << endl;
 }
 if (mCellCornersX.count() == 0) {
	boError() << k_funcinfo << "nothing to save" << endl;
	return false;
 }
 stream << (Q_UINT32)BosonMessageIds::MoveEditor;
 stream << (Q_UINT32)messageId();
 stream << (Q_UINT32) mCellCornersX.count();
 for (unsigned int i = 0; i < mCellCornersX.count(); i++) {
	stream << (Q_UINT32)mCellCornersX[i];
	stream << (Q_UINT32)mCellCornersY[i];
	stream << mCellCornersHeight[i];
 }
 return true;
}

bool BosonMessageMoveChangeHeight::load(QDataStream& stream)
{
 // AB: msgid and editor flag have been read already
 Q_UINT32 count;
 stream >> count;
 mCellCornersX.resize(count);
 mCellCornersY.resize(count);
 mCellCornersHeight.resize(count);
 for (Q_UINT32 i = 0; i < count; i++) {
	Q_UINT32 x;
	Q_UINT32 y;
	bofixed height;
	stream >> x;
	stream >> y;
	stream >> height;
	mCellCornersX[i] = x;
	mCellCornersY[i] = y;
	mCellCornersHeight[i] = height;
 }
 return true;
}

BosonMessageMoveDeleteItems::BosonMessageMoveDeleteItems()
	: BosonMessage()
{
}

BosonMessageMoveDeleteItems::BosonMessageMoveDeleteItems(Q_UINT32 count, const QValueList<Q_ULONG>& items)
	: BosonMessage()
{
 mCount = count;
 mItems = items;
}

bool BosonMessageMoveDeleteItems::save(QDataStream& stream) const
{
 stream << (Q_UINT32)BosonMessageIds::MoveEditor;
 stream << (Q_UINT32)messageId();
 stream << mCount;
 QValueList<Q_ULONG>::const_iterator it;
 for (it = mItems.begin(); it != mItems.end(); ++it) {
	stream << (*it);
 }
 return true;
}

bool BosonMessageMoveDeleteItems::load(QDataStream& stream)
{
 // AB: msgid and editor flag have been read already
 stream >> mCount;
 mItems.clear();
 for (Q_UINT32 i = 0; i < mCount; i++) {
	Q_ULONG item;
	stream >> item;
	mItems.append(item);
 }
 return true;
}

