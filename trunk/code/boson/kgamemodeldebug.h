/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef KGAMEMODELDEBUG_H
#define KGAMEMODELDEBUG_H

#include <qwidget.h>

#include <klistview.h>

#include <lib3ds/types.h>

class QListBoxItem;
class QListViewItem;
class QLabel;
class KPopupMenu;

class BosonModel;
class SpeciesTheme;
struct _Lib3dsTextureMap;

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
 * Here the user can select a model from a list (see @ref addModel and @ref
 * addTheme). For this model pretty much all available data are displayed.
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
class KGameModelDebug : public QWidget
{
	Q_OBJECT
public:
	KGameModelDebug(QWidget* parent);
	~KGameModelDebug();

	/**
	 * Note that we'll reparse the .3ds file! Most data are discarded in
	 * BosonModel, so we need to do so.
	 * @param name The text for the combo box. QString::null for the index.
	 **/
	void addModel(const QString& file, const QString& name = QString::null);

	/**
	 * Calls @ref addModel for all units in this theme. (maybe for
	 * non-units, too, don't know yet
	 **/
	void addTheme(SpeciesTheme* theme);

	static QString rgbaText(Lib3dsRgba rgba);
	static QString rgbText(Lib3dsRgba rgb);

public slots:
	void slotUpdate();

protected slots:

	void slotModelChanged(int index);
	void slotConstructMeshList();
	void slotDisplayMaterial(QListBoxItem*);
	void slotDisplayMesh(QListViewItem*);
	void slotConnectToFace(QListViewItem*);

protected:
	void updateMaterialPage();
	void updateMeshPage();

private:
	void init();
	void initMaterialPage();
	void initMeshPage();

	void addTextureMap(const QString& text, _Lib3dsTextureMap* map);

private:
	class KGameModelDebugPrivate;
	KGameModelDebugPrivate* d;
};

#endif
