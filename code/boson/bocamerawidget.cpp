/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bocamerawidget.h"
#include "bocamerawidget.moc"

#include "bocamera.h"
#include "bodebug.h"
#include "bo3dtools.h"
#include "bosonwidgets/bonuminput.h"
#include "bomatrixwidget.h"
#include "boorbiterwidget.h"

#include <qlayout.h>
#include <qvgroupbox.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qtooltip.h>
#include <qmap.h>
#include <qvbox.h>
#include <qhbox.h>

#include <knuminput.h>
#include <klocale.h>

#include <math.h>
#include <GL/glu.h>
#include <GL/gl.h>

// boson's game restrictions
#define MIN_LOOKAT_X -50.0f
#define MAX_LOOKAT_X 50.0f
#define MIN_LOOKAT_Y MIN_LOOKAT_X
#define MAX_LOOKAT_Y MAX_LOOKAT_X
#define MIN_LOOKAT_Z 0.0f
#define MAX_LOOKAT_Z 50.0f
#define MIN_CAMERAPOS_X MIN_LOOKAT_X
#define MIN_CAMERAPOS_Y MIN_LOOKAT_Y
#define MIN_CAMERAPOS_Z MIN_LOOKAT_Z
#define MAX_CAMERAPOS_X MAX_LOOKAT_X
#define MAX_CAMERAPOS_Y MAX_LOOKAT_Y
#define MAX_CAMERAPOS_Z MAX_LOOKAT_Z


/**
 * This takes a <em>rotation</em> matrix that was translated and returns the
 * translation vector.
 *
 * Note that no other operation except of these two may have happened!
 **/
BoVector3 matrixUntranslate(const BoMatrix& rotationMatrix);


class BoCameraWidgetPrivate
{
public:
	BoCameraWidgetPrivate()
	{
		mTab = 0;

		mCamera = 0;
	}
	QTabWidget* mTab;
	QPtrList<BoCameraConfigWidgetBase> mConfigWidgets;
	QMap<BoCameraConfigWidgetBase*, QString> mConfigWidget2Name;

	BoCamera* mCamera;
};


BoCameraWidget::BoCameraWidget(QWidget* parent, const char* name) : QWidget(parent, name)
{
 d = new BoCameraWidgetPrivate;
 QVBoxLayout* topLayout = new QVBoxLayout(this);

 d->mTab = new QTabWidget(this, "cameratabwidget");
 topLayout->addWidget(d->mTab);

 BoGameCameraWidget* bosonCamera = new BoGameCameraWidget(d->mTab, "gamecamera");
 BoGLUCameraWidget* gluLookAtCamera = new BoGLUCameraWidget(d->mTab, "glulookatcamera");
 BoPlainCameraWidget* plainCamera = new BoPlainCameraWidget(d->mTab, "plaincamera");
 BoOrbiterCameraWidget* orbiterCamera = new BoOrbiterCameraWidget(d->mTab, "orbitercamera");

 addConfigWidget(i18n("Orbiter camera"), orbiterCamera);
 addConfigWidget(i18n("Game camera"), bosonCamera);
 addConfigWidget(i18n("translate/rotate camera"), plainCamera);
 addConfigWidget(i18n("gluLookAt camera"), gluLookAtCamera);
}

BoCameraWidget::~BoCameraWidget()
{
 d->mConfigWidgets.clear();
 delete d;
}

void BoCameraWidget::addConfigWidget(const QString& name, BoCameraConfigWidgetBase* c)
{
 BO_CHECK_NULL_RET(c);
 d->mConfigWidgets.append(c);
 d->mConfigWidget2Name.insert(c, name);
 c->setCamera(d->mCamera);
 connect(c, SIGNAL(signalCameraChanged()),
		this, SLOT(slotUpdateFromCamera()));
 if (d->mCamera) {
	d->mTab->addTab(c, d->mConfigWidget2Name[c]);
	c->updateFromCamera();
 } else {
	c->hide();
 }
}

void BoCameraWidget::slotUpdateFromCamera()
{
 QPtrListIterator<BoCameraConfigWidgetBase> it(d->mConfigWidgets);
 for (; it.current(); ++it) {
	if (it.current()->camera()) {
		if (!it.current()->updatesBlocked()) {
			it.current()->updateFromCamera();
		}
	}
 }
}

void BoCameraWidget::setCamera(BoCamera* camera)
{
 d->mCamera = camera;
 int cameraType = -1;
 if (d->mCamera) {
	cameraType = d->mCamera->cameraType();
 }
 QPtrListIterator<BoCameraConfigWidgetBase> it(d->mConfigWidgets);
 for (; it.current(); ++it) {
	d->mTab->removePage(it.current());
	it.current()->hide();
	if (it.current()->needCameraType() != BoCamera::Camera) {
		if (it.current()->needCameraType() != cameraType) {
			it.current()->setCamera(0);
			continue;
		}
	}
	d->mTab->addTab(it.current(), d->mConfigWidget2Name[it.current()]);
	it.current()->setCamera(d->mCamera);
	it.current()->show();
 }
 slotUpdateFromCamera();
}



BoCameraConfigWidgetBase::BoCameraConfigWidgetBase(QWidget* parent, const char* name)
	: QWidget(parent, name)
{
 mCamera = 0;

 mUpdatesBlocked = false;
}


class BoGameCameraWidgetPrivate
{
public:
	BoGameCameraWidgetPrivate()
	{
		mLookAtX = 0;
		mLookAtY = 0;
		mLookAtZ = 0;
		mRotation = 0;
		mRadius = 0;

		mGameRestrictions = 0;
	}
	BoFloatNumInput* mLookAtX;
	BoFloatNumInput* mLookAtY;
	BoFloatNumInput* mLookAtZ;
	BoIntNumInput* mRotation;
	BoFloatNumInput* mRadius;

	// allow to use any values, without game restrictions, for inputs
	QCheckBox* mGameRestrictions;
};

BoGameCameraWidget::BoGameCameraWidget(QWidget* parent, const char* name)
	: BoCameraConfigWidgetBase(parent, name)
{
 d = new BoGameCameraWidgetPrivate;

 QVBoxLayout* layout = new QVBoxLayout(this);
 d->mLookAtX = new BoFloatNumInput(this);
 d->mLookAtY = new BoFloatNumInput(this);
 d->mLookAtZ = new BoFloatNumInput(this);
 d->mRotation = new BoIntNumInput(this);
 d->mRadius = new BoFloatNumInput(this);

 layout->addWidget(d->mLookAtX);
 layout->addWidget(d->mLookAtY);
 layout->addWidget(d->mLookAtZ);
 layout->addWidget(d->mRotation);
 layout->addWidget(d->mRadius);

 d->mLookAtX->setLabel(i18n("Look at X:"), AlignLeft | AlignVCenter);
 d->mLookAtY->setLabel(i18n("Look at Y:"), AlignLeft | AlignVCenter);
 d->mLookAtZ->setLabel(i18n("Look at Z:"), AlignLeft | AlignVCenter);
 d->mRotation->setLabel(i18n("Rotation:"), AlignLeft | AlignVCenter);
 d->mRadius->setLabel(i18n("Radius:"), AlignLeft | AlignVCenter);

 d->mGameRestrictions = new QCheckBox(i18n("Use game restrictions"), this);
 d->mGameRestrictions->setChecked(true);
 connect(d->mGameRestrictions, SIGNAL(toggled(bool)),
		this, SLOT(slotToggleGameRestrictions()));
 layout->addWidget(d->mGameRestrictions);
 slotToggleGameRestrictions();

 connect(d->mLookAtX, SIGNAL(signalValueChanged(float)), this, SLOT(slotLookAtChanged()));
 connect(d->mLookAtY, SIGNAL(signalValueChanged(float)), this, SLOT(slotLookAtChanged()));
 connect(d->mLookAtZ, SIGNAL(signalValueChanged(float)), this, SLOT(slotLookAtChanged()));
 connect(d->mRotation, SIGNAL(signalValueChanged(int)), this, SLOT(slotRotationChanged()));
 connect(d->mRadius, SIGNAL(signalValueChanged(float)), this, SLOT(slotRadiusChanged()));

 QLabel* text = new QLabel(this);
 text->setText(i18n("Note: The game camera can not display all points of view. So if you change the camera somewhere else the values here will be out of sync!"));
 text->setAlignment(text->alignment() | WordBreak);
 layout->addWidget(text);
}

BoGameCameraWidget::~BoGameCameraWidget()
{
 delete d;
}

int BoGameCameraWidget::needCameraType() const
{
 return BoCamera::GameCamera;
}

void BoGameCameraWidget::updateFromCamera()
{
 BO_CHECK_NULL_RET(gameCamera());
 d->mLookAtX->setValue(gameCamera()->lookAt().x(), false);
 d->mLookAtY->setValue(gameCamera()->lookAt().y(), false);
 d->mLookAtZ->setValue(gameCamera()->lookAt().z(), false);
 d->mRadius->setValue(gameCamera()->radius(), false);
 d->mRotation->setValue((int)gameCamera()->rotation(), false);
}

void BoGameCameraWidget::slotLookAtChanged()
{
 BO_CHECK_NULL_RET(camera());
 BoVector3 lookAt(d->mLookAtX->value(), d->mLookAtY->value(), d->mLookAtZ->value());
 camera()->setLookAt(lookAt);
 emitSignalCameraChanged();
}

void BoGameCameraWidget::slotRotationChanged()
{
 BO_CHECK_NULL_RET(gameCamera());
 gameCamera()->setRotation((float)d->mRotation->value());
 emitSignalCameraChanged();
}

void BoGameCameraWidget::slotRadiusChanged()
{
 BO_CHECK_NULL_RET(camera());
 gameCamera()->setRadius((float)d->mRadius->value());
 emitSignalCameraChanged();
}

void BoGameCameraWidget::slotToggleGameRestrictions()
{
 d->mRotation->setRange(-180, 180, 1, true);
 if (!d->mGameRestrictions->isChecked()) {
	d->mLookAtX->setRange(MIN_LOOKAT_X, MAX_LOOKAT_X, 0.1f, true);
	d->mLookAtY->setRange(MIN_LOOKAT_Y, MAX_LOOKAT_Y, 0.1f, true);
	d->mLookAtZ->setRange(-50.0f, 50.0f, 0.1f, true);
	d->mRadius->setRange(-50.0f, 50.0f, 0.2f, true);
 } else {
	d->mLookAtX->setRange(MIN_LOOKAT_X, MAX_LOOKAT_X, 0.1f, true);
	d->mLookAtY->setRange(MIN_LOOKAT_Y, MAX_LOOKAT_Y, 0.1f, true);
	d->mLookAtZ->setRange(MIN_LOOKAT_Z, MAX_LOOKAT_Z, 0.1f, true);
	d->mRadius->setRange(MIN_LOOKAT_Z, MAX_LOOKAT_Z, 0.2f, true);
 }
}


class BoGLUCameraWidgetPrivate
{
public:
	BoGLUCameraWidgetPrivate()
	{
		mLookAtX = 0;
		mLookAtY = 0;
		mLookAtZ = 0;
		mCameraPosX = 0;
		mCameraPosY = 0;
		mCameraPosZ = 0;
		mUpX = 0;
		mUpY = 0;
		mUpZ = 0;

		mOrientation = 0;
	}
	BoFloatNumInput* mLookAtX;
	BoFloatNumInput* mLookAtY;
	BoFloatNumInput* mLookAtZ;
	BoFloatNumInput* mCameraPosX;
	BoFloatNumInput* mCameraPosY;
	BoFloatNumInput* mCameraPosZ;
	BoFloatNumInput* mUpX;
	BoFloatNumInput* mUpY;
	BoFloatNumInput* mUpZ;

	QLabel* mOrientation;
};

BoGLUCameraWidget::BoGLUCameraWidget(QWidget* parent, const char* name)
	: BoCameraConfigWidgetBase(parent, name)
{
 d = new BoGLUCameraWidgetPrivate;

 QVBoxLayout* layout = new QVBoxLayout(this);
 d->mLookAtX = new BoFloatNumInput(this);
 d->mLookAtY = new BoFloatNumInput(this);
 d->mLookAtZ = new BoFloatNumInput(this);
 d->mCameraPosX = new BoFloatNumInput(this);
 d->mCameraPosY = new BoFloatNumInput(this);
 d->mCameraPosZ = new BoFloatNumInput(this);
 d->mUpX = new BoFloatNumInput(this);
 d->mUpY = new BoFloatNumInput(this);
 d->mUpZ = new BoFloatNumInput(this);

 QHBox* orientationHBox = new QHBox(this);
 (void)new QLabel(i18n("Orientation: "), orientationHBox);
 d->mOrientation = new QLabel(orientationHBox);

 layout->addWidget(d->mLookAtX);
 layout->addWidget(d->mLookAtY);
 layout->addWidget(d->mLookAtZ);
 layout->addWidget(d->mCameraPosX);
 layout->addWidget(d->mCameraPosY);
 layout->addWidget(d->mCameraPosZ);
 layout->addWidget(d->mUpX);
 layout->addWidget(d->mUpY);
 layout->addWidget(d->mUpZ);
 layout->addWidget(orientationHBox);

 d->mLookAtX->setLabel(i18n("Look at X:"), AlignLeft | AlignVCenter);
 d->mLookAtY->setLabel(i18n("Look at Y:"), AlignLeft | AlignVCenter);
 d->mLookAtZ->setLabel(i18n("Look at Z:"), AlignLeft | AlignVCenter);
 d->mCameraPosX->setLabel(i18n("Camera position X:"), AlignLeft | AlignVCenter);
 d->mCameraPosY->setLabel(i18n("Camera position Y:"), AlignLeft | AlignVCenter);
 d->mCameraPosZ->setLabel(i18n("Camera position Z:"), AlignLeft | AlignVCenter);
 d->mUpX->setLabel(i18n("Up vector X:"), AlignLeft | AlignVCenter);
 d->mUpY->setLabel(i18n("Up vector Y:"), AlignLeft | AlignVCenter);
 d->mUpZ->setLabel(i18n("Up vector Z:"), AlignLeft | AlignVCenter);

 d->mLookAtX->setRange(MIN_LOOKAT_X, MAX_LOOKAT_X, 0.1f, true);
 d->mLookAtY->setRange(MIN_LOOKAT_Y, MAX_LOOKAT_Y, 0.1f, true);
 d->mLookAtZ->setRange(MIN_LOOKAT_Z, MAX_LOOKAT_Z, 0.1f, true);
 d->mCameraPosX->setRange(MIN_CAMERAPOS_X, MAX_CAMERAPOS_X, 0.1f, true);
 d->mCameraPosY->setRange(MIN_CAMERAPOS_Y, MAX_CAMERAPOS_Y, 0.1f, true);
 d->mCameraPosZ->setRange(-50.0f, 50.0f, 0.1f, true);
 d->mUpX->setRange(MIN_LOOKAT_X, MAX_LOOKAT_X, 0.1f, true);
 d->mUpY->setRange(MIN_LOOKAT_Y, MAX_LOOKAT_Y, 0.1f, true);
 d->mUpZ->setRange(MIN_LOOKAT_Z, MAX_LOOKAT_Z, 0.1f, true);

 connect(d->mLookAtX, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotLookAtChanged()));
 connect(d->mLookAtY, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotLookAtChanged()));
 connect(d->mLookAtZ, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotLookAtChanged()));
 connect(d->mCameraPosX, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotCameraPosChanged()));
 connect(d->mCameraPosY, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotCameraPosChanged()));
 connect(d->mCameraPosZ, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotCameraPosChanged()));
 connect(d->mUpX, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotUpChanged()));
 connect(d->mUpY, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotUpChanged()));
 connect(d->mUpZ, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotUpChanged()));
}

void BoGLUCameraWidget::updateMatrixWidget()
{
 BoVector3 cameraPos = BoVector3(d->mCameraPosX->value(), d->mCameraPosY->value(), d->mCameraPosZ->value());
 BoVector3 lookAt = BoVector3(d->mLookAtX->value(), d->mLookAtY->value(), d->mLookAtZ->value());
 BoVector3 orientation = cameraPos - lookAt;
 d->mOrientation->setText(i18n("%1").arg(orientation.debugString()));
}

BoGLUCameraWidget::~BoGLUCameraWidget()
{
 delete d;
}

int BoGLUCameraWidget::needCameraType() const
{
 return BoCamera::Camera;
}

void BoGLUCameraWidget::slotLookAtChanged()
{
 BO_CHECK_NULL_RET(camera());
 BoVector3 lookAt(d->mLookAtX->value(), d->mLookAtY->value(), d->mLookAtZ->value());
 camera()->setGluLookAt(camera()->cameraPos(), lookAt, camera()->up());
 emitSignalCameraChanged();
}

void BoGLUCameraWidget::slotCameraPosChanged()
{
 BO_CHECK_NULL_RET(camera());
 BoVector3 cameraPos(d->mCameraPosX->value(), d->mCameraPosY->value(), d->mCameraPosZ->value());
 camera()->setGluLookAt(cameraPos, camera()->lookAt(), camera()->up());
 emitSignalCameraChanged();
}

void BoGLUCameraWidget::slotUpChanged()
{
 BO_CHECK_NULL_RET(camera());
 BoVector3 up(d->mUpX->value(), d->mUpY->value(), d->mUpZ->value());
 camera()->setGluLookAt(camera()->cameraPos(), camera()->lookAt(), up);
 emitSignalCameraChanged();
}

void BoGLUCameraWidget::updateFromCamera()
{
 BO_CHECK_NULL_RET(camera());
 d->mLookAtX->setValue(camera()->lookAt().x(), false);
 d->mLookAtY->setValue(camera()->lookAt().y(), false);
 d->mLookAtZ->setValue(camera()->lookAt().z(), false);
 d->mCameraPosX->setValue(camera()->cameraPos().x(), false);
 d->mCameraPosY->setValue(camera()->cameraPos().y(), false);
 d->mCameraPosZ->setValue(camera()->cameraPos().z(), false);
 d->mUpX->setValue(camera()->up().x(), false);
 d->mUpY->setValue(camera()->up().y(), false);
 d->mUpZ->setValue(camera()->up().z(), false);

 updateMatrixWidget();
}


class BoPlainCameraWidgetPrivate
{
public:
	BoPlainCameraWidgetPrivate()
	{
		mCameraPosX = 0;
		mCameraPosY = 0;
		mCameraPosZ = 0;
		mRotateX = 0;
		mRotateY = 0;
		mRotateZ = 0;

		mMatrices = 0;
		mMatrix = 0;
		mCameraMatrix = 0;
		mEulerToGluLookAtMatrix = 0;

		mTranslateFirst = 0;
		mShowMatrices = 0;
	}
	BoFloatNumInput* mCameraPosX;
	BoFloatNumInput* mCameraPosY;
	BoFloatNumInput* mCameraPosZ;
	BoFloatNumInput* mRotateX;
	BoFloatNumInput* mRotateY;
	BoFloatNumInput* mRotateZ;

	QVBox* mMatrices;
	BoMatrixWidget* mMatrix;
	BoMatrixWidget* mCameraMatrix;
	BoMatrixWidget* mEulerToGluLookAtMatrix;

	QCheckBox* mTranslateFirst;
	QCheckBox* mShowMatrices;
};

BoPlainCameraWidget::BoPlainCameraWidget(QWidget* parent, const char* name)
	: BoCameraConfigWidgetBase(parent, name)
{
 d = new BoPlainCameraWidgetPrivate;

 QVBoxLayout* layout = new QVBoxLayout(this);
 d->mCameraPosX = new BoFloatNumInput(this);
 d->mCameraPosY = new BoFloatNumInput(this);
 d->mCameraPosZ = new BoFloatNumInput(this);
 d->mRotateX = new BoFloatNumInput(this);
 d->mRotateY = new BoFloatNumInput(this);
 d->mRotateZ = new BoFloatNumInput(this);

 // note: *first* rotate, then camerapos
 // --> gluLookAt() does this equally. translation to eye pos happens at the
 // end.
 layout->addWidget(d->mRotateX);
 layout->addWidget(d->mRotateY);
 layout->addWidget(d->mRotateZ);
 layout->addWidget(d->mCameraPosX);
 layout->addWidget(d->mCameraPosY);
 layout->addWidget(d->mCameraPosZ);

 d->mCameraPosX->setLabel(i18n("Camera position X:"), AlignLeft | AlignVCenter);
 d->mCameraPosY->setLabel(i18n("Camera position Y:"), AlignLeft | AlignVCenter);
 d->mCameraPosZ->setLabel(i18n("Camera position Z:"), AlignLeft | AlignVCenter);
 d->mRotateX->setLabel(i18n("Rotation arounx X-axis:"), AlignLeft | AlignVCenter);
 d->mRotateY->setLabel(i18n("Rotation arounx Y-axis:"), AlignLeft | AlignVCenter);
 d->mRotateZ->setLabel(i18n("Rotation arounx Z-axis:"), AlignLeft | AlignVCenter);

 d->mCameraPosX->setRange(MIN_CAMERAPOS_X, MAX_CAMERAPOS_X, 0.1f, true);
 d->mCameraPosY->setRange(MIN_CAMERAPOS_Y, MAX_CAMERAPOS_Y, 0.1f, true);
 d->mCameraPosZ->setRange(-50.0f, 50.0f, 0.1f, true);
 d->mRotateX->setRange(-180.0f, 180.0f, 1.0f, true);
 d->mRotateY->setRange(-180.0f, 180.0f, 1.0f, true);
 d->mRotateZ->setRange(-180.0f, 180.0f, 1.0f, true);

 connect(d->mCameraPosX, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotCameraChanged()));
 connect(d->mCameraPosY, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotCameraChanged()));
 connect(d->mCameraPosZ, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotCameraChanged()));
 connect(d->mRotateX, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotCameraChanged()));
 connect(d->mRotateY, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotCameraChanged()));
 connect(d->mRotateZ, SIGNAL(signalValueChanged(float)),
		this, SLOT(slotCameraChanged()));

 // translating first is more logical from a user point of view, but sometimes
 // rotating first is important from a technical point of view (gluLookAt()
 // rotates first)
 d->mTranslateFirst = new QCheckBox(i18n("Translate first"), this);
 QToolTip::add(d->mTranslateFirst, i18n("Here you can define whether the view is first translated and then rotated, or the other way round. If that doesn't mean anything to you - leave it at the default."));
 d->mTranslateFirst->setChecked(true);
 connect(d->mTranslateFirst, SIGNAL(toggled(bool)),
		this, SLOT(slotTranslateFirstChanged()));
 layout->addWidget(d->mTranslateFirst);

 d->mShowMatrices = new QCheckBox(i18n("Show matrices"), this);
 d->mShowMatrices->setChecked(false);
 connect(d->mShowMatrices, SIGNAL(toggled(bool)),
		this, SLOT(slotShowMatricesChanged(bool)));
 layout->addWidget(d->mShowMatrices);

 d->mMatrices = new QVBox(this);
 layout->addWidget(d->mMatrices);

 QVGroupBox* box;
 box = new QVGroupBox(i18n("Generated matrix"), d->mMatrices);
 d->mMatrix = new BoMatrixWidget(box);
 QToolTip::add(d->mMatrix, i18n("This is the matrix that should be created for the current rotation and cameraPos settings.\nThis matrix id <em>directly</em> calculated from the angle values you have entered."));

 box = new QVGroupBox(i18n("Camera matrix"), d->mMatrices);
 d->mCameraMatrix = new BoMatrixWidget(box);
 QToolTip::add(d->mCameraMatrix, i18n("This is the matrix is generated by a the camera.\nIf the code has no bugs, this should be equal to the matrices above and below.\nOpenGL commands are used to generate this matrix, so it is absolutely depedable."));

 box = new QVGroupBox(i18n("gluLookAt"), d->mMatrices);
 d->mEulerToGluLookAtMatrix = new BoMatrixWidget(box);
 QToolTip::add(d->mEulerToGluLookAtMatrix, i18n("This matrix converts the cameraPos vector and the rotation angles back to gluLookAt() values.\nIt is important, that this matrix is correct, in order to ensure a correct camera."));

 slotShowMatricesChanged(d->mShowMatrices->isChecked());
}

BoPlainCameraWidget::~BoPlainCameraWidget()
{
 delete d;
}

int BoPlainCameraWidget::needCameraType() const
{
 return BoCamera::Camera;
}

void BoPlainCameraWidget::slotTranslateFirstChanged()
{
 updateFromCamera();
}

void BoPlainCameraWidget::slotShowMatricesChanged(bool show)
{
 if (show) {
	d->mMatrices->show();
 } else {
	d->mMatrices->hide();
 }
}

void BoPlainCameraWidget::updateFromCamera()
{
 BO_CHECK_NULL_RET(camera());

 BoMatrix rotationMatrix = camera()->rotationMatrix();
 BoVector3 cameraPos = camera()->cameraPos();

 if (translateFirst()) {
	BoVector3 tmp = cameraPos;
	rotationMatrix.transform(&cameraPos, &tmp);
 }
 d->mCameraPosX->setValue(cameraPos.x(), false);
 d->mCameraPosY->setValue(cameraPos.y(), false);
 d->mCameraPosZ->setValue(cameraPos.z(), false);

 float alpha, beta, gamma;
 rotationMatrix.toRotation(&alpha, &beta, &gamma);

 d->mRotateX->setValue(alpha, false);
 d->mRotateY->setValue(beta, false);
 d->mRotateZ->setValue(gamma, false);

 updateMatrixWidget();
}


void BoPlainCameraWidget::slotCameraChanged()
{
 BO_CHECK_NULL_RET(camera());

 BoVector3 lookAt;
 BoVector3 up;

 BoVector3 cameraPos(d->mCameraPosX->value(), d->mCameraPosY->value(), d->mCameraPosZ->value());
 BoMatrix M;
 BoMatrix rotationMatrix;
 rotationMatrix.rotate(d->mRotateX->value(), 1.0f, 0.0f, 0.0f);
 rotationMatrix.rotate(d->mRotateY->value(), 0.0f, 1.0f, 0.0f);
 rotationMatrix.rotate(d->mRotateZ->value(), 0.0f, 0.0f, 1.0f);

 if (translateFirst()) {
	M.translate(-cameraPos.x(), -cameraPos.y(), -cameraPos.z());
 }
 M.multiply(&rotationMatrix);
 if (!translateFirst()) {
	M.translate(-cameraPos.x(), -cameraPos.y(), -cameraPos.z());
 } else {
	// gluLookAt() (and therefore BoCamera) expects the translation after
	// the rotation. we need to transform the vector for further use.
	BoMatrix inv;
	if (!rotationMatrix.invert(&inv)) {
		boError() << k_funcinfo << "cannot invert rotation matrix" << endl;
		// all values would be broken if we would go on. return from
		// here, in order to avoid applying this to the camera.
		// but we will never reach this point.
		return;
	}
	BoVector3 tmp = cameraPos;
	inv.transform(&cameraPos, &tmp);
 }
 M.toGluLookAt(&lookAt, &up, cameraPos);

 camera()->setGluLookAt(cameraPos, lookAt, up);

 emitSignalCameraChanged();
}

bool BoPlainCameraWidget::translateFirst() const
{
 return d->mTranslateFirst->isChecked();
}

void BoPlainCameraWidget::updateMatrixWidget()
{
 BoVector3 cameraPos = BoVector3(d->mCameraPosX->value(), d->mCameraPosY->value(), d->mCameraPosZ->value());
 BoMatrix rotationMatrix;
 rotationMatrix.rotate(d->mRotateX->value(), 1.0f, 0.0f, 0.0f);
 rotationMatrix.rotate(d->mRotateY->value(), 0.0f, 1.0f, 0.0f);
 rotationMatrix.rotate(d->mRotateZ->value(), 0.0f, 0.0f, 1.0f);


 BoMatrix M;
 if (translateFirst()) {
	M.translate(-cameraPos.x(), -cameraPos.y(), -cameraPos.z());
 }
 M.multiply(&rotationMatrix);
 if (!translateFirst()) {
	M.translate(-cameraPos.x(), -cameraPos.y(), -cameraPos.z());
 }

 // this is the matrix that should get calculated by opengl. The formula above
 // is exactly what is generated by 3 glRotatef() calls with the specified
 // angles.
 d->mMatrix->setMatrix(&M);

 // for better comparison we also display the actual camera matrix.
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 camera()->applyCameraToScene();
 BoMatrix cameraMatrix(GL_MODELVIEW_MATRIX);
 glPopMatrix();
 d->mCameraMatrix->setMatrix(&cameraMatrix);
 d->mCameraMatrix->compareMatrices(d->mMatrix);



 // convert the 3 angles ("euler angles") and the cameraPos vector to gluLookAt
 // values.
 BoMatrix plainMatrix = M;
 M.loadIdentity();

 if (translateFirst()) {
	BoVector3 tmp = cameraPos;
	rotationMatrix.transform(&cameraPos, &tmp);
 }

 BoVector3 lookAt;
 BoVector3 up;
 plainMatrix.toGluLookAt(&lookAt, &up, cameraPos);

 M.setLookAtRotation(cameraPos, lookAt, up);
 M.translate(-cameraPos.x(), -cameraPos.y(), -cameraPos.z());

 d->mEulerToGluLookAtMatrix->setMatrix(&M);
 d->mEulerToGluLookAtMatrix->compareMatrices(d->mCameraMatrix);
}

BoVector3 matrixUntranslate(const BoMatrix& rotationMatrix)
{
 /*
  * A rotation matrix looks like this:
  * m0 m4 m7  0
  * m1 m5 m9  0
  * m2 m6 m10 0
  * 0  0  0   1
  * where the m's are the components that depend on the rotation
  *
  * When a matrix is translated, it will look like this:
  * a0 a4 a8  t0
  * a1 a5 a9  t1
  * a2 a6 a10 t2
  * a3 a7 a11 t3
  * where t0,..,t3 are the results from the translation. t0 to t3 depend on a0
  * to a11 and the values of t0 to t3 before the translation (let's name them
  * t0',..,t3')
  *
  * When a rotation matrix is translated, then a3,a7,a11,t0',t1',t2' are 0 and
  * t3' is 1.
  * t0,...t3 are calculated using these formulas then:
  *
  * t0 = a0*x + a4*y + a8*z
  * t1 = a1*x + a5*y + a8*z
  * t2 = a2*x + a6*y + a10*z
  * t3 = 1
  *
  * all we have is a linear equation system with 3 variables. what we want is
  * x,y,z which is the translation vector.
  */
 if (rotationMatrix[15] != 1.0f) {
	boError() << k_funcinfo << "not a valid matrix!" << endl;
	return BoVector3();
 }
 const BoMatrix& m = rotationMatrix;
 // the following is simply the solution to the linear equation system above. i
 // did not try to optimize it.
 const float t0 = m[12];
 const float t1 = m[13];
 const float t2 = m[14];
 const float m5m0 = m[5] * m[0];
 const float m4m1 = m[4] * m[1];
 const float m6m0 = m[6] * m[0];
 const float m4m2 = m[4] * m[2];

 const float m5m0_m4m1 = m5m0 - m4m1;
 const float m6m0_m4m2 = m6m0 - m4m2;
 const float m10m0_m8m2 = m[10] * m[0] - m[8] * m[2];
 const float m9m0_m8m1 = m[9] * m[0] - m[8] * m[1];

 const float t2m0_t0m2 = t2 * m[0] - t0 * m[2];
 const float t1m0_t0m1 = t1 * m[0] - t0 * m[1];

 // urks.
 float z = (t2m0_t0m2 * m5m0_m4m1 - t1m0_t0m1 * m6m0_m4m2) / (m10m0_m8m2 * m5m0_m4m1 - m6m0_m4m2 * m9m0_m8m1);
 float y = (t1m0_t0m1 - m9m0_m8m1 * z) / m5m0_m4m1;
 float x = (t0 - m[8] * z - m[4] * y) / m[0];

 return BoVector3(x, y, z);
}

void extractUp(BoVector3& up, const BoVector3& x, const BoVector3& z)
{
// keep these formulas in mind:
// (you can get them from x := up cross z , (we assume that no normalizing necessary!)
// up[2] = (x[1] + (x[0] * z[0]) / z[1] + (x[2] * z[2] / z[1])) / (z[0] + z[1]);
// up[1] = (x[0] + up[2] * z[1]) / z[2];
// up[0] = ((x[0] + up[2] * z[1] * z[0]) / z[2] + x[2]) / z[1];

 // AB: note that every component of z can actually become zero. i tested all three.
 if (fabsf(z[1]) <= 0.0001f) {
	// we can use
	// x[0] := up[1] * z[2] - up[2] * z[1] => x[0] = up[1] * z[2]
	// ==> up[1] = x[0] / z[2]
	// or
	// x[2] := up[0] * z[1] - up[1] * z[0] => x[2] = -up[1] * z[0]
	// ==> up[1] = x[2] / z[0]
	if (fabsf(z[0]) <= 0.0001f && fabsf(z[2]) <= 0.0001f) {
		// is this possible? where to fall back to?
		boError() << "oops - x[0] != 0, x[2] != 0, z[0] == z[2] == 0  !" << endl;
		up.setY(0.0f);
	} else if (fabs(z[2]) > 0.0001f) {
		up.setY(x[0] / z[2]);
	} else { // fabs(z[0]) > 0.0001
		up.setY(-x[2] / z[0]);
	}

	// only one equation for two variables left :-(
	// x[1] := up[2] * z[0] - up[0] * z[2];
	if (fabsf(z[2]) <= 0.0001f && fabs(z[0]) <= 0.0001f) {
		// all of z are zero. this is probably invalid anyway.
		// AB: to be proven.
		up.setX(0.0f);
		up.setX(0.0f);
	} else if (fabsf(z[2]) <= 0.0001f) {
		up.setZ(x[1] / z[0]);
		// up[0] doesn't influence the matrix anyway
		up.setX(0.0f);
	} else if (fabsf(z[0]) <= 0.0001f) {
		up.setX(-x[1] / z[2]);
		// up[2] doesn't influence the matrix anyway
		up.setZ(0.0f);
	} else {
		// multiple solutions possible.
		up.setX(0.0f);
		up.setZ(x[1] / z[0]);
	}
	return;
 } else if (fabsf(z[2]) <= 0.0001f) {
	// here we can assume that z[1] != 0, as we already checked above

	// we can use
	// x[0] := up[1] * z[2] - up[2] * z[1] => x[0] = -up[2] * z[1]
	// ==> up[2] = -x[0] / z[1]
	// or
	// x[1] := up[2] * z[0] - up[0] * z[2] => x[1] = up[2] * z[0]
	// ==> up[2] = x[1] / z[0]

	// we use the first, as we already know that z[1] != 0
	up.setZ(-x[0] / z[1]);

	// one equation left:
	// x[2] := up[0] * z[1] - up[1] * z[0]
	if (fabsf(z[0]) <= 0.0001f) {
		up.setX(x[2] / z[1]);
		// up[1] does't influence the matrix anyway
		up.setY(0.0f);
	} else {
		// multiple solutions possible
		up.setY(0.0f);
		up.setX(x[2] / z[1]);
	}
	return;
 } else if (fabs(z[0]) <= 0.0001f) {
	// here we can assume that z[1] != 0 and z[2] != 0

	// we can use
	// x[1] := up[2] * z[0] - up[0] * z[2] => x[1] = -up[0] * z[2]
	// ==> up[0] = -x[1] / z[2]
	// or
	// x[2] := up[0] * z[1] - up[1] * z[0] => x[2] = up[0] * z[1]
	// ==> up[0] = x[2] / z[1]

	up.setX(x[2] / z[1]);

	// one equation left:
	// x[0] := up[1] * z[2] - up[2] * z[1]
	// multiple solutions possible, as z[1] and z[2] are both != 0

	up.setZ(0.0f);
	up.setY(x[0] / z[2]);
	return;
 }

 double foo1;
 double foo2;
 double foo3;

 foo3 = 0;

 // this code depends on z[0] != 0, z[1] != 0 and z[2] != 0 !
 foo1 = (x[2] * z[2]) / (2 * z[1] * z[0]);
 foo2 = x[0] / (2 * z[1]);

 up.setZ(foo1 - foo2);

 foo1 = x[0] + up.z() * z[1];
 up.setY(foo1 / z[2]);

 up.setX((up.y() * z[0] + x[2]) / z[1]);

}









class BoOrbiterCameraWidgetPrivate
{
public:
	BoOrbiterCameraWidgetPrivate()
	{
		mOrbiter = 0;
	}
	BoOrbiterWidget* mOrbiter;
};

BoOrbiterCameraWidget::BoOrbiterCameraWidget(QWidget* parent, const char* name)
	: BoCameraConfigWidgetBase(parent, name)
{
 d = new BoOrbiterCameraWidgetPrivate;

 QVBoxLayout* layout = new QVBoxLayout(this);
 d->mOrbiter = new BoOrbiterWidget(this);
 layout->addWidget(d->mOrbiter, 1);
}

BoOrbiterCameraWidget::~BoOrbiterCameraWidget()
{
 delete d;
}

int BoOrbiterCameraWidget::needCameraType() const
{
 return BoCamera::Camera;
}

void BoOrbiterCameraWidget::updateFromCamera()
{
 BO_CHECK_NULL_RET(camera());
 d->mOrbiter->slotUpdateGL();

// BoMatrix rotationMatrix = camera()->rotationMatrix();
// BoVector3 cameraPos = camera()->cameraPos();

 updateMatrixWidget();
}

void BoOrbiterCameraWidget::updateMatrixWidget()
{
}


void BoOrbiterCameraWidget::slotCameraChanged()
{
 BO_CHECK_NULL_RET(camera());

 emitSignalCameraChanged();
}

void BoOrbiterCameraWidget::setCamera(BoCamera* camera)
{
 BoCameraConfigWidgetBase::setCamera(camera);
 d->mOrbiter->setCamera(camera);
}
