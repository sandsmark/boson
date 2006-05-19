/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef KGAME3DSMODELDEBUG_H
#define KGAME3DSMODELDEBUG_H

#include <qwidget.h>

#include <klistview.h>

#include <lib3ds/types.h>

class QListBoxItem;
class QListViewItem;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QCheckBox;
class KPopupMenu;
class KListBox;

struct _Lib3dsTextureMap;
typedef struct _Lib3dsObjectData Lib3dsObjectData;

class BosonModel;
class SpeciesTheme;
class Bo3DSTrack;
class Bo3DSTrackTCB;
class Bo3DSTrackKey;
class Bo3DSTrackQuat;
class Bo3DSTrackLin3;
class Bo3DSTrackBool;
class Bo3DSTrackMorph;

class BoListView : public KListView
{
	Q_OBJECT
public:
	BoListView(QWidget* parent);

	~BoListView();

	/**
	 * Insert the @ref QListView::columnText of @p column to the @ref
	 * KPopupMenu that appears when the user right clicks on the header.
	 *
	 * The user will be able to hide the column then by clicking that menu
	 * entry.
	 *
	 * This has no effect if it was called without any columns present. You
	 * must call @ref addColmn first.
	 **/
	void allowHide(int column);

protected slots:
	void slotToggleHideColumn(int index);

protected:
	virtual bool eventFilter(QObject* o, QEvent* e);
	
private:
	KPopupMenu* mPopup;
};


/**
 * This widget is most useful in borender, but could also be used in the main
 * boson debug dialog.
 *
 * Here the user can select a model from a list (see @ref addFile).
 * For this model pretty much all available data are displayed.
 * Currently we only re-parse the .3ds file. In the future wi might also display
 * additional data from @ref BosonModel.
 *
 * Note that this widget displays <em>a lot</em> of stuff. Most values are not
 * used in boson, as it is very easy to read them, but hard to implement. For
 * example there are 16 different texture maps for every material (8 maps and 8
 * masks). We use (and our models do, too) use texture1_map only. And I do not
 * even know what the *_mask texture maps might be.
 * @short Widget that displays most data of a model (like faces and materials)
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGame3DSModelDebug : public QWidget
{
	Q_OBJECT
public:
	KGame3DSModelDebug(QWidget* parent);
	~KGame3DSModelDebug();

	/**
	 * Note that we'll reparse the .3ds file! Most data are discarded in
	 * BosonModel, so we need to do so.
	 * @param name The text for the combo box. QString::null for the index.
	 **/
	void addFile(const QString& file, const QString& name = QString::null);

	/**
	 * Add all .3ds files found in @p dir (recursively).
	 *
	 * Files that were added previously using @ref addFile are discarded,
	 * you can use this function to add any files that might have been
	 * missed.
	 **/
	void addFiles(const QString& dir);

	void setMatrixPrecision(int prec = 6);

	/**
	 * @return A text displaying the compontents of the RGBA argument
	 **/
	static QString rgbaText(Lib3dsRgba rgba);
	/**
	 * @return A text displaying the compontents of the RGB argument
	 **/
	static QString rgbText(Lib3dsRgb rgb);

public slots:
	void slotUpdate();

protected slots:

	void slotModelChanged(int index);
	void slotFrameChanged(int frame);
	void slotConstructMeshList();
	void slotConstructNodeList();
	void slotDisplayMaterial(QListBoxItem*);
	void slotDisplayMesh(QListViewItem*);
	void slotDisplayNode(QListViewItem*);
	void slotConnectToFace(QListViewItem*);
	void slotUseLib3dsCoordinates(bool);
	void slotShowPointIndices(bool);
	void slotHideConnectableWidgets(bool);

protected:
	void updateMaterialPage();
	void updateMeshPage();
	void updateNodePage();

	void addNodeToList(QListViewItem* parent, Lib3dsNode* node);

private:
	void init();
	void initMaterialPage();
	void initMeshPage();
	void initNodePage();

	void addTextureMap(const QString& text, _Lib3dsTextureMap* map);

private:
	class KGame3DSModelDebugPrivate;
	KGame3DSModelDebugPrivate* d;
};


#include <qmap.h>
/**
 * @internal
 * Helper class.
 **/
class BoNodeTracksWidget : public QWidget
{
	Q_OBJECT
public:
	BoNodeTracksWidget(QWidget* parent);

	void setNodeObjectData(Lib3dsObjectData* d);

signals:
	void signalDisplayTrack(Bo3DSTrack* track);

protected slots:
	void slotButtonToggled(bool);

private:
	QPushButton* mPosition;
	QPushButton* mRotation;
	QPushButton* mScale;
	QPushButton* mMorph;
	QPushButton* mHide;
	QLabel* mPositionLabel;
	QLabel* mRotationLabel;
	QLabel* mScaleLabel;
	QLabel* mMorphLabel;
	QLabel* mHideLabel;
	Bo3DSTrackLin3* mPositionTrack;
	Bo3DSTrackQuat* mRotationTrack;
	Bo3DSTrackLin3* mScaleTrack;
	Bo3DSTrackBool* mHideTrack;
	Bo3DSTrackMorph* mMorphTrack;
	QMap<QPushButton*, Bo3DSTrack*> mButton2Track;
};

class BoNodeObjectDataWidget : public QWidget
{
	Q_OBJECT
public:
	BoNodeObjectDataWidget(QWidget* parent);

	void setNodeObjectData(Lib3dsObjectData* d);

signals:
	void signalDisplayTrack(Bo3DSTrack* track);

protected:
	QWidget* addWidget(const QString& label, QWidget* w);

private:
	QVBoxLayout* mLayout;

	QLabel* mPivot;
	QLabel* mInstance;
	QLabel* mBBoxMin;
	QLabel* mBBoxMax;
	QLabel* mPos;
	QLabel* mRot;
#if 0
	QLabel* mRotAngle;
#endif
	QLabel* mRotX;
	QLabel* mRotY;
	QLabel* mRotZ;
	QLabel* mScl;
	QLabel* mMorphSmooth;
	QLabel* mMorph;
	QCheckBox* mHide;
	BoNodeTracksWidget* mNodeTracks;
};

class BoTrackWidget : public QWidget
{
	Q_OBJECT
public:
	BoTrackWidget(QWidget* parent);

protected slots:
	void slotDisplayTrack(Bo3DSTrack*);

protected:
	QListViewItem* createItem(Bo3DSTrackKey* key, int type);

	void configureTCB(QListViewItem* item, const Bo3DSTrackTCB& tcb);
	void configureKey(QListViewItem* item, Bo3DSTrackKey* key, int type);

private:
	QLabel* mFlags;
	QLabel* mType;
	KListBox* mFlagList;
	KListView* mKeys;
	int mKeyData0;
	int mKeyData1;
	int mKeyData2;
	int mKeyData3;
	int mKeyData4;
};


#endif
