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

#include "speciestheme.h"
#include "unitproperties.h"
#include "bo3dtools.h"
#include "bo3dsload.h"
#include "bodebug.h"
#include "bomatrixwidget.h"

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
		addColumn(i18n("Face"));
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
		addColumn(i18n("Boson Normal"), metrics.width(QString::number(11111)));

		setAllColumnsShowFocus(true);
		resize(100, height());
	}

	~BoFaceView()
	{
	}

	QListViewItem* addFace(int index, Lib3dsFace* face, Lib3dsMesh* mesh, bool lib3dsCoordinates = true)
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
		item->setText(0, no);

		// note: calculating the inverse of the mesh matrix is slow but
		// we do it for every face. causes less work for the API - the
		// class is easier to use then.

		BoMatrix matrix;
		Lib3dsMatrix invMeshMatrix;
		lib3ds_matrix_copy(invMeshMatrix, mesh->matrix);
		lib3ds_matrix_inv(invMeshMatrix);
		matrix.loadMatrix(&invMeshMatrix[0][0]);

		BoVector3 meshVertex[3];
		BoVector3 v;
		for (int j = 0; j < 3; j++) {
			v.set(mesh->pointL[ face->points[j] ].pos);
			matrix.transform(&meshVertex[j], &v);
		}

		for (int j = 0; j < 3; j++) {
			if (lib3dsCoordinates) {
				v.set(mesh->pointL[ face->points[j] ].pos);
			} else {
				v = meshVertex[j];
			}
			item->setText(j + 1, QString("%1;%2;%3").arg(v[0]).arg(v[1]).arg(v[2]));
		}
		item->setText(4, face->material);
		QString flags = QString::number(face->flags);
		item->setText(5, flags);
		item->setText(6, QString::number(face->smoothing));
		item->setText(7, QString("%1;%2;%3").arg(face->normal[0]).arg(face->normal[1]).arg(face->normal[2]));

		BoVector3 p, q;
		p = meshVertex[2] - meshVertex[1];
		q = meshVertex[0] - meshVertex[1];
		BoVector3 normal = BoVector3::crossProduct(p, q);
		if (normal.length() != 0.0f) {
			normal.normalize();
		}
		item->setText(8, QString("%1;%2;%3").arg(normal[0]).arg(normal[1]).arg(normal[2]));
		return item;
	}
};

class BoNodeObjectDataWidget : public QWidget
{
public:
	BoNodeObjectDataWidget(QWidget* parent) : QWidget(parent, "nodeobjectdatawidget")
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
		mRot = (QLabel*)addWidget(i18n("Rotation (quaternion)"), new QLabel(this));
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
	}

	void setNodeObjectData(Lib3dsObjectData* d)
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
			pivot = QString("(%1,%2,%3)").arg(d->pivot[0]).arg(d->pivot[1]).arg(d->pivot[2]);
			instance = QString(d->instance);
			bboxMin = QString("(%1,%2,%3)").arg(d->bbox_min[0]).arg(d->bbox_min[1]).arg(d->bbox_min[2]);
			bboxMax = QString("(%1,%2,%3)").arg(d->bbox_max[0]).arg(d->bbox_max[1]).arg(d->bbox_max[2]);
			pos = QString("(%1,%2,%3)").arg(d->pos[0]).arg(d->pos[1]).arg(d->pos[2]);
			rot = QString("(%1,%2,%3,%4)").arg(d->rot[0]).arg(d->rot[1]).arg(d->rot[2]).arg(d->rot[3]);

			float rX = 0.0f, rY = 0.0f, rZ = 0.0f;
#if 0
			float angle = 0.0f;
			quatToAxisRotation(d->rot, &rX, &rY, &rZ, &angle);
			rotAngle = QString("(%1,%2,%3) -> %4 degrees").arg(rX).arg(rY).arg(rZ).arg(angle);
#endif
			quatToEulerAngles(d->rot, &rX, &rY, &rZ);
			rotX = QString::number(rX);
			rotY = QString::number(rY);
			rotZ = QString::number(rZ);

			scl = QString("(%1,%2,%3)").arg(d->scl[0]).arg(d->scl[1]).arg(d->scl[2]);
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
	}

protected:
	QWidget* addWidget(const QString& label, QWidget* w)
	{
		QWidget* box = new QWidget(this, "widgetbox");
		w->reparent(box, QPoint(0,0)); // ugly, but useful
		QHBoxLayout* l = new QHBoxLayout(box);
		l->addWidget(new QLabel(label, box, "label"));
		l->addWidget(w);
		mLayout->addWidget(box);

		return w;
	}

#if 0
	// AB: this functions doesn't work how i expect it.
	// i used the algorithms from the site mentioned below, but i haven't
	// found out what exactly it calculates (e.g. whether the angle is in
	// radians or degree, ...)
	// i am very sure that it does not calculate the rotation axis and it's
	// angle. or if it does, this implementation has a bug.
	// disabled, because it isn't important anymore - we use
	// quatToEulerAngles() instead, which is far more what we need.

	// convert quaternion to a rotation around an axis.
	void quatToAxisRotation(Lib3dsQuat _q, float* _x, float* _y, float* _z, float* _angle)
	{
		if (!_x || !_y || !_z || !_angle) {
			return;
		}
		// see e.g. http://www.j3d.org/matrix_faq/matrfaq_latest.html
		// for some useful information on quaternions (this algorithm is
		// from there!)
		Lib3dsQuat q;
		lib3ds_quat_copy(q, _q);
		lib3ds_quat_normalize(q);
		float cos_a = q[3]; // cos_a = w
		float angle = acos(cos_a) * 2;
		float sin_a = sqrt(1.0f - cos_a * cos_a);
		if (fabs(sin_a) < 0.0005) {
			sin_a = 1;
		}


		// the axis where we rotate
		*_x = q[0] / sin_a;
		*_y = q[1] / sin_a;
		*_z = q[2] / sin_a;

		// the angle that is used on the above axis
		*_angle = angle;
	}
#endif

	void quatToRotationMatrix(Lib3dsQuat _q, float* mat)
	{
		// note: mat is *NOT* a Lib3dsMatrix. it is a 16 element array,
		// in the format used by OpenGL.
		// see also http://www.j3d.org/matrix_faq/matrfaq_latest.html
		if (!mat) {
			return;
		}

		Lib3dsQuat q; // I am assuming this is of form (X,Y,Z,W), is that true?
		lib3ds_quat_copy(q, _q);
		lib3ds_quat_normalize(q);

		float xx = q[0] * q[0];
		float xy = q[0] * q[1];
		float xz = q[0] * q[2];
		float xw = q[0] * q[3];

		float yy = q[1] * q[1];
		float yz = q[1] * q[2];
		float yw = q[1] * q[3];

		float zz = q[2] * q[2];
		float zw = q[2] * q[3];

		mat[0] = 1.0f - 2.0f * ( yy + zz );
		mat[1] = 2.0f * ( xy - zw );
		mat[2] = 2.0f * ( xz + yw );
		mat[3] = 0.0f;

		mat[4] = 2.0f * ( xy + zw );
		mat[5] = 1.0f - 2.0f * ( xx + zz );
		mat[6] = 2.0f * ( yz - xw );
		mat[7] = 0.0f;

		mat[8] = 2.0f * ( xz - yw );
		mat[9] = 2.0f * ( yz + xw );
		mat[10] = 1.0f - 2.0f * ( xx + yy );
		mat[11] = 0.0f;

		mat[12] = 0.0f;
		mat[13] = 0.0f;
		mat[14] = 0.0f;
		mat[15] = 1.0f;
	}

	// convert a quaternion to so-called euler angles.
	// euler angles are angles around one of [x,y,z] axis. i.i
	// (1,0,0) or (0,1,0) or (0,0,1) in OpenGL's glRotate()
	void quatToEulerAngles(Lib3dsQuat _q, float* _x, float* _y, float* _z)
	{
		// hm
		// I cannot use quaternions properly so I am using docs only. I
		// have not yet found any docs that do this conversion directly,
		// so we convert to a rotation matrix first, then to euler
		// angles.

		Lib3dsQuat q; // I am assuming this is of form (X,Y,Z,W), is that true?
		lib3ds_quat_copy(q, _q);

		float mat[16];
		quatToRotationMatrix(q, mat);

		// now convert that matrix to euler angles.
		// see also http://www.j3d.org/matrix_faq/matrfaq_latest.html

		float angle_x, angle_y, angle_z;
		float D;
		angle_y = D =  asin(mat[2]); // Calculate Y-axis angle
		float C = cos(angle_y);
		angle_y = Bo3dTools::rad2deg(angle_y);

		float tr_x, tr_y;
		if (fabs(C) > 0.005) {
			tr_x =  mat[10] / C; // get X-axis angle
			tr_y = -mat[6]  / C;
			angle_x = Bo3dTools::rad2deg(atan2(tr_y, tr_x));
			tr_x =  mat[0] / C; // Get Z-axis angle
			tr_y = -mat[1] / C;
			angle_z  = Bo3dTools::rad2deg(atan2(tr_y, tr_x));
		} else { // gimball lock
			angle_x = 0; // Set X-axis angle to zero
			tr_x = mat[5]; // And calculate Z-axis angle
			tr_y = mat[4];
			angle_z = Bo3dTools::rad2deg(atan2(tr_y, tr_x));
		}

		// return only positive angles in [0,360]
		if (angle_x < 0) {
			angle_x += 360;
		}
		if (angle_y < 0) {
			angle_y += 360;
		}
		if (angle_z < 0) {
			angle_z += 360;
		}


		*_x = angle_x;
		*_y = angle_y;
		*_z = angle_z;
	}

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
		mNodePage = 0;

		mMaterialBox = 0;
		mMaterialData = 0;

		mMeshView = 0;
		mFaceList = 0;
		mConnectableWidget = 0;
		mConnectedFacesList = 0;
		mUnconnectedFacesList = 0;
		mUseLib3dsCoordinates = 0;
		mHideConnectableWidgets = 0;
		mMeshMatrix = 0;
		mInvMeshMatrix = 0;

		mNodeView = 0;
		mCurrentFrame = 0;
		mNodeMatrix = 0;
		mNodeObjectData = 0;

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
	BoMaterialWidget* mMaterialData;
	QPtrDict<Lib3dsMaterial> mListItem2Material;
	KListView* mTextureView;

	KListView* mMeshView;
	QPtrDict<Lib3dsMesh> mListItem2Mesh;
	QPtrDict<Lib3dsFace> mListItem2Face;
	BoFaceView* mFaceList;
	QVBox* mConnectableWidget;
	BoFaceView* mConnectedFacesList;
	BoFaceView* mUnconnectedFacesList;
	QCheckBox* mUseLib3dsCoordinates;
	QCheckBox* mHideConnectableWidgets;
	BoMatrixWidget* mMeshMatrix;
	BoMatrixWidget* mInvMeshMatrix;

	KListView* mNodeView;
	QPtrDict<Lib3dsNode> mListItem2Node;
	KIntNumInput* mCurrentFrame;
	BoMatrixWidget* mNodeMatrix;
	BoNodeObjectDataWidget* mNodeObjectData;

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
 d->mMeshView->addColumn(i18n("Name"), metrics.width(i18n("Name")));
 d->mMeshView->addColumn(i18n("Color"), metrics.width(QString::number(111)));
 d->mMeshView->addColumn(i18n("Points count"), metrics.width(QString::number(111)));
 d->mMeshView->addColumn(i18n("Texels count"), metrics.width(QString::number(111)));
 d->mMeshView->addColumn(i18n("Faces count"), metrics.width(QString::number(111)));
 d->mMeshView->addColumn(i18n("Flags count"), metrics.width(QString::number(11)));
 d->mMeshView->addColumn(i18n("Max point index"), metrics.width(QString::number(11)));
 connect(d->mMeshView, SIGNAL(executed(QListViewItem*)), this, SLOT(slotDisplayMesh(QListViewItem*)));
 QVBox* modelInfo = new QVBox(meshView); // actually it doesn't fit to "meshView", but rather display info about the model
 QHBox* faces = new QHBox(modelInfo);
 (void)new QLabel(i18n("Mesh Faces: "), faces);
 d->mMeshFacesCountLabel = new QLabel(faces);
 QToolTip::add(d->mMeshFacesCountLabel, i18n("This is the total number of faces in (different) meshes. Note that every mesh can appear several times in a model, so this number is <em>not</em> the total number of faces in the model!"));
 QHBox* vertices = new QHBox(modelInfo);
 (void)new QLabel(i18n("Mesh Vertices (Faces * 3): "), vertices);
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
 d->mHideConnectableWidgets = new QCheckBox(i18n("Hide \"connectable\" widgets"), faceView);
 d->mHideConnectableWidgets->setChecked(true);
 connect(d->mHideConnectableWidgets, SIGNAL(toggled(bool)), this, SLOT(slotHideConnectableWidgets(bool)));
 slotHideConnectableWidgets(d->mHideConnectableWidgets->isChecked());
 QToolTip::add(d->mHideConnectableWidgets, i18n("The \"connectable\" widgets show which faces of a mesh are connectable to a certain face. This is important to make GL_TRIANGLE_STRIPs, but the code used here is obsolete."));


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

 QVGroupBox* nodeMatrixBox = new QVGroupBox(i18n("Matrix"), splitter);
 d->mNodeMatrix = new BoMatrixWidget(nodeMatrixBox);


 d->mCurrentFrame = new KIntNumInput(d->mNodePage);
 d->mCurrentFrame->setLabel(i18n("Frame"));
 connect(d->mCurrentFrame, SIGNAL(valueChanged(int)),
		this, SLOT(slotFrameChanged(int)));
 l->addWidget(d->mCurrentFrame);

 d->mTabWidget->addTab(d->mNodePage, i18n("&Nodes"));
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
 
 // FIXME: duplicated code
 QString fileName = theme->themePath() + QString::fromLatin1("objects/objects.boson");
 if (!KStandardDirs::exists(fileName)) {
	boDebug() << k_funcinfo << "no objects.boson file found" << endl;
	// We assume that this theme has no objects and don't complain
	return;
 }

 KSimpleConfig cfg(fileName);
 QStringList objects = cfg.groupList();
 QStringList::Iterator oit;
 for (oit = objects.begin(); oit != objects.end(); ++oit) {
	cfg.setGroup(*oit);
	float width, height;
	QString file;
	width = (float)cfg.readDoubleNumEntry("Width", 1.0);
	height = (float)cfg.readDoubleNumEntry("Height", 1.0);
	file = cfg.readEntry("File", "missile.3ds");
	addModel(theme->themePath() + QString::fromLatin1("objects/") + file, *oit);
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
	if (strcmp(node->name, "$$$DUMMY") == 0) {
		continue;
	}
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
 if (strcmp(node->name, "$$$DUMMY") == 0) {
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
	QListViewItem* item = d->mFaceList->addFace(i, face, mesh, d->mUseLib3dsCoordinates->isChecked());
	d->mListItem2Face.insert(item, face);
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
 Lib3dsNode* node = d->mListItem2Node[item];

 if (!node) {
	boWarning() << k_funcinfo << "NULL node" << endl;
	return;
 }

 d->mNodeMatrix->setMatrix(node->matrix);
 d->mNodeObjectData->setNodeObjectData(&node->data.object);
}

void KGameModelDebug::slotFrameChanged(int frame)
{
 if (!d->m3ds) {
	return;
 }
 boDebug() << k_funcinfo << endl;
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
 for (unsigned int i = 0; i < mesh->faces; i++) {
	Lib3dsFace* f = &mesh->faceL[i];
	if (connected.contains(f)) {
		d->mConnectedFacesList->addFace(i, f, mesh, d->mUseLib3dsCoordinates->isChecked());
	} else {
		d->mUnconnectedFacesList->addFace(i, f, mesh, d->mUseLib3dsCoordinates->isChecked());
	}
 }
}

void KGameModelDebug::slotHideConnectableWidgets(bool h)
{
 if (h) {
	d->mConnectableWidget->hide();
 } else {
	d->mConnectableWidget->show();
 }
}
