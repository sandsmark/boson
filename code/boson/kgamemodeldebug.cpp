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

#include "kgamemodeldebug.h"
#include "kgamemodeldebug.moc"

#include "bosonmodel.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "bo3dtools.h"
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

#include <klistbox.h>
#include <klistview.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include <lib3ds/file.h>
#include <lib3ds/node.h>
#include <lib3ds/mesh.h>
#include <lib3ds/material.h>
#include <lib3ds/vector.h>

/**
 * Display a matrix. We use a @ref QGrid here, in order to display it like this:
 * <pre>
 * 1 0  0   0
 * 0 10 0   0
 * 0 0  100 0
 * 0 0  0   1
 * </pre>
 * and not like this
 * <pre>
 * 1 0 0 0
 * 1 10 0 0
 * 1 0 0 100 0
 * 1 0 0 1
 * </pre>
 **/
class BoMatrixWidget : public QWidget
{
public:
	BoMatrixWidget(QWidget* parent) : QWidget(parent)
	{
		mLayout = new QGridLayout(this, 0, 1);
		for (int i = 0; i < 16; i++) {
			QLabel* l = new QLabel(this);
			mLabel.insert(i, l);
			mLayout->addWidget(l, i % 4, i / 4);
		}
	}
	~BoMatrixWidget()
	{
	}

	void setMatrix(BoMatrix* m)
	{
		if (!m) {
			boError() << k_funcinfo << "NULL matrix" << endl;
			return;
		}
		for (int i = 0; i < 4; i++) {
			const float* d = m->data();
			mLabel[i + 0]->setText(QString::number(d[i + 0]));
			mLabel[i + 4]->setText(QString::number(d[i + 4]));
			mLabel[i + 8]->setText(QString::number(d[i + 8]));
			mLabel[i + 12]->setText(QString::number(d[i + 12]));
		}
	}

	void setMatrix(Lib3dsMatrix m)
	{
		BoMatrix matrix;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				matrix.setElement(i, j, m[j][i]);
			}
		}
		setMatrix(&matrix);
	}

	void clear()
	{
		for (int i = 0; i < 16; i++) {
			mLabel[i]->setText("");
		}
	}

private:
	QGridLayout* mLayout;
	QIntDict<QLabel> mLabel;
};

class BoMaterialWidget : public QWidget
{
public:
	BoMaterialWidget(QWidget* parent) : QWidget(parent)
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

	~BoMaterialWidget()
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
			mShading->setText(QString::number(m->shading));
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
		QFontMetrics metrics(font());
		setShowToolTips(true);
		addColumn(i18n("Point1"));
		addColumn(i18n("Point2"));
		addColumn(i18n("Point3"));

		// we try to keep the size as low as possible here - the list is
		// too wide anyway.
		// the titles won't be displayed, but the content should display
		// fine at least in most cases.
		addColumn(i18n("Material"), metrics.width(i18n("Material")));
		addColumn(i18n("Flags"), metrics.width(QString::number(11)));
		addColumn(i18n("Smoothing"), metrics.width(QString::number(1111)));
		addColumn(i18n("Normal"), metrics.width(QString::number(11111)));

		resize(100, height());
	}

	~BoFaceView()
	{
	}

	QListViewItem* addFace(Lib3dsFace* face, Lib3dsMesh* mesh)
	{
		QListViewItem* item = new QListViewItem(this);
		for (int j = 0; j < 3; j++) {
			Lib3dsVector v;
			lib3ds_vector_copy(v, mesh->pointL[ face->points[j] ].pos);
			item->setText(j, QString("%1;%2;%3").arg(v[0]).arg(v[1]).arg(v[2]));
		}
		item->setText(3, face->material);
		QString flags = QString::number(face->flags);
		item->setText(4, flags);
		item->setText(5, QString::number(face->smoothing));
		item->setText(6, QString("%1;%2;%3").arg(face->normal[0]).arg(face->normal[1]).arg(face->normal[2]));
		return item;
	}
};

BoListView::BoListView(QWidget* parent) : KListView(parent)
{
 mPopup = 0;
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

		mMaterialBox = 0;
		mMaterialData = 0;

		mMeshView = 0;
		mFaceList = 0;
		mConnectedFacesList = 0;
		mUnconnectedFacesList = 0;
		mMeshMatrix = 0;

		m3ds = 0;
	}

	QVBoxLayout* mTopLayout;
	QComboBox* mModelBox;
	QMap<int, QString> mModelFiles;

	QTabWidget* mTabWidget;
	QWidget* mMaterialPage;
	QWidget* mMeshPage;

	KListBox* mMaterialBox;
	BoMaterialWidget* mMaterialData;
	QPtrDict<Lib3dsMaterial> mListItem2Material;
	KListView* mTextureView;

	KListView* mMeshView;
	QPtrDict<Lib3dsMesh> mListItem2Mesh;
	QPtrDict<Lib3dsFace> mListItem2Face;
	BoFaceView* mFaceList;
	BoFaceView* mConnectedFacesList;
	BoFaceView* mUnconnectedFacesList;
	BoMatrixWidget* mMeshMatrix;

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

 d->mMaterialData = new BoMaterialWidget(splitter);

 d->mTextureView = new KListView(splitter);
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
 d->mMeshPage = new QWidget(d->mTabWidget);
 QHBoxLayout* l = new QHBoxLayout(d->mMeshPage, 10, 10);
 QSplitter* splitter = new QSplitter(d->mMeshPage);
 l->addWidget(splitter);
 QFontMetrics metrics(font());

 QVBox* meshView = new QVBox(splitter);
 d->mMeshView = new KListView(meshView);
 d->mMeshView->addColumn(i18n("Name"), metrics.width(i18n("Name")));
 d->mMeshView->addColumn(i18n("Color"), metrics.width(QString::number(111)));
 d->mMeshView->addColumn(i18n("Points count"), metrics.width(QString::number(111)));
 d->mMeshView->addColumn(i18n("Texels count"), metrics.width(QString::number(111)));
 d->mMeshView->addColumn(i18n("Faces count"), metrics.width(QString::number(111)));
 d->mMeshView->addColumn(i18n("Flags count"), metrics.width(QString::number(11)));
 connect(d->mMeshView, SIGNAL(executed(QListViewItem*)), this, SLOT(slotDisplayMesh(QListViewItem*)));
 d->mMeshView->resize(d->mMeshView->height(), 150);

 QVBox* faceView = new QVBox(splitter);
 d->mFaceList = new BoFaceView(faceView);
 connect(d->mFaceList, SIGNAL(executed(QListViewItem*)), this, SLOT(slotConnectToFace(QListViewItem*)));
 (void)new QLabel(i18n("Connectable to selected face:"), faceView);
 d->mConnectedFacesList = new BoFaceView(faceView);
 (void)new QLabel(i18n("Unconnectable to selected face:"), faceView);
 d->mUnconnectedFacesList = new BoFaceView(faceView);

 QVGroupBox* meshMatrixBox = new QVGroupBox(i18n("Matrix"), splitter);
 d->mMeshMatrix = new BoMatrixWidget(meshMatrixBox);

 d->mTabWidget->addTab(d->mMeshPage, i18n("&Meshes"));
}

void KGameModelDebug::addModel(const QString& file, const QString& _name)
{
 unsigned int i = d->mModelBox->count();
 QString name = _name.isEmpty() ? QString::number(i) : _name;
 d->mModelFiles.insert(i, file);
 d->mModelBox->insertItem(name);
}

void KGameModelDebug::addTheme(SpeciesTheme* theme)
{
 if (!theme) {
	boError() << k_funcinfo << "NULL theme" << endl;
	return;
 }
 QValueList<const UnitProperties*> prop = theme->allUnits();
 QValueList<const UnitProperties*>::Iterator it;
 for (it = prop.begin(); it != prop.end(); ++it) {
	QString file = (*it)->unitPath() + SpeciesTheme::unitModelFile();
	addModel(file, (*it)->name());
 }
 QStringList objects = theme->allObjectModels();
 for (unsigned int i = 0; i < objects.count(); i++) {
	addModel(theme->themePath() + QString::fromLatin1("objects/") + objects[i], objects[i]);
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
 d->mListItem2Mesh.clear();
 d->mListItem2Face.clear();

 if (!d->m3ds) {
	return;
 }
 slotConstructMeshList();
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
	d->mListItem2Mesh.insert(item, mesh);
 }
}

void KGameModelDebug::slotDisplayMesh(QListViewItem* item)
{
 d->mFaceList->clear();
 d->mListItem2Face.clear();

 Lib3dsMesh* mesh = d->mListItem2Mesh[item];
 if (!mesh) {
	boWarning() << k_funcinfo << "NULL mesh" << endl;
	return;
 }

 // faces
 for (unsigned int i = 0; i < mesh->faces; i++) {
	Lib3dsFace* face = &mesh->faceL[i];
	QListViewItem* item = d->mFaceList->addFace(face, mesh);
	d->mListItem2Face.insert(item, face);
 }

 // mesh->matrix
 d->mMeshMatrix->setMatrix(mesh->matrix);

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
 BosonModel::findAdjacentFaces(&connected, mesh, face);
 QPtrList<Lib3dsFace> faces;
 for (unsigned int i = 0; i < mesh->faces; i++) {
	Lib3dsFace* f = &mesh->faceL[i];
	if (connected.contains(f)) {
		d->mConnectedFacesList->addFace(f, mesh);
	} else {
		d->mUnconnectedFacesList->addFace(f, mesh);
	}
 }
}

