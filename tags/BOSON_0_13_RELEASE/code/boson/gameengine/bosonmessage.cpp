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

#include "../bomemory/bodummymemory.h"
#include <bodebug.h>

bool BosonMessage::readMessageId(QDataStream& stream) const
{
 Q_UINT32 msgid;
 stream >> msgid;
 if (msgid != (unsigned int)messageId()) {
	boError() << k_funcinfo << "read msgid=" << msgid << " expected " << messageId() << endl;
	return false;
 }
 return true;
}

bool BosonMessage::copyTo(BosonMessage& copy) const
{
 // AB: we do not reimplement operator=() in every subclass, but still we do not
 // want to depend on the default c++ implementation of operator=(). So we use
 // this indirect way, that is always guaranteed to work.

 if (messageId() != copy.messageId()) {
	boError() << k_funcinfo << "can copy to the same class only" << endl;
	return false;
 }

 QByteArray buffer;
 QDataStream write(buffer, IO_WriteOnly);
 if (!save(write)) {
	boError() << k_funcinfo << "could not save this message" << endl;
	return false;
 }

 QDataStream read(buffer, IO_ReadOnly);

 if (!readMessageId(read)) {
	boError() << k_funcinfo << "could not read messageId from saved message" << endl;
	return false;
 }
 if (!copy.load(read)) {
	boError() << k_funcinfo << "saved message could not be loaded" << endl;
	return false;
 }
 return true;
}

BosonMessageEditorMove::BosonMessageEditorMove()
	: BosonMessage()
{
 mUndo = false;
 mRedo = false;
}

bool BosonMessageEditorMove::saveFlags(QDataStream& stream) const
{
 if (mUndo && mRedo) {
	boError() << k_funcinfo << "undo AND redo flags set. not valid" << endl;
	return false;
 }
 stream << mUndo;
 stream << mRedo;
 return true;
}

bool BosonMessageEditorMove::loadFlags(QDataStream& stream)
{
 stream >> mUndo;
 stream >> mRedo;
 if (mUndo && mRedo) {
	boError() << k_funcinfo << "undo AND redo flags set. not valid" << endl;
	return false;
 }
 return true;
}

bool BosonMessageEditorMove::readMessageId(QDataStream& stream) const
{
 Q_UINT32 editor;
 stream >> editor;
 if (editor != BosonMessageIds::MoveEditor) {
	boError() << k_funcinfo << "not an editor Move message! read: " << editor << " expected: " << BosonMessageIds::MoveEditor << endl;
	return false;
 }
 return BosonMessage::readMessageId(stream);
}

BosonMessageEditorMove* BosonMessageEditorMove::newCopy(const BosonMessageEditorMove& message)
{
 BosonMessageEditorMove* m = 0;
 switch (message.messageId()) {
	case BosonMessageIds::MovePlaceUnit:
		m = new BosonMessageEditorMovePlaceUnit();
		break;
	case BosonMessageIds::MoveChangeTexMap:
		m = new BosonMessageEditorMoveChangeTexMap();
		break;
	case BosonMessageIds::MoveChangeHeight:
		m = new BosonMessageEditorMoveChangeHeight();
		break;
	case BosonMessageIds::MoveDeleteItems:
		m = new BosonMessageEditorMoveDeleteItems();
		break;
	case BosonMessageIds::MoveUndoPlaceUnit:
		m = new BosonMessageEditorMoveUndoPlaceUnit();
		break;
#if 0
	case BosonMessageIds::MoveUndoChangeTexMap:
		m = new BosonMessageEditorMoveUndoChangeTexMap();
		break;
#endif
	case BosonMessageIds::MoveUndoChangeHeight:
		m = new BosonMessageEditorMoveUndoChangeHeight();
		break;
	case BosonMessageIds::MoveUndoDeleteItems:
		m = new BosonMessageEditorMoveUndoDeleteItems();
		break;
	default:
		m = 0;
		break;
 }
 if (!m) {
	boError() << k_funcinfo << "unknown message id " << message.messageId();
	return 0;
 }
 if (!message.copyTo(*m)) {
	boError() << k_funcinfo << "unable to copy message" << endl;
	delete m;
	return 0;
 }
 return m;
}

BosonMessageEditorMovePlaceUnit::BosonMessageEditorMovePlaceUnit()
	: BosonMessageEditorMove()
{
}

BosonMessageEditorMovePlaceUnit::BosonMessageEditorMovePlaceUnit(Q_UINT32 unitType, Q_UINT32 owner, const BoVector2Fixed& pos, const bofixed& rotation)
	: BosonMessageEditorMove(),
	mUnitType(unitType),
	mOwner(owner),
	mPos(pos),
	mRotation(rotation)
{
}

bool BosonMessageEditorMovePlaceUnit::save(QDataStream& stream) const
{
 stream << (Q_UINT32)BosonMessageIds::MoveEditor;
 stream << (Q_UINT32)messageId();
 if (!BosonMessageEditorMove::saveFlags(stream)) {
	return false;
 }
 stream << mUnitType;
 stream << mOwner;
 stream << mPos;
 stream << mRotation;
 return true;
}

bool BosonMessageEditorMovePlaceUnit::load(QDataStream& stream)
{
 // AB: msgid and editor flag have been read already
 if (!BosonMessageEditorMove::loadFlags(stream)) {
	return false;
 }
 stream >> mUnitType;
 stream >> mOwner;
 stream >> mPos;
 stream >> mRotation;
 return true;
}

BosonMessageEditorMoveUndoPlaceUnit::BosonMessageEditorMoveUndoPlaceUnit(Q_ULONG unit, const BosonMessageEditorMovePlaceUnit& message)
	: BosonMessageEditorMove()
{
 setUndo();
 QValueList<Q_ULONG> items;
 items.append(unit);
 BosonMessageEditorMoveDeleteItems del(items);
 if (!del.copyTo(mDeleteUnit)) {
	boError() << k_funcinfo << "could not copy delete message" << endl;
 }

 if (!message.copyTo(mMessage)) {
	boError() << k_funcinfo << "could not copy message" << endl;
 }
}

bool BosonMessageEditorMoveUndoPlaceUnit::save(QDataStream& stream) const
{
 stream << (Q_UINT32)BosonMessageIds::MoveEditor;
 stream << (Q_UINT32)messageId();
 if (!BosonMessageEditorMove::saveFlags(stream)) {
	return false;
 }
 if (!mDeleteUnit.save(stream)) {
	boError() << k_funcinfo << "could not save delete message to stream" << endl;
	return false;
 }
 if (!mMessage.save(stream)) {
	boError() << k_funcinfo << "could not save message to stream" << endl;
	return false;
 }
 return true;
}

bool BosonMessageEditorMoveUndoPlaceUnit::load(QDataStream& stream)
{
 // AB: msgid and editor flag have been read already
 if (!BosonMessageEditorMove::loadFlags(stream)) {
	return false;
 }
 if (!mDeleteUnit.readMessageId(stream)) {
	boError() << k_funcinfo << "could not load delete messages messageId from stream" << endl;
	return false;
 }
 if (!mDeleteUnit.load(stream)) {
	boError() << k_funcinfo << "could not load delete message from stream" << endl;
	return false;
 }

 if (!mMessage.readMessageId(stream)) {
	boError() << k_funcinfo << "could not load messages messageId from stream" << endl;
	return false;
 }
 if (!mMessage.load(stream)) {
	boError() << k_funcinfo << "could not load message from stream" << endl;
	return false;
 }
 return true;
}


BosonMessageEditorMoveChangeTexMap::BosonMessageEditorMoveChangeTexMap()
	: BosonMessageEditorMove()
{
}

BosonMessageEditorMoveChangeTexMap::BosonMessageEditorMoveChangeTexMap(
		const QValueVector<Q_UINT32>& cellCornersX,
		const QValueVector<Q_UINT32>& cellCornersY,
		const QValueVector<Q_UINT32>& cellCornersTexCount,
		const QValueVector< QValueVector<Q_UINT32> > cellCornerTextures,
		const QValueVector< QValueVector<Q_UINT8> > cellCornerAlpha
		)
	: BosonMessageEditorMove(),
	mCellCornersX(cellCornersX),
	mCellCornersY(cellCornersY),
	mCellCornersTextureCount(cellCornersTexCount),
	mCellCornerTextures(cellCornerTextures),
	mCellCornerAlpha(cellCornerAlpha)
{
}

bool BosonMessageEditorMoveChangeTexMap::save(QDataStream& stream) const
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
 if (!BosonMessageEditorMove::saveFlags(stream)) {
	return false;
 }
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
		stream << (Q_UINT8)((mCellCornerAlpha[i])[j]);
	}
 }
 return true;
}

bool BosonMessageEditorMoveChangeTexMap::load(QDataStream& stream)
{
 // AB: msgid and editor flag have been read already
 if (!BosonMessageEditorMove::loadFlags(stream)) {
	return false;
 }
 Q_UINT32 count;
 stream >> count;
 if (count > 65536) {
	boError() << k_funcinfo << "broken message. tried to allocate size for " << count << " corners" << endl;
	return false;
 }
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


BosonMessageEditorMoveChangeHeight::BosonMessageEditorMoveChangeHeight()
	: BosonMessageEditorMove()
{
}

BosonMessageEditorMoveChangeHeight::BosonMessageEditorMoveChangeHeight(
		const QValueVector<Q_UINT32> cellCornersX,
		const QValueVector<Q_UINT32> cellCornersY,
		const QValueVector<bofixed> cellCornersHeight
		)
	: BosonMessageEditorMove(),
	mCellCornersX(cellCornersX),
	mCellCornersY(cellCornersY),
	mCellCornersHeight(cellCornersHeight)
{
}

bool BosonMessageEditorMoveChangeHeight::save(QDataStream& stream) const
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
 if (!BosonMessageEditorMove::saveFlags(stream)) {
	return false;
 }
 stream << (Q_UINT32) mCellCornersX.count();
 for (unsigned int i = 0; i < mCellCornersX.count(); i++) {
	stream << (Q_UINT32)mCellCornersX[i];
	stream << (Q_UINT32)mCellCornersY[i];
	stream << mCellCornersHeight[i];
 }
 return true;
}

bool BosonMessageEditorMoveChangeHeight::load(QDataStream& stream)
{
 // AB: msgid and editor flag have been read already
 if (!BosonMessageEditorMove::loadFlags(stream)) {
	return false;
 }
 Q_UINT32 count;
 stream >> count;
 if (count > 65536) {
	boError() << k_funcinfo << "broken message. tried to allocate size for " << count << " corners" << endl;
	return false;
 }
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

BosonMessageEditorMoveDeleteItems::BosonMessageEditorMoveDeleteItems()
	: BosonMessageEditorMove()
{
}

BosonMessageEditorMoveDeleteItems::BosonMessageEditorMoveDeleteItems(const QValueList<Q_ULONG>& items)
	: BosonMessageEditorMove(),
	mItems(items)
{
}

bool BosonMessageEditorMoveDeleteItems::save(QDataStream& stream) const
{
 stream << (Q_UINT32)BosonMessageIds::MoveEditor;
 stream << (Q_UINT32)messageId();
 if (!BosonMessageEditorMove::saveFlags(stream)) {
	return false;
 }
 stream << (Q_UINT32)mItems.count();
 QValueList<Q_ULONG>::const_iterator it;
 for (it = mItems.begin(); it != mItems.end(); ++it) {
	stream << (Q_ULONG)(*it);
 }
 return true;
}

bool BosonMessageEditorMoveDeleteItems::load(QDataStream& stream)
{
 // AB: msgid and editor flag have been read already
 if (!BosonMessageEditorMove::loadFlags(stream)) {
	return false;
 }
 Q_UINT32 count;
 stream >> count;
 mItems.clear();
 for (Q_UINT32 i = 0; i < count; i++) {
	Q_ULONG item;
	stream >> item;
	mItems.append(item);
 }
 return true;
}

BosonMessageEditorMoveUndoDeleteItems::BosonMessageEditorMoveUndoDeleteItems(
		const QValueList<BosonMessageEditorMovePlaceUnit*>& units,
		const QValueList<QString>& unitsData,
		const BosonMessageEditorMoveDeleteItems& message
	)
	: BosonMessageEditorMove()
{
 setUndo();
 if (!message.copyTo(mMessage)) {
	boError() << k_funcinfo << "could not copy message" << endl;
 }
 QValueList<BosonMessageEditorMovePlaceUnit*>::const_iterator it;
 for (it = units.begin(); it != units.end(); ++it) {
	BosonMessageEditorMovePlaceUnit* u = new BosonMessageEditorMovePlaceUnit();
	if (!(*it)->copyTo(*u)) {
		boError() << k_funcinfo << "could not copy PlaceUnit message" << endl;
		delete u;
		continue;
	}
	mUnits.append(u);
 }
 mUnitsData = unitsData;
}

BosonMessageEditorMoveUndoDeleteItems::~BosonMessageEditorMoveUndoDeleteItems()
{
 clearUnits();
}

void BosonMessageEditorMoveUndoDeleteItems::clearUnits()
{
 for (unsigned int i = 0; i < mUnits.count(); i++) {
	delete mUnits[i];
 }
 mUnits.clear();
 mUnitsData.clear();
}

bool BosonMessageEditorMoveUndoDeleteItems::save(QDataStream& stream) const
{
 stream << (Q_UINT32)BosonMessageIds::MoveEditor;
 stream << (Q_UINT32)messageId();
 if (!BosonMessageEditorMove::saveFlags(stream)) {
	return false;
 }

 stream << (Q_UINT32)mUnits.count();
 for (unsigned int i = 0; i < mUnits.count(); i++) {
	if (!mUnits[i]->save(stream)) {
		boError() << k_funcinfo << "could not save PlaceUnit message to stream" << endl;
		return false;
	}
	stream << mUnitsData[i];
 }

 if (!mMessage.save(stream)) {
	boError() << k_funcinfo << "could not save message to stream" << endl;
	return false;
 }
 return true;
}

bool BosonMessageEditorMoveUndoDeleteItems::load(QDataStream& stream)
{
 // AB: msgid and editor flag have been read already
 clearUnits();
 if (!BosonMessageEditorMove::loadFlags(stream)) {
	return false;
 }

 Q_UINT32 count;
 stream >> count;
 for (Q_UINT32 i = 0; i < count; i++) {
	BosonMessageEditorMovePlaceUnit* u = new BosonMessageEditorMovePlaceUnit();
	if (!u->readMessageId(stream)) {
		delete u;
		boError() << k_funcinfo << "could not load PlaceUnit's messageId from stream" << endl;
		return false;
	}
	if (!u->load(stream)) {
		boError() << k_funcinfo << "could not load PlaceUnit message from stream" << endl;
		delete u;
		return false;
	}
	QString xml;
	stream >> xml;

	mUnits.append(u);
	mUnitsData.append(xml);
 }

 if (!mMessage.readMessageId(stream)) {
	boError() << k_funcinfo << "could not load messages messageId from stream" << endl;
	return false;
 }
 if (!mMessage.load(stream)) {
	boError() << k_funcinfo << "could not load message from stream" << endl;
	return false;
 }
 return true;
}

BosonMessageEditorMoveUndoChangeHeight::BosonMessageEditorMoveUndoChangeHeight(
		const BosonMessageEditorMoveChangeHeight& originalHeights,
		const BosonMessageEditorMoveChangeHeight& message
	)
	: BosonMessageEditorMove()
{
 setUndo();
 if (!originalHeights.copyTo(mOriginalHeights)) {
	boError() << k_funcinfo << "could not copy original heights message" << endl;
 }
 if (!message.copyTo(mMessage)) {
	boError() << k_funcinfo << "could not copy message" << endl;
 }
}

bool BosonMessageEditorMoveUndoChangeHeight::save(QDataStream& stream) const
{
 stream << (Q_UINT32)BosonMessageIds::MoveEditor;
 stream << (Q_UINT32)messageId();
 if (!BosonMessageEditorMove::saveFlags(stream)) {
	return false;
 }

 if (!mOriginalHeights.save(stream)) {
	boError() << k_funcinfo << "could not save original heights message to stream" << endl;
	return false;
 }


 if (!mMessage.save(stream)) {
	boError() << k_funcinfo << "could not save message to stream" << endl;
	return false;
 }
 return true;
}

bool BosonMessageEditorMoveUndoChangeHeight::load(QDataStream& stream)
{
 // AB: msgid and editor flag have been read already
 if (!BosonMessageEditorMove::loadFlags(stream)) {
	return false;
 }

 if (!mOriginalHeights.readMessageId(stream)) {
	boError() << k_funcinfo << "could not load original heights messages messageId from stream" << endl;
	return false;
 }
 if (!mOriginalHeights.load(stream)) {
	boError() << k_funcinfo << "could not load original heights message from stream" << endl;
	return false;
 }

 if (!mMessage.readMessageId(stream)) {
	boError() << k_funcinfo << "could not load messages messageId from stream" << endl;
	return false;
 }
 if (!mMessage.load(stream)) {
	boError() << k_funcinfo << "could not load message from stream" << endl;
	return false;
 }
 return true;
}


BosonMessageMoveMove::BosonMessageMoveMove()
	: BosonMessage()
{
}

BosonMessageMoveMove::BosonMessageMoveMove(bool isAttack, const BoVector2Fixed& pos, const QValueList<Q_ULONG>& items)
	: BosonMessage(),
	mIsAttack(isAttack),
	mPos(pos),
	mItems(items)
{
}

bool BosonMessageMoveMove::save(QDataStream& stream) const
{
 stream << (Q_UINT32)messageId();
 stream << (Q_INT8)mIsAttack;
 stream << mPos;
 stream << (Q_UINT32)mItems.count();
 QValueList<Q_ULONG>::const_iterator it;
 for (it = mItems.begin(); it != mItems.end(); ++it) {
	stream << (Q_ULONG)(*it);
 }
 return true;
}

bool BosonMessageMoveMove::load(QDataStream& stream)
{
 // AB: msgid has been read already
 stream >> mIsAttack;
 stream >> mPos;
 Q_UINT32 count;
 stream >> count;
 mItems.clear();
 for (Q_UINT32 i = 0; i < count; i++) {
	Q_ULONG item;
	stream >> item;
	mItems.append(item);
 }
 return true;
}

bool BosonMessageMoveAttack::save(QDataStream& stream) const
{
 stream << (Q_UINT32)messageId();
 stream << (Q_ULONG)mAttackedUnitId;
 stream << (Q_UINT32)mItems.count();
 QValueList<Q_ULONG>::const_iterator it;
 for (it = mItems.begin(); it != mItems.end(); ++it) {
	stream << (*it);
 }
 return true;
}

bool BosonMessageMoveAttack::load(QDataStream& stream)
{
 // AB: msgid has been read already
 stream >> mAttackedUnitId;
 Q_UINT32 count;
 stream >> count;
 mItems.clear();
 for (Q_UINT32 i = 0; i < count; i++) {
	Q_ULONG item;
	stream >> item;
	mItems.append(item);
 }
 return true;
}

bool BosonMessageMoveStop::save(QDataStream& stream) const
{
 stream << (Q_UINT32)messageId();
 stream << (Q_UINT32)mItems.count();
 QValueList<Q_ULONG>::const_iterator it;
 for (it = mItems.begin(); it != mItems.end(); ++it) {
	stream << (*it);
 }
 return true;
}

bool BosonMessageMoveStop::load(QDataStream& stream)
{
 // AB: msgid has been read already
 Q_UINT32 count;
 stream >> count;
 mItems.clear();
 for (Q_UINT32 i = 0; i < count; i++) {
	Q_ULONG item;
	stream >> item;
	mItems.append(item);
 }
 return true;
}

bool BosonMessageMoveMine::save(QDataStream& stream) const
{
 stream << (Q_UINT32)messageId();
 stream << (Q_ULONG)mHarvesterId;
 stream << (Q_ULONG)mResourceMineId;
 return true;
}

bool BosonMessageMoveMine::load(QDataStream& stream)
{
 // AB: msgid has been read already
 stream >> mHarvesterId;
 stream >> mResourceMineId;
 return true;
}

bool BosonMessageMoveRefine::save(QDataStream& stream) const
{
 stream << (Q_UINT32)messageId();
 stream << (Q_UINT32)mRefineryOwner;
 stream << (Q_ULONG)mRefineryId;
 stream << (Q_UINT32)mItems.count();
 QValueList<Q_ULONG>::const_iterator it;
 for (it = mItems.begin(); it != mItems.end(); ++it) {
	stream << (*it);
 }
 return true;
}

bool BosonMessageMoveRefine::load(QDataStream& stream)
{
 // AB: msgid has been read already
 stream >> mRefineryOwner;
 stream >> mRefineryId;
 Q_UINT32 count;
 stream >> count;
 mItems.clear();
 for (Q_UINT32 i = 0; i < count; i++) {
	Q_ULONG item;
	stream >> item;
	mItems.append(item);
 }
 return true;
}

bool BosonMessageMoveRepair::save(QDataStream& stream) const
{
 stream << (Q_UINT32)messageId();
 return true;
}

bool BosonMessageMoveRepair::load(QDataStream& stream)
{
 // AB: msgid has been read already
 return true;
}

bool BosonMessageMoveProduce::save(QDataStream& stream) const
{
 stream << (Q_UINT32)messageId();
 stream << (Q_UINT32)mProduceType;
 stream << (Q_UINT32)mOwner;
 stream << (Q_ULONG)mFactoryId;
 stream << (Q_UINT32)mType;
 return true;
}

bool BosonMessageMoveProduce::load(QDataStream& stream)
{
 // AB: msgid has been read already
 stream >> mProduceType;
 stream >> mOwner;
 stream >> mFactoryId;
 stream >> mType;
 return true;
}

bool BosonMessageMoveProduceStop::save(QDataStream& stream) const
{
 stream << (Q_UINT32)messageId();
 stream << (Q_UINT32)mProduceType;
 stream << (Q_UINT32)mOwner;
 stream << (Q_ULONG)mFactoryId;
 stream << (Q_UINT32)mType;
 return true;
}

bool BosonMessageMoveProduceStop::load(QDataStream& stream)
{
 // AB: msgid has been read already
 stream >> mProduceType;
 stream >> mOwner;
 stream >> mFactoryId;
 stream >> mType;
 return true;
}

bool BosonMessageMoveBuild::save(QDataStream& stream) const
{
 stream << (Q_UINT32)messageId();
 stream << (Q_UINT32)mProduceType;
 stream << (Q_UINT32)mOwner;
 stream << (Q_ULONG)mFactoryId;
 stream << mPos;
 return true;
}

bool BosonMessageMoveBuild::load(QDataStream& stream)
{
 // AB: msgid has been read already
 stream >> mProduceType;
 stream >> mOwner;
 stream >> mFactoryId;
 stream >> mPos;
 return true;
}

bool BosonMessageMoveFollow::save(QDataStream& stream) const
{
 stream << (Q_UINT32)messageId();
 stream << (Q_UINT32)mFollowUnitId;
 stream << (Q_UINT32)mItems.count();
 QValueList<Q_ULONG>::const_iterator it;
 for (it = mItems.begin(); it != mItems.end(); ++it) {
	stream << (*it);
 }
 return true;
}

bool BosonMessageMoveFollow::load(QDataStream& stream)
{
 // AB: msgid has been read already
 stream >> mFollowUnitId;
 Q_UINT32 count;
 stream >> count;
 mItems.clear();
 for (Q_UINT32 i = 0; i < count; i++) {
	Q_ULONG item;
	stream >> item;
	mItems.append(item);
 }
 return true;
}

bool BosonMessageMoveLayMine::save(QDataStream& stream) const
{
 if (mUnits.count() != mWeapons.count()) {
	boError() << k_funcinfo << "unit count must match weapon count" << endl;
	return false;
 }
 stream << (Q_UINT32)messageId();
 stream << (Q_UINT32)mUnits.count();
 for (unsigned int i = 0; i < mUnits.count(); i++) {
	stream << (Q_ULONG)mUnits[i];
	stream << (Q_ULONG)mWeapons[i];
 }
 return true;
}

bool BosonMessageMoveLayMine::load(QDataStream& stream)
{
 // AB: msgid has been read already
 mUnits.clear();
 mWeapons.clear();
 Q_UINT32 count;
 stream >> count;
 for (Q_UINT32 i = 0; i < count; i++) {
	Q_ULONG unit;
	Q_ULONG weapon;
	stream >> unit;
	stream >> weapon;
	mUnits.append(unit);
	mWeapons.append(weapon);
 }
 return true;
}

bool BosonMessageMoveDropBomb::save(QDataStream& stream) const
{
 if (mUnits.count() != mWeapons.count()) {
	boError() << k_funcinfo << "unit count must match weapon count" << endl;
	return false;
 }
 stream << (Q_UINT32)messageId();
 stream << mPos;
 stream << (Q_UINT32)mUnits.count();
 for (unsigned int i = 0; i < mUnits.count(); i++) {
	stream << (Q_ULONG)mUnits[i];
	stream << (Q_ULONG)mWeapons[i];
 }
 return true;
}

bool BosonMessageMoveDropBomb::load(QDataStream& stream)
{
 // AB: msgid has been read already
 mUnits.clear();
 mWeapons.clear();
 stream >> mPos;
 Q_UINT32 count;
 stream >> count;
 for (Q_UINT32 i = 0; i < count; i++) {
	Q_ULONG unit;
	Q_ULONG weapon;
	stream >> unit;
	stream >> weapon;
	mUnits.append(unit);
	mWeapons.append(weapon);
 }
 return true;
}

bool BosonMessageMoveTeleport::save(QDataStream& stream) const
{
 stream << (Q_UINT32)messageId();
 stream << (Q_ULONG)mUnitId;
 stream << (Q_UINT32)mOwner;
 stream << mPos;
 return true;
}

bool BosonMessageMoveTeleport::load(QDataStream& stream)
{
 // AB: msgid has been read already
 stream >> mUnitId;
 stream >> mOwner;
 stream >> mPos;
 return true;
}

bool BosonMessageMoveRotate::save(QDataStream& stream) const
{
 stream << (Q_UINT32)messageId();
 stream << (Q_ULONG)mUnitId;
 stream << (Q_UINT32)mOwner;
 stream << mRotate;
 return true;
}

bool BosonMessageMoveRotate::load(QDataStream& stream)
{
 // AB: msgid has been read already
 stream >> mUnitId;
 stream >> mOwner;
 stream >> mRotate;
 return true;
}

