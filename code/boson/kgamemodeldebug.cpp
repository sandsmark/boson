/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "kgamemodeldebug.h"
#include "kgamemodeldebug.moc"

#include "bo3dtools.h"
#include "bomatrixwidget.h"
#include "bodebug.h"

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qmap.h>
#include <qintdict.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qptrdict.h>
#include <qvgroupbox.h>
#include <qgrid.h>
#include <qheader.h>
#include <qsplitter.h>
#include <qvbox.h>
#include <qstringlist.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qdir.h>
#include <qptrqueue.h>

#include <ksimpleconfig.h>
#include <klistbox.h>
#include <klistview.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <knuminput.h>

#include <lib3ds/file.h>
#include <lib3ds/node.h>
#include <lib3ds/mesh.h>
#include <lib3ds/material.h>
#include <lib3ds/vector.h>
#include <lib3ds/matrix.h>
#include <lib3ds/quat.h>

#include <math.h>

// not working anyway and just adds a dependancy to bo3dsload
#define ALLOW_FACE_CONNECTIONS 0

#if ALLOW_FACE_CONNECTIONS
#include "bo3dsload.h"
#endif

/**
 * AB: TCB stands for Tension-Continuity-Bias. Whatever that may be. See
 * http://www.scriptspot.com/bobo/rendfaq/ACRONYM.HTM for a (very) short
 * explanation.
 *
 * This class is simply a c++ implementation of the lib3ds struct.
 **/
class Bo3DSTrackTCB
{
public:
	Bo3DSTrackTCB()
	{
		mFrame = 0;
		mFlags = 0;
		mTens = 0.0f;
		mCont = 0.0f;
		mBias = 0.0f;
		mEaseTo = 0.0f;
		mEaseFrom = 0.0f;
	}
	~Bo3DSTrackTCB()
	{
	}
	void copyTCB(const Lib3dsTcb& tcb)
	{
		mFrame = tcb.frame;
		mFlags = tcb.flags;
		mTens = tcb.tens;
		mCont = tcb.cont;
		mBias = tcb.bias;
		mEaseTo = tcb.ease_to;
		mEaseFrom = tcb.ease_from;
	}

	long int mFrame; // AB: redundant. we store it as the position in the list already.
	unsigned short int mFlags;
	float mTens; // AB: probably short for tension
	float mCont; // AB: probably short for continuity
	float mBias;
	float mEaseTo;
	float mEaseFrom;
};


class Bo3DSTrackKey
{
public:
	class TrackKeyData {
	public:
		TrackKeyData() { }
		virtual ~TrackKeyData() { }
	};

	/**
	 * AB: no need to store anything beyond the default TCB in here.
	 * A bool tracks works this way: the initial value is predefined.
	 * whenever a bool key appears, the value is flipped. So for example
	 * you want to know the value at frame 14 out of 100 frames. You
	 * start with false (as it is predefined). There are keys at 
	 * 1,4,7,9,55,78 and 99. When you come to the key at 1, the value is
	 * flipped, i.e. is true now. At 4 it becomes false again, at 7 it will
	 * be true and at 9 it goes back to false. Here we stop, as 55
	 * is > 14. So false is the value we search.
	 **/
	class TrackKeyDataBool : public TrackKeyData {
	public:
		TrackKeyDataBool() : TrackKeyData() { }
	};

	class TrackKeyDataLin1 : public TrackKeyData {
	public:
		TrackKeyDataLin1() : TrackKeyData() { }
		float value;
		float dd;
		float ds;
	};

	class TrackKeyDataLin3 : public TrackKeyData {
	public:
		TrackKeyDataLin3() : TrackKeyData() { }
		BoVector3Float value;
		BoVector3Float dd;
		BoVector3Float ds;
	};

	class TrackKeyDataQuat : public TrackKeyData {
	public:
		TrackKeyDataQuat() : TrackKeyData() { }
		BoVector3Float axis;
		float angle;
		BoQuaternion q;
		BoQuaternion dd;
		BoQuaternion ds;
	};

	class TrackKeyDataMorph : public TrackKeyData {
	public:
		TrackKeyDataMorph() : TrackKeyData() { }
		QString name;
	};

	enum Type {
		TrackBool = 0,
		TrackLin1 = 1,
		TrackLin3 = 2,
		TrackQuat = 3,
		TrackMorph = 4
	};

public:
	Bo3DSTrackKey(int type)
	{
		mData = 0;
		switch (type) {
			case TrackBool:
				mData = (TrackKeyData*)new TrackKeyDataBool();
				break;
			case TrackLin1:
				mData = (TrackKeyData*)new TrackKeyDataLin1();
				break;
			case TrackLin3:
				mData = (TrackKeyData*)new TrackKeyDataLin3();
				break;
			case TrackQuat:
				mData = (TrackKeyData*)new TrackKeyDataQuat();
				break;
			case TrackMorph:
				mData = (TrackKeyData*)new TrackKeyDataMorph();
				break;
			default:
				boError() << k_funcinfo << "invalid type " << type << endl;
				break;
		}
	}
	~Bo3DSTrackKey()
	{
		delete mData;
	}

	TrackKeyDataBool* boolData() const { return (TrackKeyDataBool*)mData; }
	TrackKeyDataLin1* lin1Data() const { return (TrackKeyDataLin1*)mData; }
	TrackKeyDataLin3* lin3Data() const { return (TrackKeyDataLin3*)mData; }
	TrackKeyDataQuat* quatData() const { return (TrackKeyDataQuat*)mData; }
	TrackKeyDataMorph* morphData() const { return (TrackKeyDataMorph*)mData; }

	const Bo3DSTrackTCB& tcb() const
	{
		return mTCB;
	}

	Bo3DSTrackTCB mTCB;

private:
	TrackKeyData* mData;
};

class Bo3DSTrack
{
public:
	Bo3DSTrack()
	{
		mFlags = 0;
		mKeys.setAutoDelete(true);
	}
	virtual ~Bo3DSTrack()
	{
	}

	void removeKey(int frame)
	{
		mKeys.remove(frame);
	}

	bool haveKey(int frame) const
	{
		return (mKeys.find(frame) != 0);
	}
	unsigned long int flags() const
	{
		return mFlags;
	}
	unsigned int keyCount() const
	{
		return mKeys.count();
	}

	/**
	 * @return Of which type the keys in this class are. See @ref
	 * Bo3DSTrackKey::Type
	 **/
	virtual int type() const = 0;

	/**
	 * Insert a key of the correct type. Must be implemented by derived
	 * classes (you can use @ref insertKeyType)
	 **/
	Bo3DSTrackKey* insertKey(int frame)
	{
		if (haveKey(frame)) {
			removeKey(frame);
		}
		Bo3DSTrackKey* key = createKey();
		mKeys.insert(frame, key);
		return key;
	}

	void clear()
	{
		mFlags = 0x00;
		mKeys.clear();
	}
	const QIntDict<Bo3DSTrackKey>& keys() const
	{
		return mKeys;
	}

protected:
	virtual Bo3DSTrackKey* createKey() = 0;

protected:
	unsigned long int mFlags;

	// AB: lib3ds uses a simple linked list instead of a map. this is a very
	// bad choice and they have to do a lot of additional code because of
	// this, but they don't have a choice, as they use plain c - but we do
	// have a choice :)
	QIntDict<Bo3DSTrackKey> mKeys;
};

// AB: a Lin1 track stores 3 floats in every key. yet have to find out the exact
// semantics.
class Bo3DSTrackLin1 : public Bo3DSTrack
{
public:
	Bo3DSTrackLin1() : Bo3DSTrack()
	{
	}
	Bo3DSTrackLin1(const Lib3dsLin1Track& track) : Bo3DSTrack()
	{
		loadTrack(track);
	}
	void loadTrack(const Lib3dsLin1Track& track)
	{
		clear();
		mFlags = track.flags;
		Lib3dsLin1Key* key = track.keyL;
		while (key) {
			copyKey(key);
			key = key->next;
		}
	}
	virtual int type() const { return (int)Bo3DSTrackKey::TrackLin1; }

	float eval(float t) const
	{
		return 0.0f;
	}

protected:
	virtual Bo3DSTrackKey* createKey()
	{
		return new Bo3DSTrackKey(Bo3DSTrackKey::TrackLin1);
	}
	void copyKey(Lib3dsLin1Key* _key)
	{
		BO_CHECK_NULL_RET(_key);
		Bo3DSTrackKey* key = insertKey(_key->tcb.frame);
		key->mTCB.copyTCB(_key->tcb);
		key->lin1Data()->value = _key->value;
		key->lin1Data()->dd = _key->dd;
		key->lin1Data()->ds = _key->ds;
	}
};

// AB: a Lin3 track stores 3 vectors in every key. yet have to find out the exact
// semantics.
class Bo3DSTrackLin3 : public Bo3DSTrack
{
public:
	Bo3DSTrackLin3() : Bo3DSTrack()
	{
	}

	/**
	 * Construct a Bo3DSTrackLin3 object and copy the data from @p track.
	 **/
	Bo3DSTrackLin3(const Lib3dsLin3Track& track) : Bo3DSTrack()
	{
		loadTrack(track);
	}

	void loadTrack(const Lib3dsLin3Track& track)
	{
		clear();
		mFlags = track.flags;
		Lib3dsLin3Key* key = track.keyL;
		while (key) {
			copyKey(key);
			key = key->next;
		}
	}
	virtual int type() const { return (int)Bo3DSTrackKey::TrackLin3; }

	BoVector3Float eval(float t) const
	{
		return BoVector3Float();
	}

protected:
	virtual Bo3DSTrackKey* createKey()
	{
		return new Bo3DSTrackKey(Bo3DSTrackKey::TrackLin3);
	}

	void copyKey(Lib3dsLin3Key* _key)
	{
		BO_CHECK_NULL_RET(_key);
		Bo3DSTrackKey* key = insertKey(_key->tcb.frame);
		key->mTCB.copyTCB(_key->tcb);
		key->lin3Data()->value.set(_key->value);
		key->lin3Data()->dd.set(_key->dd);
		key->lin3Data()->ds.set(_key->ds);
	}

};

class Bo3DSTrackBool : public Bo3DSTrack
{
public:
	Bo3DSTrackBool() : Bo3DSTrack()
	{
	}
	Bo3DSTrackBool(const Lib3dsBoolTrack& track) : Bo3DSTrack()
	{
		loadTrack(track);
	}
	void loadTrack(const Lib3dsBoolTrack& track)
	{
		clear();
		mFlags = track.flags;
		Lib3dsBoolKey* key = track.keyL;
		while (key) {
			copyKey(key);
			key = key->next;
		}
	}
	virtual int type() const { return (int)Bo3DSTrackKey::TrackBool; }

	bool eval(float t) const
	{
		if (mKeys.isEmpty()) {
			return false;
		}
		if (mKeys.count() == 1) {
			return true;
		}
		bool result = false;

		// TODO dummy implementation only!
#if 0
		QIntDictIterator<Bo3DSTrackKey> it(mKeys);
		while (t < (float)it.currentKey()) {
			result = !result;
			++it;
			if (!it.current()) {
				return result;
			}
		}
#endif


		return result;
	}

protected:
	virtual Bo3DSTrackKey* createKey()
	{
		return new Bo3DSTrackKey(Bo3DSTrackKey::TrackBool);
	}
	void copyKey(Lib3dsBoolKey* _key)
	{
		BO_CHECK_NULL_RET(_key);
		Bo3DSTrackKey* key = insertKey(_key->tcb.frame);
		key->mTCB.copyTCB(_key->tcb);
		// nothing else to copy in a bool track.
	}

};

// AB: a "Morph" track stores an array of 64 chars ("name") in every key. since
// that array is named "name", it is probably meant as a string.
// i have not the least idea what the semantics might be.
class Bo3DSTrackMorph: public Bo3DSTrack
{
public:
	Bo3DSTrackMorph() : Bo3DSTrack()
	{
	}
	Bo3DSTrackMorph(const Lib3dsMorphTrack& track) : Bo3DSTrack()
	{
		loadTrack(track);
	}
	void loadTrack(const Lib3dsMorphTrack& track)
	{
		clear();
		mFlags = track.flags;
		Lib3dsMorphKey* key = track.keyL;
		while (key) {
			copyKey(key);
			key = key->next;
		}
	}
	virtual int type() const { return (int)Bo3DSTrackKey::TrackMorph; }

	QString eval(float t) const
	{
		return QString::null;
	}

protected:
	virtual Bo3DSTrackKey* createKey()
	{
		return new Bo3DSTrackKey(Bo3DSTrackKey::TrackMorph);
	}
	void copyKey(Lib3dsMorphKey* _key)
	{
		BO_CHECK_NULL_RET(_key);
		Bo3DSTrackKey* key = insertKey(_key->tcb.frame);
		key->mTCB.copyTCB(_key->tcb);
		key->morphData()->name = _key->name;
	}

};

class Bo3DSTrackQuat : public Bo3DSTrack
{
public:
	Bo3DSTrackQuat() : Bo3DSTrack()
	{
	}
	Bo3DSTrackQuat(const Lib3dsQuatTrack& track) : Bo3DSTrack()
	{
		loadTrack(track);
	}
	void loadTrack(const Lib3dsQuatTrack& track)
	{
		clear();
		mFlags = track.flags;
		Lib3dsQuatKey* key = track.keyL;
		while (key) {
			copyKey(key);
			key = key->next;
		}
	}
	virtual int type() const { return (int)Bo3DSTrackKey::TrackQuat; }

	BoQuaternion eval(float t) const
	{
		return BoQuaternion();
	}

protected:
	virtual Bo3DSTrackKey* createKey()
	{
		return new Bo3DSTrackKey(Bo3DSTrackKey::TrackQuat);
	}
	void copyKey(Lib3dsQuatKey* _key)
	{
		BO_CHECK_NULL_RET(_key);
		Bo3DSTrackKey* key = insertKey(_key->tcb.frame);
		key->mTCB.copyTCB(_key->tcb);
		key->quatData()->axis.set(_key->axis);
		key->quatData()->angle = _key->angle;
		key->quatData()->q.set(_key->q[3], BoVector3Float(_key->q));
		key->quatData()->dd.set(_key->dd[3], BoVector3Float(_key->dd));
		key->quatData()->ds.set(_key->ds[3], BoVector3Float(_key->ds));
	}
};

BoNodeTracksWidget::BoNodeTracksWidget(QWidget* parent) : QWidget(parent)
{
 mPositionTrack = new Bo3DSTrackLin3;
 mRotationTrack = new Bo3DSTrackQuat;
 mScaleTrack = new Bo3DSTrackLin3;
 mMorphTrack = new Bo3DSTrackMorph;
 mHideTrack = new Bo3DSTrackBool;

 QVBoxLayout* l = new QVBoxLayout(this);
 QGrid* grid = new QGrid(2, this);
 l->addWidget(grid);
 mPosition = new QPushButton(i18n("Position track"), grid);
 mPosition->setToggleButton(true);
 mPositionLabel = new QLabel(grid);

 mRotation = new QPushButton(i18n("Rotation track"), grid);
 QToolTip::add(mRotation, i18n("Note: the .3ds file stores the axis and the angle values only. Not the actual quaternion"));
 mRotation->setToggleButton(true);
 mRotationLabel = new QLabel(grid);

 mScale = new QPushButton(i18n("Scale track"), grid);
 mScale->setToggleButton(true);
 mScaleLabel = new QLabel(grid);

 mMorph = new QPushButton(i18n("Morph track"), grid);
 mMorph->setToggleButton(true);
 mMorphLabel = new QLabel(grid);

 mHide = new QPushButton(i18n("Hide track"), grid);
 mHide->setToggleButton(true);
 mHideLabel = new QLabel(grid);

 mButton2Track.insert(mPosition, mPositionTrack);
 mButton2Track.insert(mRotation, mRotationTrack);
 mButton2Track.insert(mScale, mScaleTrack);
 mButton2Track.insert(mMorph, mMorphTrack);
 mButton2Track.insert(mHide, mHideTrack);

 connect(mPosition, SIGNAL(toggled(bool)), this, SLOT(slotButtonToggled(bool)));
 connect(mRotation, SIGNAL(toggled(bool)), this, SLOT(slotButtonToggled(bool)));
 connect(mScale, SIGNAL(toggled(bool)), this, SLOT(slotButtonToggled(bool)));
 connect(mMorph, SIGNAL(toggled(bool)), this, SLOT(slotButtonToggled(bool)));
 connect(mHide, SIGNAL(toggled(bool)), this, SLOT(slotButtonToggled(bool)));

 setNodeObjectData(0);
}

void BoNodeTracksWidget::slotButtonToggled(bool on)
{
 emit signalDisplayTrack(0);
 if (!on) {
	return;
 }
 QPushButton* b = (QPushButton*)sender();
 if (!b) {
	return;
 }
 Bo3DSTrack* track = mButton2Track[b];
 if (!track) {
	return;
 }
 static bool recursive = false;
 if (recursive) {
	return;
 }
 recursive = true;

 // set all off, then re-enable b
 mPosition->setOn(false);
 mRotation->setOn(false);
 mScale->setOn(false);
 mMorph->setOn(false);
 mHide->setOn(false);
 b->setOn(true);

 emit signalDisplayTrack(track);

 recursive = false;
}

void BoNodeTracksWidget::setNodeObjectData(Lib3dsObjectData* d)
{
 // AB: the data here won't change for different frames. it's always the same in
 // every frame of a node.
 mPositionTrack->clear();
 mRotationTrack->clear();
 mScaleTrack->clear();
 mHideTrack->clear();
 mMorphTrack->clear();
 if (d) {
	mPositionTrack->loadTrack(d->pos_track);
	mRotationTrack->loadTrack(d->rot_track);
	mScaleTrack->loadTrack(d->scl_track);
	mHideTrack->loadTrack(d->hide_track);
	mMorphTrack->loadTrack(d->morph_track);
 } else {
	emit signalDisplayTrack(0);
 }
 mPositionLabel->setText(i18n("Flag: %1 Key Number: %2").arg(mPositionTrack->flags()).arg(mPositionTrack->keyCount()));
 mRotationLabel->setText(i18n("Flag: %1 Key Number: %2").arg(mRotationTrack->flags()).arg(mRotationTrack->keyCount()));
 mScaleLabel->setText(i18n("Flag: %1 Key Number: %2").arg(mScaleTrack->flags()).arg(mScaleTrack->keyCount()));
 mHideLabel->setText(i18n("Flag: %1 Key Number: %2").arg(mHideTrack->flags()).arg(mHideTrack->keyCount()));
 mMorphLabel->setText(i18n("Flag: %1 Key Number: %2").arg(mMorphTrack->flags()).arg(mMorphTrack->keyCount()));

 if (mPosition->isOn()) {
	mPosition->toggle();
	mPosition->toggle();
 }
 if (mRotation->isOn()) {
	mRotation->toggle();
	mRotation->toggle();
 }
 if (mScale->isOn()) {
	mScale->toggle();
	mScale->toggle();
 }
 if (mHide->isOn()) {
	mHide->toggle();
	mHide->toggle();
 }
 if (mMorph->isOn()) {
	mMorph->toggle();
	mMorph->toggle();
 }

}

BoTrackWidget::BoTrackWidget(QWidget* parent) : QWidget(parent)
{
 QVBoxLayout* l = new QVBoxLayout(this);

 QHBox* hbox = new QHBox(this);
 l->addWidget(hbox);
 (void)new QLabel(i18n("Flags: "), hbox);
 mFlags = new QLabel(hbox);
 (void)new QLabel(i18n("Type: "), hbox);
 mType = new QLabel(hbox);

 mFlagList = new KListBox(this);
 l->addWidget(mFlagList);
 l->addStretch(0);

 QLabel* keyLabel = new QLabel(i18n("Keys"), this);
 l->addWidget(keyLabel);
 mKeys = new KListView(this);
 mKeys->setAllColumnsShowFocus(true);
 QFontMetrics metrics(font());
 mKeys->addColumn(i18n("Frame"));
 mKeys->addColumn(i18n("Flags"), metrics.width(QString::number(11))); // number
 mKeys->addColumn(i18n("Flags"), metrics.width(QString::number(11))); // strings
 mKeys->addColumn(i18n("Tension"), metrics.width(QString::number(11)));
 mKeys->addColumn(i18n("Continuity"), metrics.width(QString::number(11)));
 mKeys->addColumn(i18n("Bias"), metrics.width(QString::number(11)));
 mKeys->addColumn(i18n("Ease To"), metrics.width(QString::number(11)));
 mKeys->addColumn(i18n("Ease From"), metrics.width(QString::number(11)));

 mKeyData0 = mKeys->addColumn(i18n("Key Data 1"));
 mKeyData1 = mKeys->addColumn(i18n("Key Data 2"));
 mKeyData2 = mKeys->addColumn(i18n("Key Data 3"));
 mKeyData3 = mKeys->addColumn(i18n("Key Data 4"));
 mKeyData4 = mKeys->addColumn(i18n("Key Data 5"));

 l->addWidget(mKeys, 1);
}

void BoTrackWidget::slotDisplayTrack(Bo3DSTrack* track)
{
 mFlagList->clear();
 mKeys->clear();
 mFlags->setText(QString(""));
 mType->setText(i18n("<none>"));

 mKeys->setColumnText(mKeyData0, QString(""));
 mKeys->setColumnText(mKeyData1, QString(""));
 mKeys->setColumnText(mKeyData2, QString(""));
 mKeys->setColumnText(mKeyData3, QString(""));
 mKeys->setColumnText(mKeyData4, QString(""));

 if (track) {
	unsigned long int flags = track->flags();
	mFlags->setText(QString::number(flags));
	if (flags != 0x00) {
		mFlagList->show();
		unsigned long int all = 0x00;
		if (flags & LIB3DS_REPEAT) {
			(void)new QListBoxText(mFlagList, QString::fromLatin1("LIB3DS_REPEAT"));
			all |= LIB3DS_REPEAT;
		}
		if (flags & LIB3DS_SMOOTH) {
			(void)new QListBoxText(mFlagList, QString::fromLatin1("LIB3DS_SMOOTH"));
			all |= LIB3DS_SMOOTH;
		}
		if (flags & LIB3DS_LOCK_X) {
			(void)new QListBoxText(mFlagList, QString::fromLatin1("LIB3DS_LOCK_X"));
			all |= LIB3DS_LOCK_X;
		}
		if (flags & LIB3DS_LOCK_Y) {
			(void)new QListBoxText(mFlagList, QString::fromLatin1("LIB3DS_LOCK_Y"));
			all |= LIB3DS_LOCK_Y;
		}
		if (flags & LIB3DS_LOCK_Z) {
			(void)new QListBoxText(mFlagList, QString::fromLatin1("LIB3DS_LOCK_Z"));
			all |= LIB3DS_LOCK_Z;
		}
		if (flags & LIB3DS_UNLINK_X) {
			(void)new QListBoxText(mFlagList, QString::fromLatin1("LIB3DS_UNLINK_X"));
			all |= LIB3DS_UNLINK_X;
		}
		if (flags & LIB3DS_UNLINK_Y) {
			(void)new QListBoxText(mFlagList, QString::fromLatin1("LIB3DS_UNLINK_Y"));
			all |= LIB3DS_UNLINK_Y;
		}
		if (flags & LIB3DS_UNLINK_Z) {
			(void)new QListBoxText(mFlagList, QString::fromLatin1("LIB3DS_UNLINK_Z"));
			all |= LIB3DS_UNLINK_Z;
		}
		if ((flags | all) != all) {
			(void)new QListBoxText(mFlagList, i18n("Some flags not recognized! Remaining: %1").arg((flags | all) ^ all));
		}
	} else {
		mFlagList->hide();
	}

	switch (track->type()) {
		case Bo3DSTrackKey::TrackLin1:
		{
			mType->setText(i18n("Lin1 (1 float)"));
			// AB: once we found out what the values mean, we should find
			// usable names
			mKeys->setColumnText(mKeyData0, i18n("value"));
			mKeys->setColumnText(mKeyData1, i18n("dd"));
			mKeys->setColumnText(mKeyData2, i18n("ds"));
			break;
		}
		case Bo3DSTrackKey::TrackLin3:
		{
			mType->setText(i18n("Lin3 (3 floats)"));
			mKeys->setColumnText(mKeyData0, i18n("value"));
			mKeys->setColumnText(mKeyData1, i18n("dd"));
			mKeys->setColumnText(mKeyData2, i18n("ds"));
			break;
		}
		case Bo3DSTrackKey::TrackBool:
		{
			// no data to be displayed
			mType->setText(i18n("Bool"));
			break;
		}
		case Bo3DSTrackKey::TrackMorph:
		{
			mType->setText(i18n("Morph"));
			mKeys->setColumnText(mKeyData0, i18n("name"));
			break;
		}
		case Bo3DSTrackKey::TrackQuat:
		{
			mType->setText(i18n("Quaternion"));
			mKeys->setColumnText(mKeyData0, i18n("axis"));
			mKeys->setColumnText(mKeyData1, i18n("angle"));
			mKeys->setColumnText(mKeyData2, i18n("q"));
			mKeys->setColumnText(mKeyData3, i18n("dd"));
			mKeys->setColumnText(mKeyData4, i18n("ds"));
			break;
		}
		default:
			boError() << k_funcinfo << "unknown type " << track->type() << endl;
			break;
	}

	QIntDictIterator<Bo3DSTrackKey> it(track->keys());
	while (it.current()) {
		QListViewItem* item = createItem(it.current(), track->type());
		++it;
	}
 }
}

QListViewItem* BoTrackWidget::createItem(Bo3DSTrackKey* key, int type)
{
 BO_CHECK_NULL_RET0(key);
 QListViewItem* item = new QListViewItem(mKeys);
 configureTCB(item, key->tcb());
 configureKey(item, key, type);
 return item;
}

void BoTrackWidget::configureTCB(QListViewItem* item, const Bo3DSTrackTCB& tcb)
{
 BO_CHECK_NULL_RET(item);
 item->setText(0, QString("%1").arg(tcb.mFrame, 3));
 item->setText(1, QString::number(tcb.mFlags));
 if (tcb.mFlags != 0x00) {
	QString flags;
	if (tcb.mFlags & LIB3DS_USE_TENSION) {
		flags += "LIB3DS_USE_TENSION";
	}
	if (tcb.mFlags & LIB3DS_USE_CONTINUITY) {
		flags += " LIB3DS_USE_CONTINUITY";
	}
	if (tcb.mFlags & LIB3DS_USE_BIAS) {
		flags += " LIB3DS_USE_BIAS";
	}
	if (tcb.mFlags & LIB3DS_USE_EASE_TO) {
		flags += " LIB3DS_USE_EASE_TO";
	}
	if (tcb.mFlags & LIB3DS_USE_EASE_FROM) {
		flags += " LIB3DS_USE_EASE_FROM";
	}
	item->setText(2, flags);
 }
 item->setText(3, QString::number(tcb.mTens));
 item->setText(4, QString::number(tcb.mCont));
 item->setText(5, QString::number(tcb.mBias));
 item->setText(6, QString::number(tcb.mEaseTo));
 item->setText(7, QString::number(tcb.mEaseFrom));
}

void BoTrackWidget::configureKey(QListViewItem* item, Bo3DSTrackKey* key, int type)
{
 BO_CHECK_NULL_RET(item);
 BO_CHECK_NULL_RET(key);
 const int prec = 3;
 switch (type) {
	case Bo3DSTrackKey::TrackLin1:
	{
		item->setText(mKeyData0, QString::number(key->lin1Data()->value, 'f', prec));
		item->setText(mKeyData1, QString::number(key->lin1Data()->dd, 'f', prec));
		item->setText(mKeyData2, QString::number(key->lin1Data()->ds, 'f', prec));
		break;
	}
	case Bo3DSTrackKey::TrackLin3:
	{
		item->setText(mKeyData0, debugStringVector(key->lin3Data()->value, prec));
		item->setText(mKeyData1, debugStringVector(key->lin3Data()->dd, prec));
		item->setText(mKeyData2, debugStringVector(key->lin3Data()->ds, prec));
		break;
	}
	case Bo3DSTrackKey::TrackBool:
	{
		// no data to be displayed
		break;
	}
	case Bo3DSTrackKey::TrackMorph:
	{
		item->setText(mKeyData0, key->morphData()->name);
		break;
	}
	case Bo3DSTrackKey::TrackQuat:
	{
		item->setText(mKeyData0, debugStringVector(key->quatData()->axis, prec));
		item->setText(mKeyData1, QString::number(key->quatData()->angle, 'f', prec));
		item->setText(mKeyData2, debugStringQuaternion(key->quatData()->q, prec));
		item->setText(mKeyData3, debugStringQuaternion(key->quatData()->dd, prec));
		item->setText(mKeyData4, debugStringQuaternion(key->quatData()->ds, prec));
		break;
	}
	default:
		boError() << k_funcinfo << "unknown type " << type << endl;
		return;
 }
}

class BoMaterialDataWidget : public QWidget
{
public:
	BoMaterialDataWidget(QWidget* parent) : QWidget(parent)
	{
		QVBoxLayout* l = new QVBoxLayout(this);
		QGrid* grid = new QGrid(2, this);
		l->addWidget(grid);

		(void)new QLabel(i18n("Ambient"), grid);
		mAmbient = new QLabel(grid);

		(void)new QLabel(i18n("Diffuse"), grid);
		mDiffuse = new QLabel(grid);
		
		(void)new QLabel(i18n("Specular"), grid);
		mSpecular = new QLabel(grid);

		(void)new QLabel(i18n("Shininess"), grid);
		mShininess = new QLabel(grid);
		
		(void)new QLabel(i18n("Shin_Strength"), grid);
		mShinStrength = new QLabel(grid);

		(void)new QLabel(i18n("Use Blur"), grid);
		mUseBlur = new QCheckBox(grid);
		mUseBlur->setEnabled(false);

		(void)new QLabel(i18n("Blur"), grid);
		mBlur = new QLabel(grid);

		(void)new QLabel(i18n("Transparency"), grid);
		mTransparency = new QLabel(grid);

		(void)new QLabel(i18n("Falloff"), grid);
		mFalloff = new QLabel(grid);

		(void)new QLabel(i18n("Additive"), grid);
		mAdditive = new QCheckBox(grid);
		mAdditive->setEnabled(false);

		(void)new QLabel(i18n("Use Falloff"), grid);
		mUseFalloff = new QCheckBox(grid);
		mUseFalloff->setEnabled(false);

		(void)new QLabel(i18n("Self_Illum"), grid);
		mSelfIllum = new QCheckBox(grid);
		mSelfIllum->setEnabled(false);

		(void)new QLabel(i18n("Shading"), grid);
		mShading = new QLabel(grid);

		(void)new QLabel(i18n("Soften"), grid);
		mSoften = new QCheckBox(grid);
		mSoften->setEnabled(false);

		(void)new QLabel(i18n("Face_Map"), grid);
		mFaceMap = new QCheckBox(grid);
		mFaceMap->setEnabled(false);

		(void)new QLabel(i18n("Two Sided"), grid);
		mTwoSided = new QCheckBox(grid);
		mTwoSided->setEnabled(false);

		(void)new QLabel(i18n("Map_Decal"), grid);
		mMapDecal = new QCheckBox(grid);
		mMapDecal->setEnabled(false);

		(void)new QLabel(i18n("Use Wire"), grid);
		mUseWire = new QCheckBox(grid);
		mUseWire->setEnabled(false);

		(void)new QLabel(i18n("Use Wire_Abs"), grid);
		mUseWireAbs = new QCheckBox(grid);
		mUseWireAbs->setEnabled(false);

		(void)new QLabel(i18n("Wire Size"), grid);
		mWireSize = new QLabel(grid);
	}

	~BoMaterialDataWidget()
	{
	}

	void setMaterial(Lib3dsMaterial *m)
	{
		if (m) {
			mAmbient->setText(KGameModelDebug::rgbaText(m->ambient));
			mDiffuse->setText(KGameModelDebug::rgbaText(m->diffuse));
			mSpecular->setText(KGameModelDebug::rgbaText(m->specular));
			mShininess->setText(QString::number(m->shininess));
			mShinStrength->setText(QString::number(m->shin_strength));
			mUseBlur->setChecked(m->use_blur);
			mBlur->setText(QString::number(m->blur));
			mTransparency->setText(QString::number(m->transparency));
			mFalloff->setText(QString::number(m->falloff));
			mAdditive->setChecked(m->additive);
			mUseFalloff->setChecked(m->use_falloff);
			mSelfIllum->setChecked(m->self_illum);
			mShading->setText(QString::number(m->shading) + i18n(" (%1)").arg(shadingText(m->shading)));
			mSoften->setChecked(m->soften);
			mFaceMap->setChecked(m->face_map);
			mTwoSided->setChecked(m->two_sided);
			mMapDecal->setChecked(m->map_decal);
			mUseWire->setChecked(m->use_wire);
			mUseWireAbs->setChecked(m->use_wire_abs);
			mWireSize->setText(QString::number(m->wire_size));
		} else {
			mAmbient->setText("");
			mDiffuse->setText("");
			mSpecular->setText("");
			mShininess->setText("");
			mShinStrength->setText("");
			mUseBlur->setChecked(false);
			mBlur->setText("");
			mTransparency->setText("");
			mFalloff->setText("");
			mAdditive->setChecked(false);
			mUseFalloff->setChecked(false);
			mSelfIllum->setChecked(false);
			mShading->setText("");
			mSoften->setChecked(false);
			mFaceMap->setChecked(false);
			mTwoSided->setChecked(false);
			mMapDecal->setChecked(false);
			mUseWire->setChecked(false);
			mUseWireAbs->setChecked(false);
			mWireSize->setText("");
		}
	}

protected:
	QString shadingText(int s) const
	{
		switch (s) {
			case LIB3DS_WIRE_FRAME:
				return QString("WIRE_FRAME");
			case LIB3DS_FLAT:
				return QString("FLAT");
			case LIB3DS_GOURAUD:
				return QString("GOURAUD");
			case LIB3DS_PHONG:
				return QString("PHONG");
			case LIB3DS_METAL:
				return QString("METAL");
			default:
				break;
		}
		return i18n("Unknown");
	}

private:
	QLabel* mAmbient;
	QLabel* mDiffuse;
	QLabel* mSpecular;
	QLabel* mShininess;
	QLabel* mShinStrength;
	QCheckBox* mUseBlur;
	QLabel* mBlur;
	QLabel* mTransparency;
	QLabel* mFalloff;
	QCheckBox* mAdditive;
	QCheckBox* mUseFalloff;
	QCheckBox* mSelfIllum;
	QLabel* mShading;
	QCheckBox* mSoften;
	QCheckBox* mFaceMap;
	QCheckBox* mTwoSided;
	QCheckBox* mMapDecal;
	QCheckBox* mUseWire;
	QCheckBox* mUseWireAbs;
	QLabel* mWireSize;
};

class BoFaceView : public KListView
{
public:
	BoFaceView(QWidget* parent) : KListView(parent)
	{
		mUseLib3dsCoordinates = true;
		mShowPointIndices = false;

		QFontMetrics metrics(font());
		setShowToolTips(true);
		faceIndexColumn = addColumn(i18n("Face"));
		pointColumn[0] = addColumn(i18n("Point1"));
		pointColumn[1] = addColumn(i18n("Point2"));
		pointColumn[2] = addColumn(i18n("Point3"));
		texelColumn[0] = addColumn(i18n("Texel1"));
		texelColumn[1] = addColumn(i18n("Texel2"));
		texelColumn[2] = addColumn(i18n("Texel3"));

		// we try to keep the size as low as possible here - the list is
		// too wide anyway.
		// the titles won't be displayed, but the content should display
		// fine at least in most cases.
		materialColumn = addColumn(i18n("Material"), metrics.width(i18n("Material")));
		flagsColumn = addColumn(i18n("Flags"), metrics.width(QString::number(11)));
		smoothingColumn = addColumn(i18n("Smoothing"), metrics.width(QString::number(1111)));
		normalColumn = addColumn(i18n("Normal"), metrics.width(QString::number(11111)));
		bosonNormalColumn = addColumn(i18n("Boson Normal"), metrics.width(QString::number(11111)));

		setAllColumnsShowFocus(true);
		resize(100, height());
	}

	~BoFaceView()
	{
	}

	/**
	 * Applies to new faces only!
	 **/
	void setUseLib3dsCoordinates(bool c)
	{
		mUseLib3dsCoordinates = c;
	}
	void setShowPointIndices(bool p)
	{
		mShowPointIndices = p;
	}

	QListViewItem* addFace(int index, Lib3dsFace* face, Lib3dsMesh* mesh)
	{
		QListViewItem* item = new QListViewItem(this);
		QString no;
		if (mesh->faces >= 1000) {
			no.sprintf("%04d", index);
		} else if (mesh->faces >= 100) {
			no.sprintf("%03d", index);
		} else {
			no.sprintf("%02d", index);
		}
		item->setText(faceIndexColumn, no);

		setPoints(item, face, mesh);
		item->setText(materialColumn, face->material);
		QString flags = QString::number(face->flags);
		item->setText(flagsColumn, flags);
		item->setText(smoothingColumn, QString::number(face->smoothing));
		item->setText(normalColumn, QString("%1;%2;%3").arg(face->normal[0]).arg(face->normal[1]).arg(face->normal[2]));

		return item;
	}

protected:
	/**
	 * Set the points and the normal columns
	 **/
	void setPoints(QListViewItem* item, Lib3dsFace* face, Lib3dsMesh* mesh)
	{
		// note: calculating the inverse of the mesh matrix is slow but
		// we do it for every face. causes less work for the API - the
		// class is easier to use then.

		BoMatrix matrix;
		Lib3dsMatrix invMeshMatrix;
		lib3ds_matrix_copy(invMeshMatrix, mesh->matrix);
		lib3ds_matrix_inv(invMeshMatrix);
		matrix.loadMatrix(&invMeshMatrix[0][0]);

		BoVector3Float meshVertex[3];
		BoVector3Float v;
		for (int j = 0; j < 3; j++) {
			v.set(mesh->pointL[ face->points[j] ].pos);
			matrix.transform(&meshVertex[j], &v);
		}

		for (int j = 0; j < 3; j++) {
			if (!mShowPointIndices) {
				if (mUseLib3dsCoordinates) {
					v.set(mesh->pointL[ face->points[j] ].pos);
				} else {
					v = meshVertex[j];
				}
				item->setText(pointColumn[j], QString("%1;%2;%3").arg(v[0]).arg(v[1]).arg(v[2]));
			} else {
				QString index;
				index.sprintf("%03d", face->points[j]);
				item->setText(pointColumn[j], index);
			}

			QString texel;
			if (mesh->texels == 0) {
				texel = i18n("None");
			} else if (mesh->texels != mesh->points) {
				texel = i18n("Invalid Texel count");
			} else {
				// TODO: boson coordinates. we do some pretty
				// funny things to the original coordinates and
				// I know that they sometimes are not correct.
				// we should allow displaying them here..
				int p = face->points[j];
				Lib3dsTexel* t = mesh->texelL;
				texel = QString("%1;%2").arg(t[p][0]).arg(t[p][1]);
			}
			item->setText(texelColumn[j], texel);
		}

		BoVector3Float p, q;
		p = meshVertex[2] - meshVertex[1];
		q = meshVertex[0] - meshVertex[1];
		BoVector3Float normal = BoVector3Float::crossProduct(p, q);
		if (normal.length() != 0.0f) {
			normal.normalize();
		}
		item->setText(bosonNormalColumn, QString("%1;%2;%3").arg(normal[0]).arg(normal[1]).arg(normal[2]));
	}

private:
	bool mUseLib3dsCoordinates;
	bool mShowPointIndices;

	// column indices:
	int faceIndexColumn;
	int pointColumn[3];
	int texelColumn[3];
	int materialColumn;
	int flagsColumn;
	int smoothingColumn;
	int normalColumn;
	int bosonNormalColumn;
};

BoNodeObjectDataWidget::BoNodeObjectDataWidget(QWidget* parent) : QWidget(parent, "nodeobjectdatawidget")
{
 mLayout = new QVBoxLayout(this);

 mPivot = (QLabel*)addWidget(i18n("Pivot"), new QLabel(this));
 QToolTip::add(mPivot, i18n("The pivot point of the node"));

 mInstance = (QLabel*)addWidget(i18n("Instance"), new QLabel(this));
 QToolTip::add(mInstance, i18n("dunno what this is"));

 mBBoxMin = (QLabel*)addWidget(i18n("bbox_min"), new QLabel(this));
 QToolTip::add(mBBoxMin, i18n("Most probably this is the min point of the bounding box"));
 mBBoxMax = (QLabel*)addWidget(i18n("bbox_max"), new QLabel(this));
 QToolTip::add(mBBoxMax, i18n("Most probably this is the max point of the bounding box"));

 mPos = (QLabel*)addWidget(i18n("Position"), new QLabel(this));
 QToolTip::add(mPos, i18n("The position of the node in this frame. The matrix of the node has already been translated by this value."));
 mRot = (QLabel*)addWidget(i18n("Rotation (quat)"), new QLabel(this));
 QToolTip::add(mRot, i18n("The rotation of the node in this frame. The matrix of the node has already been rotated by this value. These 4 values (the quaternion) are the actually stored values."));
#if 0
 mRotAngle = (QLabel*)addWidget(i18n("Axis Rotation (x,y,z) -> degree)"), new QLabel(this));
 QToolTip::add(mRotAngle, i18n("The rotation in readable angles, calculated from the quaternion.\nFirst you see the axis (x,y,z) that is rotated around and then the angle."));
#endif
 mRotX = (QLabel*)addWidget(i18n("X Rotation"), new QLabel(this));
 mRotY = (QLabel*)addWidget(i18n("Y Rotation"), new QLabel(this));
 mRotZ = (QLabel*)addWidget(i18n("Z Rotation"), new QLabel(this));
 QToolTip::add(mRotX, i18n("The rotation in readable angles, calculated from the quaternion.\n"));
 QToolTip::add(mRotY, i18n("The rotation in readable angles, calculated from the quaternion.\n"));
 QToolTip::add(mRotZ, i18n("The rotation in readable angles, calculated from the quaternion.\n"));
 mScl = (QLabel*)addWidget(i18n("Scale"), new QLabel(this));
 QToolTip::add(mScl, i18n("The scale factor of the node in this frame. The matrix of the node has already been scaled by this value."));

 mMorphSmooth = (QLabel*)addWidget(i18n("morph_smooth"), new QLabel(this));
 mMorph = (QLabel*)addWidget(i18n("morph"), new QLabel(this));

 mHide = (QCheckBox*)addWidget(i18n("Hide"), new QCheckBox(this));
 mHide->setEnabled(false);

 mNodeTracks = new BoNodeTracksWidget(this);
 mLayout->addWidget(mNodeTracks);
 connect(mNodeTracks, SIGNAL(signalDisplayTrack(Bo3DSTrack*)), this, SIGNAL(signalDisplayTrack(Bo3DSTrack*)));
}

void BoNodeObjectDataWidget::setNodeObjectData(Lib3dsObjectData* d)
{
 QString pivot;
 QString instance;
 QString bboxMin;
 QString bboxMax;
 QString pos;
 QString rot;
#if 0
 QString rotAngle;
#endif
 QString rotX;
 QString rotY;
 QString rotZ;
 QString scl;
 QString morphSmooth;
 QString morph;
 bool hide = false;

 if (d) {
	const int prec = 3; // number of digits after the decimal point
	pivot = QString("(%1,%2,%3)").arg(d->pivot[0], 0, 'f', prec).arg(d->pivot[1], 0, 'f', prec).arg(d->pivot[2], 0, 'f', prec);
	instance = QString(d->instance);
	bboxMin = QString("(%1,%2,%3)").arg(d->bbox_min[0], 0, 'f', prec).arg(d->bbox_min[1], 0, 'f', prec).arg(d->bbox_min[2], 0, 'f', prec);
	bboxMax = QString("(%1,%2,%3)").arg(d->bbox_max[0], 0, 'f', prec).arg(d->bbox_max[1], 0, 'f', prec).arg(d->bbox_max[2], 0, 'f', prec);
	pos = QString("(%1,%2,%3)").arg(d->pos[0], 0, 'f', prec).arg(d->pos[1], 0, 'f', prec).arg(d->pos[2], 0, 'f', prec);
	rot = QString("((%1,%2,%3),%4))").arg(d->rot[0], 0, 'f', prec).arg(d->rot[1], 0, 'f', prec).arg(d->rot[2], 0, 'f', prec).arg(d->rot[3], 0, 'f', prec);

	float rX = 0.0f, rY = 0.0f, rZ = 0.0f;
#if 0
	float angle = 0.0f;
	quatToAxisRotation(d->rot, &rX, &rY, &rZ, &angle);
	rotAngle = QString("(%1,%2,%3) -> %4 degrees").arg(rX).arg(rY).arg(rZ).arg(angle);
#endif
	BoQuaternion q(d->rot[3], BoVector3Float(d->rot));
	q.matrix().toRotation(&rX, &rY, &rZ);
	rotX = QString::number(rX);
	rotY = QString::number(rY);
	rotZ = QString::number(rZ);

	scl = QString("(%1,%2,%3)").arg(d->scl[0], 0, 'f', prec).arg(d->scl[1], 0, 'f', prec).arg(d->scl[2], 0, 'f', prec);
	morphSmooth = QString::number(d->morph_smooth);
	morph = QString(d->morph);
	hide = d->hide;
 }

 mPivot->setText(pivot);
 mInstance->setText(instance);
 mBBoxMin->setText(bboxMin);
 mBBoxMax->setText(bboxMax);
 mPos->setText(pos);
 mRot->setText(rot);
#if 0
 mRotAngle->setText(rotAngle);
#endif
 mRotX->setText(rotX);
 mRotY->setText(rotY);
 mRotZ->setText(rotZ);
 mScl->setText(scl);
 mMorphSmooth->setText(morphSmooth);
 mMorph->setText(morph);
 mHide->setChecked(hide);

 mNodeTracks->setNodeObjectData(d);
}

QWidget* BoNodeObjectDataWidget::addWidget(const QString& label, QWidget* w)
{
 QWidget* box = new QWidget(this, "widgetbox");
 w->reparent(box, QPoint(0,0)); // ugly, but useful
 QHBoxLayout* l = new QHBoxLayout(box);
 l->addWidget(new QLabel(label, box, "label"));
 l->addWidget(w);
 mLayout->addWidget(box);

 return w;
}




BoListView::BoListView(QWidget* parent) : KListView(parent)
{
 mPopup = 0;
 setAllColumnsShowFocus(true);
}

BoListView::~BoListView()
{
}

void BoListView::allowHide(int column)
{
 if (!mPopup) {
	header()->setClickEnabled(true);
	header()->installEventFilter(this);
	mPopup = new KPopupMenu(this);
	mPopup->insertTitle(i18n("View columns"));
	mPopup->setCheckable(true);

	connect(mPopup, SIGNAL(activated(int)), this, SLOT(slotToggleHideColumn(int)));
 }
 if (column < 0) {
	for (int i = 0; i < columns(); i++) {
		allowHide(i);
	}
 } else {
	mPopup->insertItem(columnText(column), column);
	mPopup->setItemChecked(column, true);

	boDebug() << k_funcinfo << columnText(column) << "==" << column << endl;
 }
}

void BoListView::slotToggleHideColumn(int id)
{
 boDebug() << k_funcinfo << id << endl;
 if (!mPopup) {
	boWarning() << k_funcinfo << "NULL popup menu" << endl;
	return;
 }
 if (mPopup->indexOf(id) == -1) {
	boError() << k_funcinfo << "Invalid id " << id << endl;
	return;
 }
 bool hide = mPopup->isItemChecked(id);
 mPopup->setItemChecked(id, !hide);
 if (hide) {
	removeColumn(id);
 } else {
	addColumn("test1");
 }
}

bool BoListView::eventFilter(QObject* o, QEvent* e)
{
 // shamelessy stolen from KMail :)
 if (mPopup && (e->type() == QEvent::MouseButtonPress &&
		static_cast<QMouseEvent*>(e)->button() == RightButton &&
		o->isA("QHeader"))) {
	mPopup->popup( static_cast<QMouseEvent*>(e)->globalPos() );
	return true;
 }
 return KListView::eventFilter(o, e);
}

class KGameModelDebug::KGameModelDebugPrivate
{
public:
	KGameModelDebugPrivate()
	{
		mTopLayout = 0;
		mModelBox = 0;

		mTabWidget = 0;
		mMaterialPage = 0;
		mMeshPage = 0;
		mNodePage = 0;

		mMaterialBox = 0;
		mMaterialData = 0;

		mMeshView = 0;
		mFaceList = 0;
		mConnectableWidget = 0;
		mConnectedFacesList = 0;
		mUnconnectedFacesList = 0;
		mUseLib3dsCoordinates = 0;
		mShowPointIndices = 0;
		mHideConnectableWidgets = 0;
		mMeshMatrix = 0;
		mInvMeshMatrix = 0;

		mNodeView = 0;
		mCurrentFrame = 0;
		mNodeMatrix = 0;
		mNodeObjectData = 0;
		mNodeTracks = 0;
		mTrackView = 0;

		mMeshFacesCountLabel = 0;
		mMeshVertexCountLabel = 0;
		mNodeFacesCountLabel = 0;
		mNodeVertexCountLabel = 0;

		m3ds = 0;
	}

	QVBoxLayout* mTopLayout;
	QComboBox* mModelBox;
	QMap<int, QString> mModelFiles;

	QTabWidget* mTabWidget;
	QWidget* mMaterialPage;
	QWidget* mMeshPage;
	QWidget* mNodePage;

	KListBox* mMaterialBox;
	BoMaterialDataWidget* mMaterialData;
	QPtrDict<Lib3dsMaterial> mListItem2Material;
	KListView* mTextureView;

	KListView* mMeshView;
	QPtrDict<Lib3dsMesh> mListItem2Mesh;
	QPtrDict<Lib3dsFace> mListItem2Face;
	BoFaceView* mFaceList;
	KListView* mPointList;
	QVBox* mConnectableWidget;
	BoFaceView* mConnectedFacesList;
	BoFaceView* mUnconnectedFacesList;
	QCheckBox* mUseLib3dsCoordinates;
	QCheckBox* mShowPointIndices;
	QCheckBox* mHideConnectableWidgets;
	BoMatrixWidget* mMeshMatrix;
	BoMatrixWidget* mInvMeshMatrix;

	KListView* mNodeView;
	QPtrDict<Lib3dsNode> mListItem2Node;
	KIntNumInput* mCurrentFrame;
	BoMatrixWidget* mNodeMatrix;
	BoNodeObjectDataWidget* mNodeObjectData;
	BoNodeTracksWidget* mNodeTracks;
	BoTrackWidget* mTrackView;

	QLabel* mMeshFacesCountLabel;
	QLabel* mMeshVertexCountLabel;
	QLabel* mNodeFacesCountLabel;
	QLabel* mNodeVertexCountLabel;

	int mCurrentItem;
	Lib3dsFile* m3ds;
};

KGameModelDebug::KGameModelDebug(QWidget* parent) : QWidget(parent, "KGameModelDebug")
{
 d = new KGameModelDebugPrivate;
 init();
}

KGameModelDebug::~KGameModelDebug()
{
 if (d->m3ds) {
	lib3ds_file_free(d->m3ds);
 }
 delete d;
}

void KGameModelDebug::init()
{
 d->mCurrentItem = -1;
 d->mTopLayout = new QVBoxLayout(this);
 QHBoxLayout* modelLayout = new QHBoxLayout(d->mTopLayout);
 QLabel* modelLabel = new QLabel(i18n("Model: "), this);
 d->mModelBox = new QComboBox(this);
 connect(d->mModelBox, SIGNAL(activated(int)), this, SLOT(slotModelChanged(int)));
 modelLayout->addWidget(modelLabel);
 modelLayout->addWidget(d->mModelBox);

 d->mTabWidget = new QTabWidget(this);
 d->mTopLayout->addWidget(d->mTabWidget);

 initMeshPage();
 initMaterialPage();
 initNodePage();
 setMatrixPrecision(3);

 slotUpdate();
}

void KGameModelDebug::initMaterialPage()
{
 d->mMaterialPage = new QWidget(d->mTabWidget);
 QHBoxLayout* l = new QHBoxLayout(d->mMaterialPage, 10, 10);
 QSplitter* splitter = new QSplitter(d->mMaterialPage);
 l->addWidget(splitter, 0);

 d->mMaterialBox = new KListBox(splitter);
 connect(d->mMaterialBox, SIGNAL(executed(QListBoxItem*)), this, SLOT(slotDisplayMaterial(QListBoxItem*)));
 QFontMetrics metrics(font());

 d->mMaterialData = new BoMaterialDataWidget(splitter);

 d->mTextureView = new KListView(splitter);
 d->mTextureView->setAllColumnsShowFocus(true);
 d->mTextureView->addColumn(i18n("Map"));
 d->mTextureView->addColumn(i18n("Name"));
 d->mTextureView->addColumn(i18n("Flags"), metrics.width(QString::number(111)));
 d->mTextureView->addColumn(i18n("Percent"), metrics.width(QString::number(11)));
 d->mTextureView->addColumn(i18n("Blur"), metrics.width(QString::number(11)));
 d->mTextureView->addColumn(i18n("Scale"), metrics.width(QString::number(1111111)));
 d->mTextureView->addColumn(i18n("Offset"), metrics.width(QString::number(1111111)));
 d->mTextureView->addColumn(i18n("Rotation"), metrics.width(QString::number(111)));
 d->mTextureView->addColumn(i18n("Tint1"), metrics.width(QString::number(11)));
 d->mTextureView->addColumn(i18n("Tint2"), metrics.width(QString::number(11)));
 d->mTextureView->addColumn(i18n("Tint_R"), metrics.width(QString::number(11)));
 d->mTextureView->addColumn(i18n("Tint_G"), metrics.width(QString::number(11)));
 d->mTextureView->addColumn(i18n("Tint_B"), metrics.width(QString::number(11)));
 d->mTextureView->setMinimumWidth(100);

 d->mTabWidget->addTab(d->mMaterialPage, i18n("M&aterials"));
}

void KGameModelDebug::initMeshPage()
{
 // AB: a "mesh" from a lib3ds point of view is the object itself. it does not
 // contain any information on it's position - it is always at (0,0,0).
 // a "node" is an instance of a mesh
 d->mMeshPage = new QWidget(d->mTabWidget);
 QHBoxLayout* l = new QHBoxLayout(d->mMeshPage, 10, 10);
 QSplitter* splitter = new QSplitter(d->mMeshPage);
 l->addWidget(splitter);
 QFontMetrics metrics(font());

 QVBox* meshView = new QVBox(splitter);
 d->mMeshView = new KListView(meshView);
 d->mMeshView->setAllColumnsShowFocus(true);
 d->mMeshView->addColumn(i18n("Name"), metrics.width(i18n("Name")));
 d->mMeshView->addColumn(i18n("Color"), metrics.width(QString::number(111)));
 d->mMeshView->addColumn(i18n("Points count"), metrics.width(QString::number(111)));
 d->mMeshView->addColumn(i18n("Texels count"), metrics.width(QString::number(111)));
 d->mMeshView->addColumn(i18n("Faces count"), metrics.width(QString::number(111)));
 d->mMeshView->addColumn(i18n("Flags count"), metrics.width(QString::number(11)));
 d->mMeshView->addColumn(i18n("Max point index"), metrics.width(QString::number(11)));
 d->mMeshView->addColumn(i18n("# of instances"), metrics.width(QString::number(11)));
 connect(d->mMeshView, SIGNAL(executed(QListViewItem*)), this, SLOT(slotDisplayMesh(QListViewItem*)));
 QVBox* modelInfo = new QVBox(meshView); // actually it doesn't fit to "meshView", but rather display info about the model
 QHBox* faces = new QHBox(modelInfo);
 (void)new QLabel(i18n("Total Mesh Faces: "), faces);
 d->mMeshFacesCountLabel = new QLabel(faces);
 QToolTip::add(d->mMeshFacesCountLabel, i18n("This is the total number of faces in (different) meshes. Note that every mesh can appear several times in a model, so this number is <em>not</em> the total number of faces in the model!"));
 QHBox* vertices = new QHBox(modelInfo);
 (void)new QLabel(i18n("Total Mesh Vertices (Faces * 3): "), vertices);
 d->mMeshVertexCountLabel = new QLabel(vertices);

 QVBox* faceView = new QVBox(splitter);
 d->mFaceList = new BoFaceView(faceView);
 connect(d->mFaceList, SIGNAL(executed(QListViewItem*)), this, SLOT(slotConnectToFace(QListViewItem*)));
 d->mConnectableWidget = new QVBox(faceView);
 (void)new QLabel(i18n("Connectable to selected face:"), d->mConnectableWidget);
 d->mConnectedFacesList = new BoFaceView(d->mConnectableWidget);
 (void)new QLabel(i18n("Unconnectable to selected face:"), d->mConnectableWidget);
 d->mUnconnectedFacesList = new BoFaceView(d->mConnectableWidget);
 d->mUseLib3dsCoordinates = new QCheckBox(i18n("Display lib3ds coordinates of points"), faceView);
 d->mUseLib3dsCoordinates->setChecked(true);
 connect(d->mUseLib3dsCoordinates, SIGNAL(toggled(bool)), this, SLOT(slotUseLib3dsCoordinates(bool)));
 QToolTip::add(d->mUseLib3dsCoordinates, i18n("Display the coordinates of the points of a face as they appear in a .3ds file. If unchecked display them as they get rendered (i.e. in mesh coordinates)."));
 d->mShowPointIndices = new QCheckBox(i18n("Show point indices instead of coordinates"), faceView);
 d->mShowPointIndices->setChecked(false);
 connect(d->mShowPointIndices, SIGNAL(toggled(bool)), this, SLOT(slotShowPointIndices(bool)));
 QToolTip::add(d->mShowPointIndices, i18n("Display the index of the point in the vertex pool (of the mesh), not the coordinates"));
 d->mHideConnectableWidgets = new QCheckBox(i18n("Hide \"connectable\" widgets"), faceView);
 d->mHideConnectableWidgets->setChecked(true);
 connect(d->mHideConnectableWidgets, SIGNAL(toggled(bool)), this, SLOT(slotHideConnectableWidgets(bool)));
 slotHideConnectableWidgets(d->mHideConnectableWidgets->isChecked());
 QToolTip::add(d->mHideConnectableWidgets, i18n("The \"connectable\" widgets show which faces of a mesh are connectable to a certain face. This is important to make GL_TRIANGLE_STRIPs, but the code used here is obsolete."));
#if !ALLOW_FACE_CONNECTIONS
 d->mHideConnectableWidgets->hide();
#endif

 QVBox* pointView = new QVBox(splitter);
 d->mPointList = new KListView(pointView);
 d->mPointList->setAllColumnsShowFocus(true);
 d->mPointList->addColumn(i18n("Point"));
 d->mPointList->addColumn(i18n("x"));
 d->mPointList->addColumn(i18n("y"));
 d->mPointList->addColumn(i18n("z"));
 d->mPointList->addColumn(i18n("Texel x"));
 d->mPointList->addColumn(i18n("Texel y"));

 QVBox* matrixBox = new QVBox(splitter);
 QVGroupBox* meshMatrixBox = new QVGroupBox(i18n("Mesh Matrix"), matrixBox);
 d->mMeshMatrix = new BoMatrixWidget(meshMatrixBox);
 QToolTip::add(d->mMeshMatrix, i18n("This is the mesh matrix (i.e. mesh->matrix in a lib3ds mesh)."));
 QVGroupBox* invMeshMatrixBox = new QVGroupBox(i18n("Inv Mesh Matrix"), matrixBox);
 d->mInvMeshMatrix = new BoMatrixWidget(invMeshMatrixBox);
 QToolTip::add(d->mInvMeshMatrix, i18n("This is the inverse of the mesh matrix."));

 d->mTabWidget->addTab(d->mMeshPage, i18n("&Meshes"));
}

void KGameModelDebug::initNodePage()
{
 // a node is an "instance" of a mesh from a lib3ds point of view. the mesh is
 // the "class" and the node is the "object".
 d->mNodePage = new QWidget(d->mTabWidget);
 QVBoxLayout* l = new QVBoxLayout(d->mNodePage, 10, 10);
 QSplitter* splitter = new QSplitter(d->mNodePage);
 l->addWidget(splitter, 1);
 QFontMetrics metrics(font());

 QVBox* nodeView = new QVBox(splitter);
 d->mNodeView = new KListView(nodeView);
 d->mNodeView->setAllColumnsShowFocus(true);
 d->mNodeView->setRootIsDecorated(true);
 d->mNodeView->addColumn(i18n("Name"));
 d->mNodeView->addColumn(i18n("ID"));
 d->mNodeView->addColumn(i18n("flags1"));
 d->mNodeView->addColumn(i18n("flags2"));
 connect(d->mNodeView, SIGNAL(executed(QListViewItem*)),
		this, SLOT(slotDisplayNode(QListViewItem*)));
 QVBox* nodesInfo = new QVBox(nodeView);
 QHBox* faces = new QHBox(nodesInfo);
 (void)new QLabel(i18n("Node Faces: "), faces);
 d->mNodeFacesCountLabel = new QLabel(faces);
 QHBox* vertices = new QHBox(nodesInfo);
 (void)new QLabel(i18n("Node Vertices (Faces * 3): "), vertices);
 QToolTip::add(d->mNodeFacesCountLabel, i18n("This is the total number of faces in <em>all</em> nodes and therefore the total number of rendered faces."));
 d->mNodeVertexCountLabel = new QLabel(vertices);
 QToolTip::add(d->mNodeVertexCountLabel, i18n("The actual number of vertices in the nodes. This is the same as the faces number above, multiplied by 3 (a face/triangle has always 3 points)."));

 d->mNodeObjectData = new BoNodeObjectDataWidget(splitter);


 // display all data from Lib3dsObjectData !
 // note that this data depends on the current frame!
 // data includes: pivot, bbox, pos, rot (rotation?), scl (scale?), morph_smooth
 // (?), morph (?), hide (?)
 //
 // and node->matrix

 QVBox* box = new QVBox(splitter);
 QVGroupBox* nodeMatrixBox = new QVGroupBox(i18n("Matrix"), box);
 d->mNodeMatrix = new BoMatrixWidget(nodeMatrixBox);
 QVGroupBox* trackBox = new QVGroupBox(i18n("Track"), box);
 d->mTrackView = new BoTrackWidget(trackBox);
 connect(d->mNodeObjectData, SIGNAL(signalDisplayTrack(Bo3DSTrack*)),
		d->mTrackView, SLOT(slotDisplayTrack(Bo3DSTrack*)));


 d->mCurrentFrame = new KIntNumInput(d->mNodePage);
 d->mCurrentFrame->setLabel(i18n("Frame"));
 connect(d->mCurrentFrame, SIGNAL(valueChanged(int)),
		this, SLOT(slotFrameChanged(int)));
 l->addWidget(d->mCurrentFrame);

 d->mTabWidget->addTab(d->mNodePage, i18n("&Nodes"));
}

void KGameModelDebug::setMatrixPrecision(int prec)
{
 d->mMeshMatrix->setPrecision(prec);
 d->mInvMeshMatrix->setPrecision(prec);
 d->mNodeMatrix->setPrecision(prec);
}

void KGameModelDebug::addFile(const QString& file, const QString& _name)
{
 unsigned int i = d->mModelBox->count();
 QString name = _name.isEmpty() ? QString::number(i) : _name;
 d->mModelFiles.insert(i, file);
 d->mModelBox->insertItem(name);
}

void KGameModelDebug::addFiles(const QString& _dir)
{
 QStringList allDirs;
 QStringList allFiles;
 allDirs.append(_dir);
 for (unsigned int i = 0; i < allDirs.count(); i++) {
	QDir dir(allDirs[i]);
	QStringList dirs = dir.entryList(QDir::Dirs | QDir::Readable | QDir::Executable);
	dirs.remove(".");
	dirs.remove("..");
	for (unsigned int j = 0; j < dirs.count(); j++) {
		if (allDirs.contains(dirs[j]) == 0) {
			allDirs.append(dir.absFilePath(dirs[j]));
		}
	}

	QStringList files = dir.entryList(QString::fromLatin1("*.3ds"), QDir::Files | QDir::Readable);
	for (unsigned int j = 0; j < files.count(); j++) {
		if (allFiles.contains(files[j]) == 0) {
			allFiles.append(dir.absFilePath(files[j]));
		}
	}
 }

 // throw away any files that we know already
 QMap<int, QString>::Iterator it;
 for (it = d->mModelFiles.begin(); it != d->mModelFiles.end(); ++it) {
	if (allFiles.contains(it.data()) != 0) {
		allFiles.remove(it.data());
	}
 }

 // add remaining files
 for (unsigned int i = 0; i < allFiles.count(); i++) {
	addFile(allFiles[i], allFiles[i]);
 }
}

void KGameModelDebug::slotModelChanged(int index)
{
 if (index < 0) {
	boWarning() << k_funcinfo << "index==" << index << endl;
	return;
 } else if (index >= d->mModelBox->count()) {
	boError() << k_funcinfo << "index out of range: " << index << endl;
	return;
 }
 if (d->m3ds) {
	if (d->mCurrentItem == index) {
		return;
	}
 }
 slotUpdate();
}

void KGameModelDebug::slotUpdate()
{
 if (d->m3ds) {
	lib3ds_file_free(d->m3ds);
	d->m3ds = 0;
 }
 d->mCurrentItem = d->mModelBox->currentItem();
 d->m3ds = lib3ds_file_load(d->mModelFiles[d->mCurrentItem]);

 updateMaterialPage();
 updateMeshPage();
 updateNodePage();
}

void KGameModelDebug::updateMaterialPage()
{
 d->mMaterialBox->clear();
 d->mMaterialData->setMaterial(0);
 d->mTextureView->clear();
 d->mListItem2Material.clear();
 if (!d->m3ds) {
	return;
 }
 Lib3dsMaterial* mat = d->m3ds->materials;
 for (; mat; mat = mat->next) {
	QString m = mat->name;
	QListBoxText* item = new QListBoxText(d->mMaterialBox, m);
	d->mListItem2Material.insert(item, mat);
 }
}

void KGameModelDebug::updateMeshPage()
{
 d->mMeshView->clear();
 d->mFaceList->clear();
 d->mPointList->clear();
 d->mListItem2Mesh.clear();
 d->mListItem2Face.clear();
 d->mMeshMatrix->setIdentity();
 d->mInvMeshMatrix->setIdentity();

 if (!d->m3ds) {
	return;
 }
 slotConstructMeshList();

 int faces = 0;
 Lib3dsMesh* mesh = d->m3ds->meshes;
 for (; mesh; mesh = mesh->next) {
	faces += mesh->faces;
 }
 d->mMeshFacesCountLabel->setText(QString::number(faces));
 d->mMeshVertexCountLabel->setText(QString::number(faces * 3));
}

void KGameModelDebug::updateNodePage()
{
 d->mNodeView->clear();
 d->mListItem2Node.clear();
 d->mNodeMatrix->setIdentity();
 d->mNodeObjectData->setNodeObjectData(0);

 d->mCurrentFrame->setValue(0);

 if (!d->m3ds) {
	return;
 }
 slotConstructNodeList();
 d->mNodeView->setCurrentItem(d->mNodeView->firstChild());
 d->mCurrentFrame->setRange(0, d->m3ds->frames - 1);
 slotFrameChanged(0);

 int faces = 0;
 QValueList<Lib3dsNode*> allNodes;
 Lib3dsNode* node = d->m3ds->nodes;
 for (; node; node = node->next) {
	allNodes.append(node);
 }
 QValueList<Lib3dsNode*>::Iterator it;
 for (it = allNodes.begin(); it != allNodes.end(); ++it) {
	Lib3dsNode* node = *it;
	if (node->type != LIB3DS_OBJECT_NODE) {
		continue;
	}
	// AB: $$$DUMMY nodes can contain child nodes as well.
	// for debugging purposes we display them as well
	Lib3dsNode* p;
	for (p = node->childs; p; p = p->next) {
		allNodes.append(p);
	}
	Lib3dsMesh* mesh = lib3ds_file_mesh_by_name(d->m3ds, node->name);
	if (mesh) {
		faces += mesh->faces;
	}
 }
 d->mNodeFacesCountLabel->setText(QString::number(faces));
 d->mNodeVertexCountLabel->setText(QString::number(faces * 3));
}

void KGameModelDebug::slotConstructMeshList()
{
 boDebug() << k_funcinfo << endl;
 d->mMeshView->clear();
 d->mListItem2Mesh.clear();
 if (!d->m3ds) {
	return;
 }
 Lib3dsMesh* mesh = d->m3ds->meshes;
 for (; mesh; mesh = mesh->next) {
	QString name(mesh->name);
	QListViewItem* item = new QListViewItem(d->mMeshView);
	item->setText(0, name);
	item->setText(1, QString::number(mesh->color));
	item->setText(2, QString::number(mesh->points));
	item->setText(3, QString::number(mesh->texels));
	item->setText(4, QString::number(mesh->faces));
	item->setText(5, QString::number(mesh->flags));
	int indices = 0;
	for (unsigned int i = 0; i < mesh->faces; i++) {
		Lib3dsFace* f = &mesh->faceL[i];
		indices = QMAX(indices, f->points[0]);
		indices = QMAX(indices, f->points[1]);
		indices = QMAX(indices, f->points[2]);
	}
	item->setText(6, QString::number(indices));
	unsigned int instances = 0;
	QPtrQueue<Lib3dsNode> allNodes;
	for (Lib3dsNode* n = d->m3ds->nodes; n; n = n->next) {
		allNodes.enqueue(n);
	}
	while (!allNodes.isEmpty()) {
		Lib3dsNode* currentNode = allNodes.dequeue();
		for (Lib3dsNode* n = currentNode->childs; n; n = n->next) {
			allNodes.enqueue(n);
		}
		if ((currentNode->type == LIB3DS_OBJECT_NODE) && (strcmp(currentNode->name, mesh->name)) == 0) {
			instances++;
		}
	}
	item->setText(7, QString::number(instances));
	d->mListItem2Mesh.insert(item, mesh);
 }
}

void KGameModelDebug::slotConstructNodeList()
{
 boDebug() << k_funcinfo << endl;
 d->mNodeView->clear();
 d->mListItem2Node.clear();
 if (!d->m3ds) {
	return;
 }
 Lib3dsNode* node = d->m3ds->nodes;
 for (; node; node = node->next) {
	addNodeToList(0, node);
 }

}

void KGameModelDebug::addNodeToList(QListViewItem* parent, Lib3dsNode* node)
{
 BO_CHECK_NULL_RET(node);
 if (node->type != LIB3DS_OBJECT_NODE) {
	return;
 }
 QListViewItem* item = 0;
 if (parent) {
	item = new QListViewItem(parent);
 } else {
	item = new QListViewItem(d->mNodeView);
 }
 item->setText(0, node->name);
 item->setText(1, QString::number(node->node_id));
 item->setText(2, QString::number(node->flags1));
 item->setText(3, QString::number(node->flags2));
 d->mListItem2Node.insert(item, node);
 item->setOpen(true);

 {
	Lib3dsNode* p;
	for (p = node->childs; p; p = p->next) {
		addNodeToList(item, p);
	}
 }
}

void KGameModelDebug::slotUseLib3dsCoordinates(bool)
{
 slotDisplayMesh(d->mMeshView->currentItem());
}

void KGameModelDebug::slotShowPointIndices(bool)
{
 slotDisplayMesh(d->mMeshView->currentItem());
}

void KGameModelDebug::slotDisplayMesh(QListViewItem* item)
{
 d->mFaceList->clear();
 d->mPointList->clear();
 d->mListItem2Face.clear();

 if (!item) {
	boDebug() << k_funcinfo << "NULL item" << endl;
	return;
 }

 Lib3dsMesh* mesh = d->mListItem2Mesh[item];
 if (!mesh) {
	boWarning() << k_funcinfo << "NULL mesh" << endl;
	return;
 }

 // faces
 d->mFaceList->setUseLib3dsCoordinates(d->mUseLib3dsCoordinates->isChecked());
 d->mFaceList->setShowPointIndices(d->mShowPointIndices->isChecked());
 for (unsigned int i = 0; i < mesh->faces; i++) {
	Lib3dsFace* face = &mesh->faceL[i];
	QListViewItem* item = d->mFaceList->addFace(i, face, mesh);
	d->mListItem2Face.insert(item, face);
 }

 // points
 for (unsigned int i = 0; i < mesh->points; i++) {
	QString no;
	if (mesh->points >= 1000) {
		no.sprintf("%04d", i);
	} else if (mesh->points >= 100) {
		no.sprintf("%03d", i);
	} else {
		no.sprintf("%02d", i);
	}
	QListViewItem* item = new QListViewItem(d->mPointList);
	item->setText(0, no);
	item->setText(1, QString::number(mesh->pointL[i].pos[0]));
	item->setText(2, QString::number(mesh->pointL[i].pos[1]));
	item->setText(3, QString::number(mesh->pointL[i].pos[2]));
	if (mesh->texels == mesh->points) {
		item->setText(4, QString::number(mesh->texelL[i][0]));
		item->setText(5, QString::number(mesh->texelL[i][1]));
	}
 }

 // mesh->matrix
 d->mMeshMatrix->setMatrix(mesh->matrix);
 Lib3dsMatrix invMeshMatrix;
 lib3ds_matrix_copy(invMeshMatrix, mesh->matrix);
 lib3ds_matrix_inv(invMeshMatrix);
 d->mInvMeshMatrix->setMatrix(invMeshMatrix);

 // TODO: mesh->box_map
 // TODO: mesh->map_data
}

void KGameModelDebug::slotDisplayMaterial(QListBoxItem* item)
{
 d->mTextureView->clear();
 Lib3dsMaterial* mat = d->mListItem2Material[item];
 d->mMaterialData->setMaterial(mat);
 if (!mat) {
	boWarning() << k_funcinfo << "NULL material" << endl;
	return;
 }

 // note: it is very easy to display all values of the texture maps, but it's
 // very hard to implement them. afaik 3ds is a propietary format (baaad, btw)
 // and lib3ds is hardly (ahem - not at all!) documented.
 // e.g. i use currently texture1_map in boson only. and i do not even know what
 // the *_mask maps are!
 addTextureMap(i18n("Texture1 Map"), &mat->texture1_map);
 addTextureMap(i18n("Texture1 Mask"), &mat->texture1_mask);
 addTextureMap(i18n("Texture2 Map"), &mat->texture2_map);
 addTextureMap(i18n("Texture2 Mask"), &mat->texture2_mask);
 addTextureMap(i18n("Opacity Map"), &mat->opacity_map);
 addTextureMap(i18n("Opacity Mask"), &mat->opacity_mask);
 addTextureMap(i18n("Bump Map"), &mat->bump_map);
 addTextureMap(i18n("Bump Mask"), &mat->bump_mask);
 addTextureMap(i18n("Specular Map"), &mat->specular_map);
 addTextureMap(i18n("Specular Mask"), &mat->specular_mask);
 addTextureMap(i18n("Shininess Map"), &mat->shininess_map);
 addTextureMap(i18n("Shininess Mask"), &mat->shininess_mask);
 addTextureMap(i18n("Self Illum Map"), &mat->self_illum_map);
 addTextureMap(i18n("Self Illum Mask"), &mat->self_illum_mask);
 addTextureMap(i18n("Reflection Map"), &mat->reflection_map);
 addTextureMap(i18n("Reflection Mask"), &mat->reflection_mask);
}

void KGameModelDebug::slotDisplayNode(QListViewItem* item)
{
 if (!item) {
	boDebug() << k_funcinfo << "NULL item" << endl;
	return;
 }
 item->setSelected(true);
 Lib3dsNode* node = d->mListItem2Node[item];

 if (!node) {
	boWarning() << k_funcinfo << "NULL node" << endl;
	return;
 }
 boDebug() << k_funcinfo << node->name << endl;

 d->mNodeMatrix->setMatrix(node->matrix);
 d->mNodeObjectData->setNodeObjectData(&node->data.object);
}

void KGameModelDebug::slotFrameChanged(int frame)
{
 if (!d->m3ds) {
	return;
 }
 boDebug() << k_funcinfo << frame << endl;
 if (frame >= d->m3ds->frames) {
	boWarning() << k_funcinfo << "invalid frame " << frame << endl;
	return;
 }
 lib3ds_file_eval(d->m3ds, frame);
 d->m3ds->current_frame = frame;

 slotDisplayNode(d->mNodeView->currentItem());
}

QString KGameModelDebug::rgbaText(Lib3dsRgba r)
{
 return i18n("%1,%2,%3,%4").arg(r[0]).arg(r[1]).arg(r[2]).arg(r[3]);
}

QString KGameModelDebug::rgbText(Lib3dsRgb r)
{
 return i18n("%1,%2,%3").arg(r[0]).arg(r[1]).arg(r[2]);
}

void KGameModelDebug::addTextureMap(const QString& name, _Lib3dsTextureMap* t)
{
 QListViewItem* item = new QListViewItem(d->mTextureView);
 item->setText(0, name);
 item->setText(1, t->name);
 QString flags = QString::number(t->flags); // TODO: display the actual flags, too - see _Lib3dsTextureMapFlags in material.h
 item->setText(2, flags);
 item->setText(3, QString::number(t->percent));
 item->setText(4, QString::number(t->blur));
 item->setText(5, i18n("%1,%2").arg(t->scale[0]).arg(t->scale[1]));
 item->setText(6, i18n("%1,%2").arg(t->offset[0]).arg(t->offset[1]));
 item->setText(7, QString::number(t->rotation));
 item->setText(8, rgbText(t->tint_1));
 item->setText(9, rgbText(t->tint_2));
 item->setText(10, rgbText(t->tint_r));
 item->setText(11, rgbText(t->tint_g));
 item->setText(12, rgbText(t->tint_b));
}

void KGameModelDebug::slotConnectToFace(QListViewItem* item)
{
#if ALLOW_FACE_CONNECTIONS
 Lib3dsFace* face = d->mListItem2Face[item];
 d->mConnectedFacesList->clear();
 d->mUnconnectedFacesList->clear();
 if (!face) {
	boWarning() << k_funcinfo << "NULL face" << endl;
	return;
 }
 boDebug() << k_funcinfo << endl;
 QPtrList<Lib3dsFace> connected;
 Lib3dsMesh* mesh = d->mListItem2Mesh[d->mMeshView->selectedItem()];
 if (!mesh) {
	boError() << k_funcinfo << "NULL mesh" << endl;
	return;
 }
 Bo3DSLoad::findAdjacentFaces(&connected, mesh, face);
 QPtrList<Lib3dsFace> faces;
 d->mConnectedFacesList->setUseLib3dsCoordinates(d->mUseLib3dsCoordinates->isChecked());
 d->mUnconnectedFacesList->setUseLib3dsCoordinates(d->mUseLib3dsCoordinates->isChecked());
 d->mConnectedFacesList->setShowPointIndices(d->mShowPointIndices->isChecked());
 d->mUnconnectedFacesList->setShowPointIndices(d->mShowPointIndices->isChecked());
 for (unsigned int i = 0; i < mesh->faces; i++) {
	Lib3dsFace* f = &mesh->faceL[i];
	if (connected.contains(f)) {
		d->mConnectedFacesList->addFace(i, f, mesh);
	} else {
		d->mUnconnectedFacesList->addFace(i, f, mesh);
	}
 }
#endif
}

void KGameModelDebug::slotHideConnectableWidgets(bool h)
{
 if (h) {
	d->mConnectableWidget->hide();
 } else {
	d->mConnectableWidget->show();
 }
}
