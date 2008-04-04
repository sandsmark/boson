/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bocamerawidget.h"
#include "bocamerawidget.moc"

#include "../bomemory/bodummymemory.h"
#include "bocamera.h"
#include "bodebug.h"
#include "bo3dtools.h"
#include "bonuminput.h"
#include "bomatrixwidget.h"
#include "boorbiterwidget.h"
#include "bosonufoglwidget.h"
#include "bolight.h"
#include <bogl.h>

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
BoVector3Float matrixUntranslate(const BoMatrix& rotationMatrix);

BoVector3Float matrixUntranslate(const BoMatrix& rotationMatrix)
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
	return BoVector3Float();
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

 return BoVector3Float(x, y, z);
}

void extractUp(BoVector3Float& up, const BoVector3Float& x, const BoVector3Float& z)
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


class BoUfoCameraWidgetPrivate
{
public:
	BoUfoCameraWidgetPrivate()
	{
		mTab = 0;

		mCamera = 0;
	}
	BoUfoTabWidget* mTab;
	QPtrList<BoUfoCameraConfigWidgetBase> mConfigWidgets;
	QMap<BoUfoCameraConfigWidgetBase*, QString> mConfigWidget2Name;

	BoCamera* mCamera;
};

BoUfoCameraWidget::BoUfoCameraWidget() : BoUfoWidget()
{
 d = new BoUfoCameraWidgetPrivate;
 BoUfoVBox* topLayoutWidget = new BoUfoVBox();
 addWidget(topLayoutWidget);

 d->mTab = new BoUfoTabWidget();
 topLayoutWidget->addWidget(d->mTab);

 BoUfoGLUCameraWidget* gluLookAtCamera = new BoUfoGLUCameraWidget();
 BoUfoGameCameraWidget* bosonCamera = new BoUfoGameCameraWidget();
 BoUfoPlainCameraWidget* plainCamera = new BoUfoPlainCameraWidget();
 BoUfoOrbiterCameraWidget* orbiterCamera = new BoUfoOrbiterCameraWidget();

 addConfigWidget(i18n("Orbiter camera"), orbiterCamera);
 addConfigWidget(i18n("Game camera"), bosonCamera);
 addConfigWidget(i18n("translate/rotate camera"), plainCamera);
 addConfigWidget(i18n("gluLookAt camera"), gluLookAtCamera);
}

BoUfoCameraWidget::~BoUfoCameraWidget()
{
 d->mConfigWidgets.clear();
 delete d;
}

void BoUfoCameraWidget::addConfigWidget(const QString& name, BoUfoCameraConfigWidgetBase* c)
{
 BO_CHECK_NULL_RET(c);
 d->mConfigWidgets.append(c);
 d->mConfigWidget2Name.insert(c, name);
 c->setCamera(d->mCamera);
 connect(c, SIGNAL(signalCameraChanged()),
		this, SLOT(slotUpdateFromCamera()));

 d->mTab->addTab(c, d->mConfigWidget2Name[c]);
 c->updateFromCamera();
}

void BoUfoCameraWidget::slotUpdateFromCamera()
{
 QPtrListIterator<BoUfoCameraConfigWidgetBase> it(d->mConfigWidgets);
 for (; it.current(); ++it) {
	if (it.current()->camera()) {
		if (!it.current()->updatesBlocked()) {
			it.current()->updateFromCamera();
		}
	}
 }
}

void BoUfoCameraWidget::setCamera(BoCamera* camera)
{
 d->mCamera = camera;
 int cameraType = -1;
 if (d->mCamera) {
	cameraType = d->mCamera->cameraType();
 }
 QPtrListIterator<BoUfoCameraConfigWidgetBase> it(d->mConfigWidgets);
 for (; it.current(); ++it) {
//	d->mTab->removeTab(it.current());
	it.current()->hide();
	if (it.current()->needCameraType() != BoCamera::Camera) {
		if (it.current()->needCameraType() != cameraType) {
			it.current()->setCamera(0);
			continue;
		}
	}
//	d->mTab->addTab(it.current(), d->mConfigWidget2Name[it.current()]);
	it.current()->setCamera(d->mCamera);
 }
 slotUpdateFromCamera();
}


BoUfoCameraConfigWidgetBase::BoUfoCameraConfigWidgetBase()
	: BoUfoWidget()
{
 mCamera = 0;

 mUpdatesBlocked = false;
}


class BoUfoGLUCameraWidgetPrivate
{
public:
	BoUfoGLUCameraWidgetPrivate()
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
	BoUfoNumInput* mLookAtX;
	BoUfoNumInput* mLookAtY;
	BoUfoNumInput* mLookAtZ;
	BoUfoNumInput* mCameraPosX;
	BoUfoNumInput* mCameraPosY;
	BoUfoNumInput* mCameraPosZ;
	BoUfoNumInput* mUpX;
	BoUfoNumInput* mUpY;
	BoUfoNumInput* mUpZ;

	BoUfoLabel* mOrientation;
};

BoUfoGLUCameraWidget::BoUfoGLUCameraWidget()
	: BoUfoCameraConfigWidgetBase()
{
 d = new BoUfoGLUCameraWidgetPrivate;
 BoUfoVBox* layoutWidget = new BoUfoVBox();
 addWidget(layoutWidget);

 d->mLookAtX = new BoUfoNumInput();
 d->mLookAtY = new BoUfoNumInput();
 d->mLookAtZ = new BoUfoNumInput();
 d->mCameraPosX = new BoUfoNumInput();
 d->mCameraPosY = new BoUfoNumInput();
 d->mCameraPosZ = new BoUfoNumInput();
 d->mUpX = new BoUfoNumInput();
 d->mUpY = new BoUfoNumInput();
 d->mUpZ = new BoUfoNumInput();

 layoutWidget->addWidget(d->mLookAtX);
 layoutWidget->addWidget(d->mLookAtY);
 layoutWidget->addWidget(d->mLookAtZ);
 layoutWidget->addWidget(d->mCameraPosX);
 layoutWidget->addWidget(d->mCameraPosY);
 layoutWidget->addWidget(d->mCameraPosZ);
 layoutWidget->addWidget(d->mUpX);
 layoutWidget->addWidget(d->mUpY);
 layoutWidget->addWidget(d->mUpZ);

 BoUfoWidget* orientationLayoutWidget = new BoUfoWidget();
 layoutWidget->addWidget(orientationLayoutWidget);
 BoUfoLabel* orientationLabel = new BoUfoLabel(i18n("Orientation: "));
 d->mOrientation = new BoUfoLabel();
 orientationLayoutWidget->addWidget(orientationLabel);
 orientationLayoutWidget->addWidget(d->mOrientation);

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

BoUfoGLUCameraWidget::~BoUfoGLUCameraWidget()
{
 delete d;
}

int BoUfoGLUCameraWidget::needCameraType() const
{
 return BoCamera::Camera;
}

void BoUfoGLUCameraWidget::updateFromCamera()
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

void BoUfoGLUCameraWidget::slotLookAtChanged()
{
 BO_CHECK_NULL_RET(camera());
 BoVector3Float lookAt(d->mLookAtX->value(), d->mLookAtY->value(), d->mLookAtZ->value());
 camera()->setGluLookAt(camera()->cameraPos(), lookAt, camera()->up());
 emitSignalCameraChanged();
}

void BoUfoGLUCameraWidget::slotCameraPosChanged()
{
 BO_CHECK_NULL_RET(camera());
 BoVector3Float cameraPos(d->mCameraPosX->value(), d->mCameraPosY->value(), d->mCameraPosZ->value());
 camera()->setGluLookAt(cameraPos, camera()->lookAt(), camera()->up());
 emitSignalCameraChanged();
}

void BoUfoGLUCameraWidget::slotUpChanged()
{
 BO_CHECK_NULL_RET(camera());
 BoVector3Float up(d->mUpX->value(), d->mUpY->value(), d->mUpZ->value());
 camera()->setGluLookAt(camera()->cameraPos(), camera()->lookAt(), up);
 emitSignalCameraChanged();
}

void BoUfoGLUCameraWidget::updateMatrixWidget()
{
 BoVector3Float cameraPos = BoVector3Float(d->mCameraPosX->value(), d->mCameraPosY->value(), d->mCameraPosZ->value());
 BoVector3Float lookAt = BoVector3Float(d->mLookAtX->value(), d->mLookAtY->value(), d->mLookAtZ->value());
 BoVector3Float orientation = cameraPos - lookAt;
 QString s = QString("(%1,%2,%3").arg(orientation[0]).arg(orientation[1]).arg(orientation[2]);
 d->mOrientation->setText(i18n("%1").arg(s));
}


class BoUfoPlainCameraWidgetPrivate
{
public:
	BoUfoPlainCameraWidgetPrivate()
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
	BoUfoNumInput* mCameraPosX;
	BoUfoNumInput* mCameraPosY;
	BoUfoNumInput* mCameraPosZ;
	BoUfoNumInput* mRotateX;
	BoUfoNumInput* mRotateY;
	BoUfoNumInput* mRotateZ;

	BoUfoVBox* mMatrices;
	BoUfoMatrixWidget* mMatrix;
	BoUfoMatrixWidget* mCameraMatrix;
	BoUfoMatrixWidget* mEulerToGluLookAtMatrix;

	BoUfoCheckBox* mTranslateFirst;
	BoUfoCheckBox* mShowMatrices;
};

BoUfoPlainCameraWidget::BoUfoPlainCameraWidget()
	: BoUfoCameraConfigWidgetBase()
{
 d = new BoUfoPlainCameraWidgetPrivate;

 BoUfoVBox* layoutWidget = new BoUfoVBox();
 addWidget(layoutWidget);

 d->mCameraPosX = new BoUfoNumInput();
 d->mCameraPosY = new BoUfoNumInput();
 d->mCameraPosZ = new BoUfoNumInput();
 d->mRotateX = new BoUfoNumInput();
 d->mRotateY = new BoUfoNumInput();
 d->mRotateZ = new BoUfoNumInput();

 // note: *first* rotate, then camerapos
 // --> gluLookAt() does this equally. translation to eye pos happens at the
 // end.
 layoutWidget->addWidget(d->mRotateX);
 layoutWidget->addWidget(d->mRotateY);
 layoutWidget->addWidget(d->mRotateZ);
 layoutWidget->addWidget(d->mCameraPosX);
 layoutWidget->addWidget(d->mCameraPosY);
 layoutWidget->addWidget(d->mCameraPosZ);

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
 d->mTranslateFirst = new BoUfoCheckBox(i18n("Translate first"));
// QToolTip::add(d->mTranslateFirst, i18n("Here you can define whether the view is first translated and then rotated, or the other way round. If that doesn't mean anything to you - leave it at the default."));
 d->mTranslateFirst->setChecked(true);
 connect(d->mTranslateFirst, SIGNAL(signalToggled(bool)),
		this, SLOT(slotTranslateFirstChanged()));
 layoutWidget->addWidget(d->mTranslateFirst);

 d->mShowMatrices = new BoUfoCheckBox(i18n("Show matrices"));
 d->mShowMatrices->setChecked(false);
 connect(d->mShowMatrices, SIGNAL(signalToggled(bool)),
		this, SLOT(slotShowMatricesChanged(bool)));
 layoutWidget->addWidget(d->mShowMatrices);


 d->mMatrices = new BoUfoVBox();
 layoutWidget->addWidget(d->mMatrices);
 BoUfoVBox* box; // TODO: provide a groupbox here
 BoUfoLabel* boxText;

// box = new QVGroupBox(i18n("Generated matrix"), d->mMatrices);
 box = new BoUfoVBox();
 boxText = new BoUfoLabel(i18n("Generated matrix"));
 d->mMatrix = new BoUfoMatrixWidget();
 box->addWidget(boxText);
 box->addWidget(d->mMatrix);
 d->mMatrices->addWidget(box);
#warning TODO: tooltips for Ufo
// QToolTip::add(d->mMatrix, i18n("This is the matrix that should be created for the current rotation and cameraPos settings.\nThis matrix id <em>directly</em> calculated from the angle values you have entered."));

// box = new QVGroupBox(i18n("Camera matrix"), d->mMatrices);
 box = new BoUfoVBox();
 boxText = new BoUfoLabel(i18n("Camera matrix"));
 d->mCameraMatrix = new BoUfoMatrixWidget();
 box->addWidget(boxText);
 box->addWidget(d->mCameraMatrix);
 d->mMatrices->addWidget(box);
// QToolTip::add(d->mCameraMatrix, i18n("This is the matrix is generated by a the camera.\nIf the code has no bugs, this should be equal to the matrices above and below.\nOpenGL commands are used to generate this matrix, so it is absolutely depedable."));

// box = new QVGroupBox(i18n("gluLookAt"), d->mMatrices);
 box = new BoUfoVBox();
 boxText = new BoUfoLabel(i18n("gluLookAt"));
 d->mEulerToGluLookAtMatrix = new BoUfoMatrixWidget();
 box->addWidget(boxText);
 box->addWidget(d->mEulerToGluLookAtMatrix);
 d->mMatrices->addWidget(box);
// QToolTip::add(d->mEulerToGluLookAtMatrix, i18n("This matrix converts the cameraPos vector and the rotation angles back to gluLookAt() values.\nIt is important, that this matrix is correct, in order to ensure a correct camera."));

 slotShowMatricesChanged(d->mShowMatrices->checked());
}

BoUfoPlainCameraWidget::~BoUfoPlainCameraWidget()
{
 delete d;
}

int BoUfoPlainCameraWidget::needCameraType() const
{
 return BoCamera::Camera;
}

void BoUfoPlainCameraWidget::slotTranslateFirstChanged()
{
 updateFromCamera();
}

void BoUfoPlainCameraWidget::slotShowMatricesChanged(bool show)
{
 if (show) {
	d->mMatrices->show();
 } else {
	d->mMatrices->hide();
 }
}

void BoUfoPlainCameraWidget::updateFromCamera()
{
 BO_CHECK_NULL_RET(camera());

 BoMatrix rotationMatrix = camera()->rotationMatrix();
 BoVector3Float cameraPos = camera()->cameraPos();

 if (translateFirst()) {
	BoVector3Float tmp = cameraPos;
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


void BoUfoPlainCameraWidget::slotCameraChanged()
{
 BO_CHECK_NULL_RET(camera());

 BoVector3Float lookAt;
 BoVector3Float up;

 BoVector3Float cameraPos(d->mCameraPosX->value(), d->mCameraPosY->value(), d->mCameraPosZ->value());
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
	BoVector3Float tmp = cameraPos;
	inv.transform(&cameraPos, &tmp);
 }
 M.toGluLookAt(&lookAt, &up, cameraPos);

 camera()->setGluLookAt(cameraPos, lookAt, up);

 emitSignalCameraChanged();
}

bool BoUfoPlainCameraWidget::translateFirst() const
{
 return d->mTranslateFirst->checked();
}

void BoUfoPlainCameraWidget::updateMatrixWidget()
{
 BoVector3Float cameraPos = BoVector3Float(d->mCameraPosX->value(), d->mCameraPosY->value(), d->mCameraPosZ->value());
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
 BoMatrix cameraMatrix = createMatrixFromOpenGL(GL_MODELVIEW_MATRIX);
 glPopMatrix();
 d->mCameraMatrix->setMatrix(&cameraMatrix);
 d->mCameraMatrix->compareMatrices(d->mMatrix);



 // convert the 3 angles ("euler angles") and the cameraPos vector to gluLookAt
 // values.
 BoMatrix plainMatrix = M;
 M.loadIdentity();

 if (translateFirst()) {
	BoVector3Float tmp = cameraPos;
	rotationMatrix.transform(&cameraPos, &tmp);
 }

 BoVector3Float lookAt;
 BoVector3Float up;
 plainMatrix.toGluLookAt(&lookAt, &up, cameraPos);

 M.setLookAtRotation(cameraPos, lookAt, up);
 M.translate(-cameraPos.x(), -cameraPos.y(), -cameraPos.z());

 d->mEulerToGluLookAtMatrix->setMatrix(&M);
 d->mEulerToGluLookAtMatrix->compareMatrices(d->mCameraMatrix);
}



class BoUfoGameCameraWidgetPrivate
{
public:
	BoUfoGameCameraWidgetPrivate()
	{
		mLookAtX = 0;
		mLookAtY = 0;
		mLookAtZ = 0;
		mRotation = 0;
		mXRotation = 0;
		mDistance = 0;

		mGameRestrictions = 0;
	}
	BoUfoNumInput* mLookAtX;
	BoUfoNumInput* mLookAtY;
	BoUfoNumInput* mLookAtZ;
	BoUfoNumInput* mRotation;
	BoUfoNumInput* mXRotation;
	BoUfoNumInput* mDistance;

	// allow to use any values, without game restrictions, for inputs
	BoUfoCheckBox* mGameRestrictions;
};

BoUfoGameCameraWidget::BoUfoGameCameraWidget()
	: BoUfoCameraConfigWidgetBase()
{
 d = new BoUfoGameCameraWidgetPrivate;

 BoUfoVBox* layoutWidget = new BoUfoVBox();
 addWidget(layoutWidget);
 d->mLookAtX = new BoUfoNumInput();
 d->mLookAtY = new BoUfoNumInput();
 d->mLookAtZ = new BoUfoNumInput();
 d->mRotation = new BoUfoNumInput();
 d->mXRotation = new BoUfoNumInput();
 d->mDistance = new BoUfoNumInput();

 layoutWidget->addWidget(d->mLookAtX);
 layoutWidget->addWidget(d->mLookAtY);
 layoutWidget->addWidget(d->mLookAtZ);
 layoutWidget->addWidget(d->mRotation);
 layoutWidget->addWidget(d->mXRotation);
 layoutWidget->addWidget(d->mDistance);

 d->mLookAtX->setLabel(i18n("Look at X:"), AlignLeft | AlignVCenter);
 d->mLookAtY->setLabel(i18n("Look at Y:"), AlignLeft | AlignVCenter);
 d->mLookAtZ->setLabel(i18n("Look at Z:"), AlignLeft | AlignVCenter);
 d->mRotation->setLabel(i18n("Rotation:"), AlignLeft | AlignVCenter);
 d->mXRotation->setLabel(i18n("XRotation:"), AlignLeft | AlignVCenter);
 d->mDistance->setLabel(i18n("Distance:"), AlignLeft | AlignVCenter);

 d->mGameRestrictions = new BoUfoCheckBox(i18n("Use game restrictions"), this);
 d->mGameRestrictions->setChecked(true);
 connect(d->mGameRestrictions, SIGNAL(signalToggled(bool)),
		this, SLOT(slotToggleGameRestrictions()));
 layoutWidget->addWidget(d->mGameRestrictions);
 slotToggleGameRestrictions();

 connect(d->mLookAtX, SIGNAL(signalValueChanged(float)), this, SLOT(slotLookAtChanged()));
 connect(d->mLookAtY, SIGNAL(signalValueChanged(float)), this, SLOT(slotLookAtChanged()));
 connect(d->mLookAtZ, SIGNAL(signalValueChanged(float)), this, SLOT(slotLookAtChanged()));
 connect(d->mRotation, SIGNAL(signalValueChanged(float)), this, SLOT(slotRotationChanged()));
 connect(d->mXRotation, SIGNAL(signalValueChanged(float)), this, SLOT(slotXRotationChanged()));
 connect(d->mDistance, SIGNAL(signalValueChanged(float)), this, SLOT(slotDistanceChanged()));

#warning fixme
 // FIXME: we probably need a puLargeInput or so here
 // -> we need line breaks
 BoUfoLabel* text = new BoUfoLabel();
 text->setText(i18n("Note: The game camera can not display all points of view. So if you change the camera somewhere else the values here will be out of sync!"));
// text->setAlignment(text->alignment() | WordBreak);
 layoutWidget->addWidget(text);
}

BoUfoGameCameraWidget::~BoUfoGameCameraWidget()
{
 delete d;
}

int BoUfoGameCameraWidget::needCameraType() const
{
 return BoCamera::GameCamera;
}

void BoUfoGameCameraWidget::updateFromCamera()
{
 BO_CHECK_NULL_RET(gameCamera());
 d->mLookAtX->setValue(gameCamera()->lookAt().x());
 d->mLookAtY->setValue(gameCamera()->lookAt().y());
 d->mLookAtZ->setValue(gameCamera()->lookAt().z());
 d->mRotation->setValue(gameCamera()->rotation());
 d->mXRotation->setValue(gameCamera()->xRotation());
 d->mDistance->setValue(gameCamera()->distance());
}

void BoUfoGameCameraWidget::slotLookAtChanged()
{
 BO_CHECK_NULL_RET(camera());
 BoVector3Float lookAt(d->mLookAtX->value(), d->mLookAtY->value(), d->mLookAtZ->value());
 camera()->setLookAt(lookAt);
 emitSignalCameraChanged();
}

void BoUfoGameCameraWidget::slotRotationChanged()
{
 BO_CHECK_NULL_RET(gameCamera());
 gameCamera()->setRotation(d->mRotation->value());
 emitSignalCameraChanged();
}

void BoUfoGameCameraWidget::slotXRotationChanged()
{
 BO_CHECK_NULL_RET(gameCamera());
 gameCamera()->setXRotation(d->mXRotation->value());
 emitSignalCameraChanged();
}

void BoUfoGameCameraWidget::slotDistanceChanged()
{
 BO_CHECK_NULL_RET(camera());
 gameCamera()->setDistance(d->mDistance->value());
 emitSignalCameraChanged();
}

void BoUfoGameCameraWidget::slotToggleGameRestrictions()
{
 d->mRotation->setRange(-180.0f, 180.0f);
 d->mRotation->setStepSize(1.0f);
 if (!d->mGameRestrictions->checked()) {
	d->mLookAtX->setRange(MIN_LOOKAT_X, MAX_LOOKAT_X);
	d->mLookAtY->setRange(MIN_LOOKAT_Y, MAX_LOOKAT_Y);
	d->mLookAtZ->setRange(-50.0f, 50.0f);
	d->mXRotation->setRange(-180.0f, 180.0f);
	d->mDistance->setRange(0.0f, 100.0f);
 } else {
	d->mLookAtX->setRange(MIN_LOOKAT_X, MAX_LOOKAT_X);
	d->mLookAtY->setRange(MIN_LOOKAT_Y, MAX_LOOKAT_Y);
	d->mLookAtZ->setRange(MIN_LOOKAT_Z, MAX_LOOKAT_Z);
	d->mXRotation->setRange(0.0f, 180.0f);
	d->mDistance->setRange(MIN_LOOKAT_Z, MAX_LOOKAT_Z);
 }
 d->mLookAtX->setStepSize(0.1f);
 d->mLookAtY->setStepSize(0.1f);
 d->mLookAtZ->setStepSize(0.1f);
 d->mXRotation->setStepSize(1.0f);
 d->mDistance->setStepSize(0.5f);
}



class BoUfoOrbiterCameraWidgetPrivate
{
public:
	BoUfoOrbiterCameraWidgetPrivate()
	{
		mOrbiter = 0;
	}
#warning TODO
	BoOrbiterWidget* mOrbiter;
};

BoUfoOrbiterCameraWidget::BoUfoOrbiterCameraWidget()
	: BoUfoCameraConfigWidgetBase()
{
 d = new BoUfoOrbiterCameraWidgetPrivate;

 BoUfoVBox* layoutWidget = new BoUfoVBox();
 addWidget(layoutWidget);
// d->mOrbiter = new BoOrbiterWidget(this);
// connect(d->mOrbiter, SIGNAL(signalChanged(BoCamera*)), this, SLOT(slotCameraChanged()));
// layoutWidget->addWidget(d->mOrbiter, 1);
}

BoUfoOrbiterCameraWidget::~BoUfoOrbiterCameraWidget()
{
 delete d;
}

int BoUfoOrbiterCameraWidget::needCameraType() const
{
 return BoCamera::Camera;
}

void BoUfoOrbiterCameraWidget::updateFromCamera()
{
 BO_CHECK_NULL_RET(camera());

 if (d->mOrbiter) {
	d->mOrbiter->slotUpdateGL();
 }

// BoMatrix rotationMatrix = camera()->rotationMatrix();
// BoVector3Float cameraPos = camera()->cameraPos();

 updateMatrixWidget();
}

void BoUfoOrbiterCameraWidget::updateMatrixWidget()
{
}

void BoUfoOrbiterCameraWidget::slotCameraChanged()
{
 BO_CHECK_NULL_RET(camera());

 emitSignalCameraChanged();
}

void BoUfoOrbiterCameraWidget::setCamera(BoCamera* camera)
{
 BoUfoCameraConfigWidgetBase::setCamera(camera);
 if (d->mOrbiter) {
	d->mOrbiter->setCamera(camera);
 }
}


BoUfoLightCameraWidget::BoUfoLightCameraWidget(bool showGlobalValues) : BoUfoWidget()
{
 mCamera = 0;
 setLayoutClass(UVBoxLayout);

 BoUfoVBox* layoutWidget = new BoUfoVBox();
 addWidget(layoutWidget);

 mCameraWidget = new BoUfoCameraWidget();
 layoutWidget->addWidget(mCameraWidget);

 mDirectional = new BoUfoCheckBox(i18n("Directional light"));
 layoutWidget->addWidget(mDirectional);

 mConstantAttenuation = new BoUfoNumInput();
 mConstantAttenuation->setLabel(i18n("Constant Attenuation"));
 layoutWidget->addWidget(mConstantAttenuation);

 mLinearAttenuation = new BoUfoNumInput();
 mLinearAttenuation->setLabel(i18n("Linear Attenuation"));
 layoutWidget->addWidget(mLinearAttenuation);

 mQuadraticAttenuation = new BoUfoNumInput();
 mQuadraticAttenuation->setLabel(i18n("Quadratic Attenuation"));
 layoutWidget->addWidget(mQuadraticAttenuation);

 BoUfoHBox* ambientLayoutWidget = new BoUfoHBox();
 layoutWidget->addWidget(ambientLayoutWidget);
 BoUfoLabel* ambientLabel = new BoUfoLabel(i18n("Ambient:"));
 mAmbientR = new BoUfoNumInput();
 mAmbientG = new BoUfoNumInput();
 mAmbientB = new BoUfoNumInput();
 mAmbientA = new BoUfoNumInput();
 ambientLayoutWidget->addWidget(ambientLabel);
 ambientLayoutWidget->addWidget(mAmbientR);
 ambientLayoutWidget->addWidget(mAmbientG);
 ambientLayoutWidget->addWidget(mAmbientB);
 ambientLayoutWidget->addWidget(mAmbientA);

 BoUfoHBox* diffuseLayoutWidget = new BoUfoHBox();
 layoutWidget->addWidget(diffuseLayoutWidget);
 BoUfoLabel* diffuseLabel = new BoUfoLabel(i18n("Diffuse:"));
 mDiffuseR = new BoUfoNumInput();
 mDiffuseG = new BoUfoNumInput();
 mDiffuseB = new BoUfoNumInput();
 mDiffuseA = new BoUfoNumInput();
 diffuseLayoutWidget->addWidget(diffuseLabel);
 diffuseLayoutWidget->addWidget(mDiffuseR);
 diffuseLayoutWidget->addWidget(mDiffuseG);
 diffuseLayoutWidget->addWidget(mDiffuseB);
 diffuseLayoutWidget->addWidget(mDiffuseA);

 BoUfoHBox* specularLayoutWidget = new BoUfoHBox();
 layoutWidget->addWidget(specularLayoutWidget);
 BoUfoLabel* specularLabel = new BoUfoLabel(i18n("Specular:"));
 mSpecularR = new BoUfoNumInput();
 mSpecularG = new BoUfoNumInput();
 mSpecularB = new BoUfoNumInput();
 mSpecularA = new BoUfoNumInput();
 specularLayoutWidget->addWidget(specularLabel);
 specularLayoutWidget->addWidget(mSpecularR);
 specularLayoutWidget->addWidget(mSpecularG);
 specularLayoutWidget->addWidget(mSpecularB);
 specularLayoutWidget->addWidget(mSpecularA);

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
	BoUfoHBox* globalAmbientLayoutWidget = new BoUfoHBox();
	layoutWidget->addWidget(globalAmbientLayoutWidget);
	BoUfoLabel* globalAmbientLabel = new BoUfoLabel(i18n("Global Ambient:"));
	mGlobalAmbientR = new BoUfoNumInput();
	mGlobalAmbientG = new BoUfoNumInput();
	mGlobalAmbientB = new BoUfoNumInput();
	mGlobalAmbientA = new BoUfoNumInput();
	mGlobalAmbientR->setRange(0.0f, 1.0f);
	mGlobalAmbientG->setRange(0.0f, 1.0f);
	mGlobalAmbientB->setRange(0.0f, 1.0f);
	mGlobalAmbientA->setRange(0.0f, 1.0f);
	mGlobalAmbientR->setStepSize(0.1f);
	mGlobalAmbientG->setStepSize(0.1f);
	mGlobalAmbientB->setStepSize(0.1f);
	mGlobalAmbientA->setStepSize(0.1f);
	globalAmbientLayoutWidget->addWidget(globalAmbientLabel);
	globalAmbientLayoutWidget->addWidget(mGlobalAmbientR);
	globalAmbientLayoutWidget->addWidget(mGlobalAmbientG);
	globalAmbientLayoutWidget->addWidget(mGlobalAmbientB);
	globalAmbientLayoutWidget->addWidget(mGlobalAmbientA);
	connect(mGlobalAmbientR, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightModelChanged()));
	connect(mGlobalAmbientG, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightModelChanged()));
	connect(mGlobalAmbientB, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightModelChanged()));
	connect(mGlobalAmbientA, SIGNAL(signalValueChanged(float)), this, SLOT(slotLightModelChanged()));
 }

 mBlockLightChanges = false;
}

BoUfoLightCameraWidget::~BoUfoLightCameraWidget()
{
 delete mCamera;
}

void BoUfoLightCameraWidget::setLight(BoLight* light, BoContext* context)
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

 mConstantAttenuation->setEnabled(!mDirectional->checked());
 mLinearAttenuation->setEnabled(!mDirectional->checked());
 mQuadraticAttenuation->setEnabled(!mDirectional->checked());

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

void BoUfoLightCameraWidget::slotLightChanged()
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

 mLight->setDirectional(mDirectional->checked());
 mLight->setAmbient(BoVector4Float(mAmbientR->value(), mAmbientG->value(), mAmbientB->value(), mAmbientA->value()));
 mLight->setDiffuse(BoVector4Float(mDiffuseR->value(), mDiffuseG->value(), mDiffuseB->value(), mDiffuseA->value()));
 mLight->setSpecular(BoVector4Float(mSpecularR->value(), mSpecularG->value(), mSpecularB->value(), mSpecularA->value()));
 mLight->setConstantAttenuation(mConstantAttenuation->value());
 mLight->setLinearAttenuation(mLinearAttenuation->value());
 mLight->setQuadraticAttenuation(mQuadraticAttenuation->value());

 if (old) {
	old->makeCurrent();
 }

 // when directional light is enabled attenuation is per definition disabled
 mConstantAttenuation->setEnabled(!mDirectional->checked());
 mLinearAttenuation->setEnabled(!mDirectional->checked());
 mQuadraticAttenuation->setEnabled(!mDirectional->checked());
}

void BoUfoLightCameraWidget::slotLightModelChanged()
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


class BosonGLWidgetLight : public BosonUfoGLWidget
{
public:
	BosonGLWidgetLight(QWidget* parent) : BosonUfoGLWidget(parent, "lightwidget")
	{
		mTopWidget = 0;

		setMouseTracking(true);
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
		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor3ub(255, 255, 255);
		if (ufoManager()) {
			ufoManager()->dispatchEvents();
			ufoManager()->render();
		}
	}

public:
	BoUfoWidget* mTopWidget;

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
		makeCurrent();
		recursive = true;
		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
		glShadeModel(GL_FLAT);
		glDisable(GL_DITHER);
		glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
		glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		initUfo();
		mTopWidget = ufoManager()->contentWidget();
//		mTopWidget->setLayoutClass(BoUfoWidget::UVBoxLayout);
		recursive = false;
	}

	virtual void resizeGL(int w, int h)
	{
		glViewport(0, 0, w, h);
		BosonUfoGLWidget::resizeGL(w, h);
	}
};

BoLightCameraWidget1::BoLightCameraWidget1(QWidget* parent, bool showGlobalValues)
	: QWidget(parent)
{
 QVBoxLayout* l = new QVBoxLayout(this);
 mWidget = new BosonGLWidgetLight(this);
 l->addWidget(mWidget);
 mWidget->initialize();

 mLightWidget = new BoUfoLightCameraWidget();
 mWidget->mTopWidget->addWidget(mLightWidget);
 repaint(false);
}

BoLightCameraWidget1::~BoLightCameraWidget1()
{
}

void BoLightCameraWidget1::setLight(BoLight* l, BoContext* c)
{
 mLightWidget->setLight(l, c);
}


