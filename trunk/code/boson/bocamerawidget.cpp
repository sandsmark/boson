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
#include "bosonglwidget.h" // BoContext
#include "bolight.h"

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


class BoPUICameraWidgetPrivate
{
public:
	BoPUICameraWidgetPrivate()
	{
		mTab = 0;

		mCamera = 0;
	}
	BoPUITabWidget* mTab;
	QPtrList<BoPUICameraConfigWidgetBase> mConfigWidgets;
	QMap<BoPUICameraConfigWidgetBase*, QString> mConfigWidget2Name;

	BoCamera* mCamera;
};

BoPUICameraWidget::BoPUICameraWidget(QObject* parent, const char* name) : BoPUIWidget(parent, name)
{
 d = new BoPUICameraWidgetPrivate;
 BoPUIVLayout* topLayout = new BoPUIVLayout(0);
 mLayout->addLayout(topLayout);

 d->mTab = new BoPUITabWidget(this, "cameratabwidget");
 topLayout->addWidget(d->mTab);

 BoPUIGameCameraWidget* bosonCamera = new BoPUIGameCameraWidget(d->mTab, "gamecamera");
 BoPUIGLUCameraWidget* gluLookAtCamera = new BoPUIGLUCameraWidget(d->mTab, "glulookatcamera");
 BoPUIPlainCameraWidget* plainCamera = new BoPUIPlainCameraWidget(d->mTab, "plaincamera");
 BoPUIOrbiterCameraWidget* orbiterCamera = new BoPUIOrbiterCameraWidget(d->mTab, "orbitercamera");

 addConfigWidget(i18n("Orbiter camera"), orbiterCamera);
 addConfigWidget(i18n("Game camera"), bosonCamera);
 addConfigWidget(i18n("translate/rotate camera"), plainCamera);
 addConfigWidget(i18n("gluLookAt camera"), gluLookAtCamera);
}

BoPUICameraWidget::~BoPUICameraWidget()
{
 d->mConfigWidgets.clear();
 delete d;
}

void BoPUICameraWidget::addConfigWidget(const QString& name, BoPUICameraConfigWidgetBase* c)
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

void BoPUICameraWidget::slotUpdateFromCamera()
{
 QPtrListIterator<BoPUICameraConfigWidgetBase> it(d->mConfigWidgets);
 for (; it.current(); ++it) {
	if (it.current()->camera()) {
		if (!it.current()->updatesBlocked()) {
			it.current()->updateFromCamera();
		}
	}
 }
}

void BoPUICameraWidget::setCamera(BoCamera* camera)
{
 d->mCamera = camera;
 int cameraType = -1;
 if (d->mCamera) {
	cameraType = d->mCamera->cameraType();
 }
 QPtrListIterator<BoPUICameraConfigWidgetBase> it(d->mConfigWidgets);
 for (; it.current(); ++it) {
	d->mTab->removeTab(it.current());
	it.current()->hide();
	if (it.current()->needCameraType() != BoCamera::Camera) {
		if (it.current()->needCameraType() != cameraType) {
			it.current()->setCamera(0);
			continue;
		}
	}
	d->mTab->addTab(it.current(), d->mConfigWidget2Name[it.current()]);
	it.current()->setCamera(d->mCamera);
 }
 slotUpdateFromCamera();
}


BoPUICameraConfigWidgetBase::BoPUICameraConfigWidgetBase(QObject* parent, const char* name)
	: BoPUIWidget(parent, name)
{
 mCamera = 0;

 mUpdatesBlocked = false;
}


class BoPUIGLUCameraWidgetPrivate
{
public:
	BoPUIGLUCameraWidgetPrivate()
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
	BoPUINumInput* mLookAtX;
	BoPUINumInput* mLookAtY;
	BoPUINumInput* mLookAtZ;
	BoPUINumInput* mCameraPosX;
	BoPUINumInput* mCameraPosY;
	BoPUINumInput* mCameraPosZ;
	BoPUINumInput* mUpX;
	BoPUINumInput* mUpY;
	BoPUINumInput* mUpZ;

	BoPUILabel* mOrientation;
};

BoPUIGLUCameraWidget::BoPUIGLUCameraWidget(QObject* parent, const char* name)
	: BoPUICameraConfigWidgetBase(parent, name)
{
 d = new BoPUIGLUCameraWidgetPrivate;

 BoPUIVLayout* layout = new BoPUIVLayout(0);
 mLayout->addLayout(layout);

 d->mLookAtX = new BoPUINumInput(this);
 d->mLookAtY = new BoPUINumInput(this);
 d->mLookAtZ = new BoPUINumInput(this);
 d->mCameraPosX = new BoPUINumInput(this);
 d->mCameraPosY = new BoPUINumInput(this);
 d->mCameraPosZ = new BoPUINumInput(this);
 d->mUpX = new BoPUINumInput(this);
 d->mUpY = new BoPUINumInput(this);
 d->mUpZ = new BoPUINumInput(this);

 BoPUILabel* orientationLabel = new BoPUILabel(i18n("Orientation: "), this);
 d->mOrientation = new BoPUILabel(this);

 layout->addWidget(d->mLookAtX);
 layout->addWidget(d->mLookAtY);
 layout->addWidget(d->mLookAtZ);
 layout->addWidget(d->mCameraPosX);
 layout->addWidget(d->mCameraPosY);
 layout->addWidget(d->mCameraPosZ);
 layout->addWidget(d->mUpX);
 layout->addWidget(d->mUpY);
 layout->addWidget(d->mUpZ);

 BoPUIHLayout* orientationLayout = new BoPUIHLayout(0);
 orientationLayout->addWidget(orientationLabel);
 orientationLayout->addWidget(d->mOrientation);
 layout->addLayout(orientationLayout);

 d->mLookAtX->setLabel(i18n("Look at X:"), AlignLeft | AlignVCenter);
 d->mLookAtY->setLabel(i18n("Look at Y:"), AlignLeft | AlignVCenter);
 d->mLookAtZ->setLabel(i18n("Look at Z:"), AlignLeft | AlignVCenter);
 d->mCameraPosX->setLabel(i18n("Camera position X:"), AlignLeft | AlignVCenter);
 d->mCameraPosY->setLabel(i18n("Camera position Y:"), AlignLeft | AlignVCenter);
 d->mCameraPosZ->setLabel(i18n("Camera position Z:"), AlignLeft | AlignVCenter);
 d->mUpX->setLabel(i18n("Up vector X:"), AlignLeft | AlignVCenter);
 d->mUpY->setLabel(i18n("Up vector Y:"), AlignLeft | AlignVCenter);
 d->mUpZ->setLabel(i18n("Up vector Z:"), AlignLeft | AlignVCenter);

 d->mLookAtX->setRange(MIN_LOOKAT_X, MAX_LOOKAT_X);
 d->mLookAtY->setRange(MIN_LOOKAT_Y, MAX_LOOKAT_Y);
 d->mLookAtZ->setRange(MIN_LOOKAT_Z, MAX_LOOKAT_Z);
 d->mLookAtX->setStepSize(0.1f);
 d->mLookAtY->setStepSize(0.1f);
 d->mLookAtZ->setStepSize(0.1f);
 d->mCameraPosX->setRange(MIN_CAMERAPOS_X, MAX_CAMERAPOS_X);
 d->mCameraPosY->setRange(MIN_CAMERAPOS_Y, MAX_CAMERAPOS_Y);
 d->mCameraPosZ->setRange(-50.0f, 50.0f);
 d->mCameraPosX->setStepSize(0.1f);
 d->mCameraPosY->setStepSize(0.1f);
 d->mCameraPosZ->setStepSize(0.1f);
 d->mUpX->setRange(MIN_LOOKAT_X, MAX_LOOKAT_X);
 d->mUpY->setRange(MIN_LOOKAT_Y, MAX_LOOKAT_Y);
 d->mUpZ->setRange(MIN_LOOKAT_Z, MAX_LOOKAT_Z);
 d->mUpX->setStepSize(0.1f);
 d->mUpY->setStepSize(0.1f);
 d->mUpZ->setStepSize(0.1f);

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

BoPUIGLUCameraWidget::~BoPUIGLUCameraWidget()
{
 delete d;
}

int BoPUIGLUCameraWidget::needCameraType() const
{
 return BoCamera::Camera;
}

void BoPUIGLUCameraWidget::updateFromCamera()
{
 BO_CHECK_NULL_RET(camera());
 d->mLookAtX->setValue(camera()->lookAt().x());
 d->mLookAtY->setValue(camera()->lookAt().y());
 d->mLookAtZ->setValue(camera()->lookAt().z());
 d->mCameraPosX->setValue(camera()->cameraPos().x());
 d->mCameraPosY->setValue(camera()->cameraPos().y());
 d->mCameraPosZ->setValue(camera()->cameraPos().z());
 d->mUpX->setValue(camera()->up().x());
 d->mUpY->setValue(camera()->up().y());
 d->mUpZ->setValue(camera()->up().z());

 updateMatrixWidget();
}

void BoPUIGLUCameraWidget::slotLookAtChanged()
{
 BO_CHECK_NULL_RET(camera());
 BoVector3 lookAt(d->mLookAtX->value(), d->mLookAtY->value(), d->mLookAtZ->value());
 camera()->setGluLookAt(camera()->cameraPos(), lookAt, camera()->up());
 emitSignalCameraChanged();
}

void BoPUIGLUCameraWidget::slotCameraPosChanged()
{
 BO_CHECK_NULL_RET(camera());
 BoVector3 cameraPos(d->mCameraPosX->value(), d->mCameraPosY->value(), d->mCameraPosZ->value());
 camera()->setGluLookAt(cameraPos, camera()->lookAt(), camera()->up());
 emitSignalCameraChanged();
}

void BoPUIGLUCameraWidget::slotUpChanged()
{
 BO_CHECK_NULL_RET(camera());
 BoVector3 up(d->mUpX->value(), d->mUpY->value(), d->mUpZ->value());
 camera()->setGluLookAt(camera()->cameraPos(), camera()->lookAt(), up);
 emitSignalCameraChanged();
}

void BoPUIGLUCameraWidget::updateMatrixWidget()
{
 BoVector3 cameraPos = BoVector3(d->mCameraPosX->value(), d->mCameraPosY->value(), d->mCameraPosZ->value());
 BoVector3 lookAt = BoVector3(d->mLookAtX->value(), d->mLookAtY->value(), d->mLookAtZ->value());
 BoVector3 orientation = cameraPos - lookAt;
 d->mOrientation->setText(i18n("%1").arg(orientation.debugString()));
}


class BoPUIPlainCameraWidgetPrivate
{
public:
	BoPUIPlainCameraWidgetPrivate()
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
	BoPUINumInput* mCameraPosX;
	BoPUINumInput* mCameraPosY;
	BoPUINumInput* mCameraPosZ;
	BoPUINumInput* mRotateX;
	BoPUINumInput* mRotateY;
	BoPUINumInput* mRotateZ;

	BoPUIWidget* mMatrices;
	BoPUIMatrixWidget* mMatrix;
	BoPUIMatrixWidget* mCameraMatrix;
	BoPUIMatrixWidget* mEulerToGluLookAtMatrix;

	BoPUICheckBox* mTranslateFirst;
	BoPUICheckBox* mShowMatrices;
};

BoPUIPlainCameraWidget::BoPUIPlainCameraWidget(QObject* parent, const char* name)
	: BoPUICameraConfigWidgetBase(parent, name)
{
 d = new BoPUIPlainCameraWidgetPrivate;

 BoPUIVLayout* layout = new BoPUIVLayout(0);
 mLayout->addLayout(layout);

 d->mCameraPosX = new BoPUINumInput(this);
 d->mCameraPosY = new BoPUINumInput(this);
 d->mCameraPosZ = new BoPUINumInput(this);
 d->mRotateX = new BoPUINumInput(this);
 d->mRotateY = new BoPUINumInput(this);
 d->mRotateZ = new BoPUINumInput(this);

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

 d->mCameraPosX->setRange(MIN_CAMERAPOS_X, MAX_CAMERAPOS_X);
 d->mCameraPosY->setRange(MIN_CAMERAPOS_Y, MAX_CAMERAPOS_Y);
 d->mCameraPosZ->setRange(-50.0f, 50.0f);
 d->mCameraPosX->setStepSize(0.1f);
 d->mCameraPosY->setStepSize(0.1f);
 d->mCameraPosZ->setStepSize(0.1f);
 d->mRotateX->setRange(-180.0f, 180.0f);
 d->mRotateY->setRange(-180.0f, 180.0f);
 d->mRotateZ->setRange(-180.0f, 180.0f);
 d->mRotateX->setStepSize(0.1f);
 d->mRotateY->setStepSize(0.1f);
 d->mRotateZ->setStepSize(0.1f);

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
 d->mTranslateFirst = new BoPUICheckBox(i18n("Translate first"), this);
// QToolTip::add(d->mTranslateFirst, i18n("Here you can define whether the view is first translated and then rotated, or the other way round. If that doesn't mean anything to you - leave it at the default."));
 d->mTranslateFirst->setChecked(true);
 connect(d->mTranslateFirst, SIGNAL(signalToggled(bool)),
		this, SLOT(slotTranslateFirstChanged()));
 layout->addWidget(d->mTranslateFirst);

 d->mShowMatrices = new BoPUICheckBox(i18n("Show matrices"), this);
 d->mShowMatrices->setChecked(false);
 connect(d->mShowMatrices, SIGNAL(signalToggled(bool)),
		this, SLOT(slotShowMatricesChanged(bool)));
 layout->addWidget(d->mShowMatrices);

 d->mMatrices = new BoPUIWidget(true, this);
 layout->addWidget(d->mMatrices);
 BoPUIWidget* box; // TODO: provide a groupbox here
 BoPUILabel* boxText;

// box = new QVGroupBox(i18n("Generated matrix"), d->mMatrices);
 box = new BoPUIWidget(true, this);
 boxText = new BoPUILabel(i18n("Generated matrix"), box);
 d->mMatrix = new BoPUIMatrixWidget(box);
 box->layout()->addWidget(boxText);
 box->layout()->addWidget(d->mMatrix);
 d->mMatrices->layout()->addWidget(box);
#warning TODO: tooltips for PUI
// QToolTip::add(d->mMatrix, i18n("This is the matrix that should be created for the current rotation and cameraPos settings.\nThis matrix id <em>directly</em> calculated from the angle values you have entered."));

// box = new QVGroupBox(i18n("Camera matrix"), d->mMatrices);
 box = new BoPUIWidget(true, this);
 boxText = new BoPUILabel(i18n("Camera matrix"), box);
 d->mCameraMatrix = new BoPUIMatrixWidget(box);
 box->layout()->addWidget(boxText);
 box->layout()->addWidget(d->mCameraMatrix);
 d->mMatrices->layout()->addWidget(box);
// QToolTip::add(d->mCameraMatrix, i18n("This is the matrix is generated by a the camera.\nIf the code has no bugs, this should be equal to the matrices above and below.\nOpenGL commands are used to generate this matrix, so it is absolutely depedable."));

// box = new QVGroupBox(i18n("gluLookAt"), d->mMatrices);
 box = new BoPUIWidget(true, this);
 boxText = new BoPUILabel(i18n("gluLookAt"), box);
 d->mEulerToGluLookAtMatrix = new BoPUIMatrixWidget(box);
 box->layout()->addWidget(boxText);
 box->layout()->addWidget(d->mEulerToGluLookAtMatrix);
 d->mMatrices->layout()->addWidget(box);
// QToolTip::add(d->mEulerToGluLookAtMatrix, i18n("This matrix converts the cameraPos vector and the rotation angles back to gluLookAt() values.\nIt is important, that this matrix is correct, in order to ensure a correct camera."));

 slotShowMatricesChanged(d->mShowMatrices->isChecked());
}

BoPUIPlainCameraWidget::~BoPUIPlainCameraWidget()
{
 delete d;
}

int BoPUIPlainCameraWidget::needCameraType() const
{
 return BoCamera::Camera;
}

void BoPUIPlainCameraWidget::slotTranslateFirstChanged()
{
 updateFromCamera();
}

void BoPUIPlainCameraWidget::slotShowMatricesChanged(bool show)
{
 if (show) {
	d->mMatrices->show();
 } else {
	d->mMatrices->hide();
 }
}

void BoPUIPlainCameraWidget::updateFromCamera()
{
 BO_CHECK_NULL_RET(camera());

 BoMatrix rotationMatrix = camera()->rotationMatrix();
 BoVector3 cameraPos = camera()->cameraPos();

 if (translateFirst()) {
	BoVector3 tmp = cameraPos;
	rotationMatrix.transform(&cameraPos, &tmp);
 }
 d->mCameraPosX->setValue(cameraPos.x());
 d->mCameraPosY->setValue(cameraPos.y());
 d->mCameraPosZ->setValue(cameraPos.z());

 float alpha, beta, gamma;
 rotationMatrix.toRotation(&alpha, &beta, &gamma);

 d->mRotateX->setValue(alpha);
 d->mRotateY->setValue(beta);
 d->mRotateZ->setValue(gamma);

 updateMatrixWidget();
}


void BoPUIPlainCameraWidget::slotCameraChanged()
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

bool BoPUIPlainCameraWidget::translateFirst() const
{
 return d->mTranslateFirst->isChecked();
}

void BoPUIPlainCameraWidget::updateMatrixWidget()
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



class BoPUIGameCameraWidgetPrivate
{
public:
	BoPUIGameCameraWidgetPrivate()
	{
		mLookAtX = 0;
		mLookAtY = 0;
		mLookAtZ = 0;
		mRotation = 0;
		mRadius = 0;

		mGameRestrictions = 0;
	}
	BoPUINumInput* mLookAtX;
	BoPUINumInput* mLookAtY;
	BoPUINumInput* mLookAtZ;
	BoPUINumInput* mRotation;
	BoPUINumInput* mRadius;

	// allow to use any values, without game restrictions, for inputs
	BoPUICheckBox* mGameRestrictions;
};

BoPUIGameCameraWidget::BoPUIGameCameraWidget(QObject* parent, const char* name)
	: BoPUICameraConfigWidgetBase(parent, name)
{
 d = new BoPUIGameCameraWidgetPrivate;

 BoPUIVLayout* layout = new BoPUIVLayout(0);
 mLayout->addLayout(layout);
 d->mLookAtX = new BoPUINumInput(this);
 d->mLookAtY = new BoPUINumInput(this);
 d->mLookAtZ = new BoPUINumInput(this);
 d->mRotation = new BoPUINumInput(this);
 d->mRadius = new BoPUINumInput(this);

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

 d->mGameRestrictions = new BoPUICheckBox(i18n("Use game restrictions"), this);
 d->mGameRestrictions->setChecked(true);
 connect(d->mGameRestrictions, SIGNAL(signalToggled(bool)),
		this, SLOT(slotToggleGameRestrictions()));
 layout->addWidget(d->mGameRestrictions);
 slotToggleGameRestrictions();

 connect(d->mLookAtX, SIGNAL(signalValueChanged(float)), this, SLOT(slotLookAtChanged()));
 connect(d->mLookAtY, SIGNAL(signalValueChanged(float)), this, SLOT(slotLookAtChanged()));
 connect(d->mLookAtZ, SIGNAL(signalValueChanged(float)), this, SLOT(slotLookAtChanged()));
 connect(d->mRotation, SIGNAL(signalValueChanged(float)), this, SLOT(slotRotationChanged()));
 connect(d->mRadius, SIGNAL(signalValueChanged(float)), this, SLOT(slotRadiusChanged()));

#warning fixme
 // FIXME: we probably need a puLargeInput or so here
 // -> we need line breaks
 BoPUILabel* text = new BoPUILabel(this);
 text->setText(i18n("Note: The game camera can not display all points of view. So if you change the camera somewhere else the values here will be out of sync!"));
// text->setAlignment(text->alignment() | WordBreak);
 layout->addWidget(text);
}

BoPUIGameCameraWidget::~BoPUIGameCameraWidget()
{
 delete d;
}

int BoPUIGameCameraWidget::needCameraType() const
{
 return BoCamera::GameCamera;
}

void BoPUIGameCameraWidget::updateFromCamera()
{
 BO_CHECK_NULL_RET(gameCamera());
 d->mLookAtX->setValue(gameCamera()->lookAt().x());
 d->mLookAtY->setValue(gameCamera()->lookAt().y());
 d->mLookAtZ->setValue(gameCamera()->lookAt().z());
 d->mRadius->setValue(gameCamera()->radius());
 d->mRotation->setValue(gameCamera()->rotation());
}

void BoPUIGameCameraWidget::slotLookAtChanged()
{
 BO_CHECK_NULL_RET(camera());
 BoVector3 lookAt(d->mLookAtX->value(), d->mLookAtY->value(), d->mLookAtZ->value());
 camera()->setLookAt(lookAt);
 emitSignalCameraChanged();
}

void BoPUIGameCameraWidget::slotRotationChanged()
{
 BO_CHECK_NULL_RET(gameCamera());
 gameCamera()->setRotation(d->mRotation->value());
 emitSignalCameraChanged();
}

void BoPUIGameCameraWidget::slotRadiusChanged()
{
 BO_CHECK_NULL_RET(camera());
 gameCamera()->setRadius(d->mRadius->value());
 emitSignalCameraChanged();
}

void BoPUIGameCameraWidget::slotToggleGameRestrictions()
{
 d->mRotation->setRange(-180.0f, 180.0f);
 d->mRotation->setStepSize(1.0f);
 if (!d->mGameRestrictions->isChecked()) {
	d->mLookAtX->setRange(MIN_LOOKAT_X, MAX_LOOKAT_X);
	d->mLookAtY->setRange(MIN_LOOKAT_Y, MAX_LOOKAT_Y);
	d->mLookAtZ->setRange(-50.0f, 50.0f);
	d->mRadius->setRange(-50.0f, 50.0f);
 } else {
	d->mLookAtX->setRange(MIN_LOOKAT_X, MAX_LOOKAT_X);
	d->mLookAtY->setRange(MIN_LOOKAT_Y, MAX_LOOKAT_Y);
	d->mLookAtZ->setRange(MIN_LOOKAT_Z, MAX_LOOKAT_Z);
	d->mRadius->setRange(MIN_LOOKAT_Z, MAX_LOOKAT_Z);
 }
 d->mLookAtX->setStepSize(0.1f);
 d->mLookAtY->setStepSize(0.1f);
 d->mLookAtZ->setStepSize(0.1f);
 d->mRadius->setStepSize(0.2f);
}



class BoPUIOrbiterCameraWidgetPrivate
{
public:
	BoPUIOrbiterCameraWidgetPrivate()
	{
		mOrbiter = 0;
	}
#warning TODO
	BoOrbiterWidget* mOrbiter;
};

BoPUIOrbiterCameraWidget::BoPUIOrbiterCameraWidget(QObject* parent, const char* name)
	: BoPUICameraConfigWidgetBase(parent, name)
{
 d = new BoPUIOrbiterCameraWidgetPrivate;

 BoPUIVLayout* layout = new BoPUIVLayout(0);
 mLayout->addLayout(layout);
// d->mOrbiter = new BoOrbiterWidget(this);
// connect(d->mOrbiter, SIGNAL(signalChanged(BoCamera*)), this, SLOT(slotCameraChanged()));
// layout->addWidget(d->mOrbiter, 1);
}

BoPUIOrbiterCameraWidget::~BoPUIOrbiterCameraWidget()
{
 delete d;
}

int BoPUIOrbiterCameraWidget::needCameraType() const
{
 return BoCamera::Camera;
}

void BoPUIOrbiterCameraWidget::updateFromCamera()
{
 BO_CHECK_NULL_RET(camera());

 if (d->mOrbiter) {
	d->mOrbiter->slotUpdateGL();
 }

// BoMatrix rotationMatrix = camera()->rotationMatrix();
// BoVector3 cameraPos = camera()->cameraPos();

 updateMatrixWidget();
}

void BoPUIOrbiterCameraWidget::updateMatrixWidget()
{
}

void BoPUIOrbiterCameraWidget::slotCameraChanged()
{
 BO_CHECK_NULL_RET(camera());

 emitSignalCameraChanged();
}

void BoPUIOrbiterCameraWidget::setCamera(BoCamera* camera)
{
 BoPUICameraConfigWidgetBase::setCamera(camera);
 if (d->mOrbiter) {
	d->mOrbiter->setCamera(camera);
 }
}


BoPUILightCameraWidget::BoPUILightCameraWidget(QObject* parent, bool showGlobalValues) : BoPUIWidget(parent)
{
 mCamera = 0;

 BoPUIVLayout* layout = new BoPUIVLayout(0);
 mLayout->addLayout(layout);

 mCameraWidget = new BoPUICameraWidget(this);
 layout->addWidget(mCameraWidget);

 mDirectional = new BoPUICheckBox(i18n("Directional light"), this);
 layout->addWidget(mDirectional);

// QHBox* hbox = 0;
// hbox = new QHBox(this);
// layout->addWidget(hbox);
// (void)new QLabel(i18n("Constant Attenuation"), hbox);
 mConstantAttenuation = new BoPUINumInput(this);
 mConstantAttenuation->setLabel(i18n("Constant Attenuation"));
 layout->addWidget(mConstantAttenuation);

// hbox = new QHBox(this);
// layout->addWidget(hbox);
// (void)new QLabel(i18n("Linear Attenuation"), hbox);
 mLinearAttenuation = new BoPUINumInput(this);
 mLinearAttenuation->setLabel(i18n("Linear Attenuation"));
 layout->addWidget(mLinearAttenuation);

// hbox = new QHBox(this);
// layout->addWidget(hbox);
// (void)new QLabel(i18n("Quadratic Attenuation"), hbox);
 mQuadraticAttenuation = new BoPUINumInput(this);
 mQuadraticAttenuation->setLabel(i18n("Quadratic Attenuation"));
 layout->addWidget(mQuadraticAttenuation);

// hbox = new QHBox(this);
// layout->addWidget(hbox);
// (void)new QLabel(i18n("Ambient:"), hbox);
 BoPUIHLayout* ambientLayout = new BoPUIHLayout(0);
 layout->addLayout(ambientLayout);
 BoPUILabel* ambientLabel = new BoPUILabel(i18n("Ambient:"), this);
 mAmbientR = new BoPUINumInput(this);
 mAmbientG = new BoPUINumInput(this);
 mAmbientB = new BoPUINumInput(this);
 mAmbientA = new BoPUINumInput(this);
 ambientLayout->addWidget(ambientLabel);
 ambientLayout->addWidget(mAmbientR);
 ambientLayout->addWidget(mAmbientG);
 ambientLayout->addWidget(mAmbientB);
 ambientLayout->addWidget(mAmbientA);

// hbox = new QHBox(this);
// layout->addWidget(hbox);
// (void)new QLabel(i18n("Diffuse:"), hbox);
 BoPUIHLayout* diffuseLayout = new BoPUIHLayout(0);
 layout->addLayout(diffuseLayout);
 BoPUILabel* diffuseLabel = new BoPUILabel(i18n("Diffuse:"), this);
 mDiffuseR = new BoPUINumInput(this);
 mDiffuseG = new BoPUINumInput(this);
 mDiffuseB = new BoPUINumInput(this);
 mDiffuseA = new BoPUINumInput(this);
 diffuseLayout->addWidget(diffuseLabel);
 diffuseLayout->addWidget(mDiffuseR);
 diffuseLayout->addWidget(mDiffuseG);
 diffuseLayout->addWidget(mDiffuseB);
 diffuseLayout->addWidget(mDiffuseA);

// hbox = new QHBox(this);
// layout->addWidget(hbox);
// (void)new QLabel(i18n("Specular:"), hbox);
 BoPUIHLayout* specularLayout = new BoPUIHLayout(0);
 layout->addLayout(specularLayout);
 BoPUILabel* specularLabel = new BoPUILabel(i18n("Specular:"), this);
 mSpecularR = new BoPUINumInput(this);
 mSpecularG = new BoPUINumInput(this);
 mSpecularB = new BoPUINumInput(this);
 mSpecularA = new BoPUINumInput(this);
 specularLayout->addWidget(specularLabel);
 specularLayout->addWidget(mSpecularR);
 specularLayout->addWidget(mSpecularG);
 specularLayout->addWidget(mSpecularB);
 specularLayout->addWidget(mSpecularA);

 mAmbientR->setRange(0.0f, 1.0f);
 mAmbientG->setRange(0.0f, 1.0f);
 mAmbientB->setRange(0.0f, 1.0f);
 mAmbientA->setRange(0.0f, 1.0f);
 mDiffuseR->setRange(0.0f, 1.0f);
 mDiffuseG->setRange(0.0f, 1.0f);
 mDiffuseB->setRange(0.0f, 1.0f);
 mDiffuseA->setRange(0.0f, 1.0f);
 mSpecularR->setRange(0.0f, 1.0f);
 mSpecularG->setRange(0.0f, 1.0f);
 mSpecularB->setRange(0.0f, 1.0f);
 mSpecularA->setRange(0.0f, 1.0f);
 mAmbientR->setStepSize(0.1f);
 mAmbientG->setStepSize(0.1f);
 mAmbientB->setStepSize(0.1f);
 mAmbientA->setStepSize(0.1f);
 mDiffuseR->setStepSize(0.1f);
 mDiffuseG->setStepSize(0.1f);
 mDiffuseB->setStepSize(0.1f);
 mDiffuseA->setStepSize(0.1f);
 mSpecularR->setStepSize(0.1f);
 mSpecularG->setStepSize(0.1f);
 mSpecularB->setStepSize(0.1f);
 mSpecularA->setStepSize(0.1f);

 connect(mDirectional, SIGNAL(signalToggled(bool)), this, SLOT(slotLightChanged()));
 connect(mConstantAttenuation, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightChanged()));
 connect(mLinearAttenuation, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightChanged()));
 connect(mQuadraticAttenuation, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightChanged()));
 connect(mAmbientR, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightChanged()));
 connect(mAmbientG, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightChanged()));
 connect(mAmbientB, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightChanged()));
 connect(mAmbientA, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightChanged()));
 connect(mDiffuseR, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightChanged()));
 connect(mDiffuseG, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightChanged()));
 connect(mDiffuseB, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightChanged()));
 connect(mDiffuseA, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightChanged()));
 connect(mSpecularR, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightChanged()));
 connect(mSpecularG, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightChanged()));
 connect(mSpecularB, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightChanged()));
 connect(mSpecularA, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightChanged()));

 mShowGlobalValues = showGlobalValues;
 mGlobalAmbientR = 0;
 mGlobalAmbientG = 0;
 mGlobalAmbientB = 0;
 mGlobalAmbientA = 0;
 if (showGlobalValues) {
//	hbox = new QHBox(this);
//	layout->addWidget(hbox);
//	(void)new QLabel(i18n("Global Ambient:"), hbox);
	BoPUIHLayout* globalAmbientLayout = new BoPUIHLayout(0);
	layout->addLayout(globalAmbientLayout);
	BoPUILabel* globalAmbientLabel = new BoPUILabel(i18n("Global Ambient:"), this);
	mGlobalAmbientR = new BoPUINumInput(this);
	mGlobalAmbientG = new BoPUINumInput(this);
	mGlobalAmbientB = new BoPUINumInput(this);
	mGlobalAmbientA = new BoPUINumInput(this);
	mGlobalAmbientR->setRange(0.0f, 1.0f);
	mGlobalAmbientG->setRange(0.0f, 1.0f);
	mGlobalAmbientB->setRange(0.0f, 1.0f);
	mGlobalAmbientA->setRange(0.0f, 1.0f);
	mGlobalAmbientR->setStepSize(0.1f);
	mGlobalAmbientG->setStepSize(0.1f);
	mGlobalAmbientB->setStepSize(0.1f);
	mGlobalAmbientA->setStepSize(0.1f);
	globalAmbientLayout->addWidget(globalAmbientLabel);
	globalAmbientLayout->addWidget(mGlobalAmbientR);
	globalAmbientLayout->addWidget(mGlobalAmbientG);
	globalAmbientLayout->addWidget(mGlobalAmbientB);
	globalAmbientLayout->addWidget(mGlobalAmbientA);
	connect(mGlobalAmbientR, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightModelChanged()));
	connect(mGlobalAmbientG, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightModelChanged()));
	connect(mGlobalAmbientB, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightModelChanged()));
	connect(mGlobalAmbientA, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightModelChanged()));
 }

 mBlockLightChanges = false;
}

BoPUILightCameraWidget::~BoPUILightCameraWidget()
{
 delete mCamera;
}

void BoPUILightCameraWidget::setLight(BoLight* light, BoContext* context)
{
 boDebug() << k_funcinfo << endl;
 if (mCamera) {
	mCameraWidget->setCamera(0);
 }
 delete mCamera;
 mCamera = new BoLightCamera(light, context);
 mCameraWidget->setCamera(mCamera);
 mLight = light;
 mContext = context;

 if (!mLight) {
	BO_NULL_ERROR(mLight);
	return;
 }
 mBlockLightChanges = true;
 mDirectional->setChecked(mLight->isDirectional());
 mConstantAttenuation->setValue(mLight->constantAttenuation());
 mLinearAttenuation->setValue(mLight->linearAttenuation());
 mQuadraticAttenuation->setValue(mLight->quadraticAttenuation());
 mAmbientR->setValue(mLight->ambient().x());
 mAmbientG->setValue(mLight->ambient().y());
 mAmbientB->setValue(mLight->ambient().z());
 mAmbientA->setValue(mLight->ambient().w());
 mDiffuseR->setValue(mLight->diffuse().x());
 mDiffuseG->setValue(mLight->diffuse().y());
 mDiffuseB->setValue(mLight->diffuse().z());
 mDiffuseA->setValue(mLight->diffuse().w());
 mSpecularR->setValue(mLight->specular().x());
 mSpecularG->setValue(mLight->specular().y());
 mSpecularB->setValue(mLight->specular().z());
 mSpecularA->setValue(mLight->specular().w());
 mConstantAttenuation->setEnabled(!mDirectional->isChecked());
 mLinearAttenuation->setEnabled(!mDirectional->isChecked());
 mQuadraticAttenuation->setEnabled(!mDirectional->isChecked());

 if (mShowGlobalValues) {
	GLfloat amb[4];
	glGetFloatv(GL_LIGHT_MODEL_AMBIENT, amb);
	mGlobalAmbientR->setValue(amb[0]);
	mGlobalAmbientG->setValue(amb[1]);
	mGlobalAmbientB->setValue(amb[2]);
	mGlobalAmbientA->setValue(amb[3]);
 }

 mBlockLightChanges = false;
}

void BoPUILightCameraWidget::slotLightChanged()
{
 if (!mLight) {
	return;
 }
 if (!mContext) {
	return;
 }
 if (mBlockLightChanges) {
	return;
 }
 boDebug() << k_funcinfo << endl;
 BoContext* old = BoContext::currentContext();
 mContext->makeCurrent();

 mLight->setDirectional(mDirectional->isChecked());
 mLight->setAmbient(BoVector4(mAmbientR->value(), mAmbientG->value(), mAmbientB->value(), mAmbientA->value()));
 mLight->setDiffuse(BoVector4(mDiffuseR->value(), mDiffuseG->value(), mDiffuseB->value(), mDiffuseA->value()));
 mLight->setSpecular(BoVector4(mSpecularR->value(), mSpecularG->value(), mSpecularB->value(), mSpecularA->value()));
 mLight->setConstantAttenuation(mConstantAttenuation->value());
 mLight->setLinearAttenuation(mLinearAttenuation->value());
 mLight->setQuadraticAttenuation(mQuadraticAttenuation->value());

 if (old) {
	old->makeCurrent();
 }

 // when directional light is enabled attenuation is per definition disabled
 mConstantAttenuation->setEnabled(!mDirectional->isChecked());
 mLinearAttenuation->setEnabled(!mDirectional->isChecked());
 mQuadraticAttenuation->setEnabled(!mDirectional->isChecked());
}

void BoPUILightCameraWidget::slotLightModelChanged()
{
 if (!mLight) {
	return;
 }
 if (!mContext) {
	return;
 }
 if (mBlockLightChanges) {
	return;
 }
 if (!mShowGlobalValues) {
	return;
 }

 boDebug() << k_funcinfo << endl;
 BoContext* old = BoContext::currentContext();
 mContext->makeCurrent();

 GLfloat amb[4];
 amb[0] = mGlobalAmbientR->value();
 amb[1] = mGlobalAmbientG->value();
 amb[2] = mGlobalAmbientB->value();
 amb[3] = mGlobalAmbientA->value();
 glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

 if (old) {
	old->makeCurrent();
 }
}

#include <plib/pu.h>
class BosonGLWidgetLight : public BosonGLWidget
{
public:
	BosonGLWidgetLight(QWidget* parent) : BosonGLWidget(parent)
	{
	}
	~BosonGLWidgetLight()
	{
	}

	void initialize()
	{
		initGL();
	}
	virtual void paintGL()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor3ub(255, 255, 255);
		puSetWindow(winId());
		puDisplay();
	}

public:
	BoPUIVLayout* mLayout;

protected:
	virtual void initializeGL()
	{
		if (isInitialized()) {
			return;
		}
		BO_CHECK_NULL_RET(context());
		static bool recursive = false;
		if (recursive) {
			return;
		}
		recursive = true;
		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
		glShadeModel(GL_FLAT);
		glDisable(GL_DITHER);
		glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
		glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		puSetWindow(winId());
		mLayout = new BoPUIVLayout(this);
		recursive = false;
	}
	virtual void resizeGL(int w, int h)
	{
		puSetWindow(winId());
		mLayout->doLayout();
		repaint(false);
	}

	virtual void mousePressEvent(QMouseEvent* e)
	{
		puSetWindow(winId());
		int puButton;
		switch (e->button()) {
			case QMouseEvent::LeftButton:
				puButton = PU_LEFT_BUTTON;
				break;
			case QMouseEvent::RightButton:
				puButton = PU_RIGHT_BUTTON;
				break;
			case QMouseEvent::MidButton:
				puButton = PU_MIDDLE_BUTTON;
				break;
			default:
				puButton = PU_NOBUTTON;
				break;
		}
		puMouse(puButton, PU_DOWN, e->x(), e->y());
		repaint(false);
	}
	virtual void mouseReleaseEvent(QMouseEvent* e)
	{
		puSetWindow(winId());
		int puButton;
		switch (e->button()) {
			case QMouseEvent::LeftButton:
				puButton = PU_LEFT_BUTTON;
				break;
			case QMouseEvent::RightButton:
				puButton = PU_RIGHT_BUTTON;
				break;
			case QMouseEvent::MidButton:
				puButton = PU_MIDDLE_BUTTON;
				break;
			default:
				puButton = PU_NOBUTTON;
				break;
		}
		puMouse(puButton, PU_UP, e->x(), e->y());
		repaint(false);
	}
	virtual void mouseMoveEvent(QMouseEvent* e)
	{
		puSetWindow(winId());
		puMouse(e->x(), e->y());
		repaint(false);
	}
	virtual void keyPressEvent(QKeyEvent* e)
	{
		puSetWindow(winId());
		BosonGLWidget::keyPressEvent(e);
		puKeyboard(e->ascii(), PU_DOWN);
		e->ignore();
		repaint(false);
	}
	virtual void keyReleaseEvent(QKeyEvent* e)
	{
		puSetWindow(winId());
		BosonGLWidget::keyPressEvent(e);
		puKeyboard(e->ascii(), PU_UP);
		e->ignore();
		repaint(false);
	}

};

BoLightCameraWidget1::BoLightCameraWidget1(QWidget* parent, bool showGlobalValues)
	: QWidget(parent)
{
 QVBoxLayout* l = new QVBoxLayout(this);
 mWidget = new BosonGLWidgetLight(this);
 l->addWidget(mWidget);
 mWidget->initialize();

 puSetWindow(mWidget->winId());
 mLightWidget = new BoPUILightCameraWidget(mWidget);
 mWidget->mLayout->addWidget(mLightWidget);
 repaint(false);
}

BoLightCameraWidget1::~BoLightCameraWidget1()
{
}

void BoLightCameraWidget1::setLight(BoLight* l, BoContext* c)
{
 mLightWidget->setLight(l, c);
}


