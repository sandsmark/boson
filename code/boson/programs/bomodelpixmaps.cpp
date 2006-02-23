/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bomodelpixmaps.h"
#include "bomodelpixmaps.moc"

#include "../bomemory/bodummymemory.h"
#include "bosonconfig.h"
#include "modelrendering/bosonmodel.h"
#include "borenderrendermodel.h"
#include "bosonprofiling.h"
#include "bodebug.h"
#include "boversion.h"
#include "boapplication.h"
#include "bocamera.h"
#include "botexture.h"
#include "bopixmaprenderer.h"
#include "info/boinfo.h"
#include "bosonviewdata.h"
#include "modelrendering/bomeshrenderermanager.h"
#include "bomaterial.h"
#include "bosonmodeltextures.h"
#include "bolight.h"
#include "bomodelpixmapsgui.h"
#ifdef BOSON_USE_BOMEMORY
#include "bomemory/bomemorydialog.h"
#endif
#include "bofiledialog.h"
#include <bogl.h>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kaction.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <ktar.h>

#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qlistbox.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qfile.h>
#include <qbuffer.h>
#include <qimage.h>

#include <math.h>
#include <stdlib.h>

#define NEAR 0.125
#define FAR 64.0

static const char *description =
    I18N_NOOP("Model Pixmaps for Boson");

static const char *version = BOSON_VERSION_STRING;

static KCmdLineOptions options[] =
{
    { "package", I18N_NOOP("Automatically package all files loaded from cmd line"), 0 },
    { "+[files]", I18N_NOOP("Files for --package"), 0 },
    { "xrotation <rotation>", I18N_NOOP("Initial X rotation"), 0 },
    { "yrotation <rotation>", I18N_NOOP("Initial Y rotation"), 0 },
    { "zrotation <rotation>", I18N_NOOP("Initial Z rotation"), 0 },
    { 0, 0, 0 }
};

void postBosonConfigInit();


BoTextureCopyright::BoTextureCopyright(QWidget* parent)
	: QWidget(parent)
{
 QVBoxLayout* layout = new QVBoxLayout(this);
 mTexture = new QLabel(this);
 layout->addWidget(mTexture);

 QWidget* w;
 QHBoxLayout* h;

 w = new QWidget(this);
 h = new QHBoxLayout(w);
 h->setAutoAdd(true);
 new QLabel(i18n("Author"), w);
 mCopyrightOwner = new QLineEdit(w);
 layout->addWidget(w);

 w = new QWidget(this);
 h = new QHBoxLayout(w);
 h->setAutoAdd(true);
 new QLabel(i18n("License"), w);
 mLicense = new QLineEdit(w);
 layout->addWidget(w);
}

BoTextureCopyright::~BoTextureCopyright()
{
}

void BoTextureCopyright::setTexture(const QString& f)
{
 mFile = f;
 mTexture->setText(i18n("Texture: %1").arg(textureFile()));
}

QString BoTextureCopyright::textureFile() const
{
 if (mFile.isEmpty()) {
	return QString::null;
 }
 QFileInfo info(mFile);
 return info.fileName();
}

void BoTextureCopyright::setAuthor(const QString& name)
{
 mCopyrightOwner->setText(name);
}

QString BoTextureCopyright::author() const
{
 return mCopyrightOwner->text();
}

void BoTextureCopyright::setLicense(const QString& name)
{
 mLicense->setText(name);
}

QString BoTextureCopyright::license() const
{
 return mLicense->text();
}

class BoModelPixmapsGLWidgetPrivate
{
public:
	BoModelPixmapsGLWidgetPrivate()
	{
		mViewData = 0;
		mRenderModel = 0;

		mModel = 0;
	}
	BosonViewData* mViewData;
	BoRenderRenderModel* mRenderModel;
	float mFovY;

	BosonModel* mModel;

	BoMatrix mProjectionMatrix;
	BoMatrix mBaseProjectionMatrix;
};

BoModelPixmapsGLWidget::BoModelPixmapsGLWidget(QWidget* parent)
	: BosonGLWidget(parent, "bomodelpixmapsglwidget", false)
{
 d = new BoModelPixmapsGLWidgetPrivate();

 d->mRenderModel = 0;
 d->mViewData = new BosonViewData(this);
 BosonViewData::setGlobalViewData(d->mViewData);

 d->mFovY = 60.0f;

 boConfig->addDynamicEntry(new BoConfigColorEntry(boConfig, "BoRenderBackgroundColor", QColor(183, 183, 183)));
}

void BoModelPixmapsGLWidget::initWidget()
{
 initGL();
}

BoModelPixmapsGLWidget::~BoModelPixmapsGLWidget()
{
 delete d->mRenderModel;
 BoMeshRendererManager::manager()->unsetCurrentRenderer();
 delete d->mViewData;
 delete d;
 BoTextureManager::deleteStatic();
}

const BosonModel* BoModelPixmapsGLWidget::model() const
{
 return d->mModel;
}

BoCamera* BoModelPixmapsGLWidget::camera() const
{
 BO_CHECK_NULL_RET0(d->mRenderModel);
 return d->mRenderModel->camera();
}

bool BoModelPixmapsGLWidget::parseCmdLineArgs(KCmdLineArgs* args)
{
 if (!loadCamera(args)){
	return false;
 }

#if 0
 if (args->isSet("fovy")) {
	bool ok = false;
	float f = args->getOption("fovy").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "fovy must be a number" << endl;
		return 1;
	}
	d->mFovY = f;
 }
#endif
 return true;
}

bool BoModelPixmapsGLWidget::loadCamera(KCmdLineArgs* args)
{
#if 0
 BoQuaternion quat = camera()->quaternion();
 BoVector3Float cameraPos = camera()->cameraPos();

 // in borender we use a first translate, then rotate approach, whereas the
 // camera does it the other way round. we need to transform the vector first.
 quat.transform(&cameraPos, &cameraPos);

 if (args->isSet("camera-x")) {
	bool ok = false;
	float c = args->getOption("camera-x").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "camera-x must be a number" << endl;
		return false;
	}
	cameraPos.setX(c);
 }
 if (args->isSet("camera-y")) {
	bool ok = false;
	float c = args->getOption("camera-y").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "camera-y must be a number" << endl;
		return false;
	}
	cameraPos.setY(c);
 }
 if (args->isSet("camera-z")) {
	bool ok = false;
	float c = args->getOption("camera-z").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "camera-z must be a number" << endl;
		return false;
	}
	cameraPos.setZ(c);
 }
 if (args->isSet("rotate-x")) {
	bool ok = false;
	float r = args->getOption("rotate-x").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "rotate-x must be a number" << endl;
		return false;
	}
	BoQuaternion q;
	q.setRotation(r, 0.0f, 0.0f);
	quat.multiply(q);
 }
 if (args->isSet("rotate-y")) {
	bool ok = false;
	float r = args->getOption("rotate-y").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "rotate-y must be a number" << endl;
		return false;
	}
	BoQuaternion q;
	q.setRotation(0.0f, r, 0.0f);
	quat.multiply(q);
 }
 if (args->isSet("rotate-z")) {
	bool ok = false;
	float r = args->getOption("rotate-z").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "rotate-z must be a number" << endl;
		return false;
	}
	BoQuaternion q;
	q.setRotation(0.0f, 0.0f, r);
	quat.multiply(q);
 }
 // re-transform to gluLookAt() values
 quat.inverse().transform(&cameraPos, &cameraPos);

 if (args->isSet("lookAtCenter")) {
	if (args->isSet("rotate-x") ||
			args->isSet("rotate-y") ||
			args->isSet("rotate-z")) {
		boWarning() << "--rotate-x, --rotate-y and --rotate-z are ignored when --lookAtCenter was specified!" << endl;
	}
	BoVector3Float lookAt = BoVector3Float(0, 0, 0);
	BoVector3Float up = BoVector3Float(0, 0, 1);
	quat.setRotation(cameraPos, lookAt, up);
 }

 BoVector3Float lookAt, up;
 quat.matrix().toGluLookAt(&lookAt, &up, BoVector3Float(0, 0, 0));

 updateCamera(cameraPos, lookAt, up);
#endif

 return true;
}

void BoModelPixmapsGLWidget::initializeGL()
{
 boDebug() << k_funcinfo << endl;
 makeCurrent();
 BoInfo::boInfo()->update(this);
 setUpdatesEnabled(false);

 BoTextureManager::initStatic();
 BoLightManager::initStatic();
 BoMeshRendererManager::initStatic();
 boTextureManager->initOpenGL();

 glClearColor(0.0, 0.0, 0.0, 0.0);
 glShadeModel(GL_FLAT);
 glDisable(GL_DITHER);
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 // not supported atm
 boConfig->setBoolValue("UseLight", false);

 if (d->mRenderModel) {
	boWarning() << k_funcinfo << "called twice" << endl;
	return;
 }
 d->mRenderModel = new BoRenderRenderModel(this);

 updateBaseProjectionMatrix();
}

const BoMatrix& BoModelPixmapsGLWidget::baseProjectionMatrix() const
{
 return d->mBaseProjectionMatrix;
}

void BoModelPixmapsGLWidget::resizeGL(int width, int height)
{
 BosonGLWidget::resizeGL(width, height);
 updateBaseProjectionMatrix();
}

void BoModelPixmapsGLWidget::updateBaseProjectionMatrix()
{
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 gluPerspective(d->mFovY, (float)width() / (float)height(), NEAR, FAR);
 d->mBaseProjectionMatrix = createMatrixFromOpenGL(GL_PROJECTION_MATRIX);
 glMatrixMode(GL_MODELVIEW);
 d->mProjectionMatrix = d->mBaseProjectionMatrix;
}

void BoModelPixmapsGLWidget::setProjectionMatrix(const BoMatrix& matrix)
{
 d->mProjectionMatrix = matrix;
}

void BoModelPixmapsGLWidget::paintGL()
{
 QColor background = boConfig->colorValue("BoRenderBackgroundColor", Qt::black);
 glClearColor((GLfloat)background.red() / 255.0f, (GLfloat)background.green() / 255.0f, background.blue() / 255.0f, 0.0f);

 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 boTextureManager->clearStatistics();

 boDebug() << k_funcinfo << width() << " " << height() << endl;

 glPushAttrib(GL_ALL_ATTRIB_BITS);
 glViewport(0, 0, width(), height());

 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadMatrixf(d->mProjectionMatrix.data());
 glMatrixMode(GL_MODELVIEW);
 glColor3ub(255, 255, 255);

 boTextureManager->invalidateCache();
 d->mRenderModel->render();

 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glPopAttrib();
}

bool BoModelPixmapsGLWidget::loadModel(const QString& file)
{
 QFileInfo info(file);
 if (!info.exists()) {
	return false;
 }
 resetModel();
 delete d->mModel;
 d->mModel = new BosonModel(info.dirPath(true) + "/", info.fileName());
 bool ret = d->mModel->loadModel(QString::null);
 if (ret) {
	d->mRenderModel->setModel(d->mModel);
 } else {
	d->mRenderModel->setModel(0);
}
 return ret;
}

void BoModelPixmapsGLWidget::resetModel()
{
 d->mRenderModel->setModel(0);
 delete d->mModel;
 d->mModel = 0;
}



#define PIXMAP_WIDTH 600
#define PIXMAP_HEIGHT 400
#define PIXMAP_THUMBNAIL_WIDTH 150
#define PIXMAP_THUMBNAIL_HEIGHT 100

class BoModelPixmapsPrivate
{
public:
	BoModelPixmapsPrivate()
	{
		mGUI = 0;
		mTextureCopyrights = 0;
	}
	BoModelPixmapsGUI* mGUI;
	QWidget* mTextureCopyrights;
};

BoModelPixmaps::BoModelPixmaps()
	: KMainWindow()
{
 d = new BoModelPixmapsPrivate;
 mGLWidget = new BoModelPixmapsGLWidget(0);
 mPixmapRenderer = new BoPixmapRenderer();
 mPixmapRenderer->setWidget(mGLWidget, PIXMAP_WIDTH, PIXMAP_HEIGHT);
 mGLWidget->initWidget();
 mGLWidget->hide();

 mIface = new BoDebugDCOPIface();

 boConfig->addDynamicEntry(new BoConfigStringEntry(boConfig, "TexturePath", ""));

 d->mGUI = new BoModelPixmapsGUI(this);
 d->mGUI->mTextureCopyrights->setOrientation(Qt::Vertical);
 d->mGUI->mTextureCopyrights->setColumnLayout(1, Qt::Vertical);
 QScrollView* textureCopyrightScroll = new QScrollView(d->mGUI->mTextureCopyrights);
 textureCopyrightScroll->setResizePolicy(QScrollView::AutoOneFit);
 d->mTextureCopyrights = new QWidget(textureCopyrightScroll->viewport());
 QVBoxLayout* textureCopyrightsLayout = new QVBoxLayout(d->mTextureCopyrights, 5, 5);
 textureCopyrightsLayout->setAutoAdd(true);
 textureCopyrightScroll->addChild(d->mTextureCopyrights);

 connect(d->mGUI->mModelFile, SIGNAL(clicked()),
		this, SLOT(slotSelectModelFile()));
 connect(d->mGUI->mTextureDirectory, SIGNAL(clicked()),
		this, SLOT(slotSelectTextureDirectory()));
 connect(d->mGUI->mCheckPackaging, SIGNAL(clicked()),
		this, SLOT(slotCheckPackage()));
 connect(d->mGUI->mPackageIt, SIGNAL(clicked()),
		this, SLOT(slotPackageIt()));
 QWidget* pixmapsWidget = d->mGUI->mTabWidget->page(0);
 BO_CHECK_NULL_RET(pixmapsWidget);
 mModelPixmapLabelsLayout = new QGridLayout(pixmapsWidget, -1, 3);
 displayLabels(6);

 setCentralWidget(d->mGUI);

 mTextureCopyright.setAutoDelete(true);

 reset();
 selectTextureDirectory(boConfig->stringValue("TexturePath"));
}

BoModelPixmaps::~BoModelPixmaps()
{
 boConfig->save(false);
 mTextureCopyright.setAutoDelete(true);
 mTextureCopyright.clear();
 delete mPixmapRenderer;
 delete mGLWidget;
 delete mIface;
 delete d;
}

bool BoModelPixmaps::parseCmdLineArgs(KCmdLineArgs* args)
{
 if (!mGLWidget->parseCmdLineArgs(args)) {
	return false;
 }
 if (args->isSet("xrotation")) {
	bool ok;
	args->getOption("xrotation").toDouble(&ok);
	if (!ok) {
		boError() << k_funcinfo << "value for xrotation is not a valid number" << endl;
		return false;
	}
	d->mGUI->mRotationX->setText(args->getOption("xrotation"));
 }
 if (args->isSet("yrotation")) {
	bool ok;
	args->getOption("yrotation").toDouble(&ok);
	if (!ok) {
		boError() << k_funcinfo << "value for yrotation is not a valid number" << endl;
		return false;
	}
	d->mGUI->mRotationY->setText(args->getOption("yrotation"));
 }
 if (args->isSet("zrotation")) {
	bool ok;
	args->getOption("zrotation").toDouble(&ok);
	if (!ok) {
		boError() << k_funcinfo << "value for zrotation is not a valid number" << endl;
		return false;
	}
	d->mGUI->mRotationZ->setText(args->getOption("zrotation"));
 }
 return true;
}

void BoModelPixmaps::slotSelectModelFile()
{
 QString file = BoFileDialog::getOpenFileName();
 if (file.isEmpty()) {
	return;
 }
 selectModelFile(file);
}

void BoModelPixmaps::selectModelFile(const QString& file)
{
 reset();
 if (!mGLWidget->loadModel(file)) {
	KMessageBox::sorry(this, i18n("Unable to load from file %1").arg(file));
	reset();
	return;
 }
 mModelFileName = file;
 d->mGUI->mModelFile->setText(mModelFileName);
 retrievePixmaps();

 QFileInfo fileInfo(mModelFileName);
 QString copyrightFile = fileInfo.dirPath(true) + "/" + fileInfo.baseName() + ".copyright";
 if (QFile::exists(copyrightFile)) {
	KSimpleConfig conf(copyrightFile);
	if (!conf.hasGroup("Copyright")) {
		boWarning() << k_funcinfo << "no Copyright group found in file " << copyrightFile << endl;
	} else {
		conf.setGroup("Copyright");
		d->mGUI->mModelAuthor->setText(conf.readEntry("Author"));
		d->mGUI->mModelLicense->setText(conf.readEntry("License"));
	}
 } else {
	boDebug() << k_funcinfo << "no .copyright file for model " << mModelFileName << endl;
 }

 const BosonModel* model = mGLWidget->model();
 BO_CHECK_NULL_RET(model);
 QStringList textures;
 for (unsigned int i = 0; i < model->materialCount(); i++) {
	BoMaterial* mat = model->material(i);
	if (!mat) {
		BO_NULL_ERROR(mat);
		continue;
	}
	if (mat->textureName().isEmpty()) {
		continue;
	}
	if (!textures.contains(mat->textureName())) {
		textures.append(mat->textureName());
	}
 }

 QStringList found;
 QStringList notFound;
 QString path = BosonModelTextures::modelTextures()->texturePath() + "/";
 for (QStringList::iterator it = textures.begin(); it != textures.end(); ++it) {
	BoTexture* tex = BosonModelTextures::modelTextures()->texture(*it);
	bool foundTexture = true;
	if (!tex) {
		foundTexture = false;
	} else {
		QFileInfo textureInfo(tex->filePath());
		if (!textureInfo.exists()) {
			foundTexture = false;
		}
	}

	if (!foundTexture) {
		notFound.append(*it);
		boDebug() << k_funcinfo << "texture not found: " << *it << endl;
	} else {
		found.append(*it);
		addTextureCopyright(path + *it);
	}
 }
 d->mGUI->mTexturesFoundList->insertStringList(found);
 d->mGUI->mTexturesNotFoundList->insertStringList(notFound);
}

void BoModelPixmaps::slotSelectTextureDirectory()
{
 QString old = BosonModelTextures::modelTextures()->texturePath();
 QString dir = BoFileDialog::getExistingDirectory(old, this);
 if (dir.isEmpty()) {
	return;
 }
 selectTextureDirectory(dir);
}

void BoModelPixmaps::selectTextureDirectory(const QString& dir_)
{
 if (dir_.isEmpty()) {
	return;
 }
 QString dir = dir_;
 if (dir.right(1) != "/") {
	dir += "/";
 }
 BosonModelTextures::modelTextures()->setTexturePath(dir);
 QString model = mModelFileName;
 reset();
 if (!model.isEmpty()) {
	selectModelFile(model);
 }

 d->mGUI->mTextureDirectory->setText(dir);
 boConfig->setStringValue("TexturePath", dir);
}

void BoModelPixmaps::reset()
{
 d->mGUI->mModelFile->setText(i18n("Model..."));
 mModelPixmaps.setAutoDelete(true);
 mModelPixmaps.clear();
 mModelFileName = QString::null;
 mGLWidget->resetModel();
 d->mGUI->mTexturesFoundList->clear();
 d->mGUI->mTexturesNotFoundList->clear();
 for (unsigned int i = 0; i < mModelPixmapLabels.count(); i++) {
	mModelPixmapLabels[i]->setPixmap(QPixmap());
 }
 mTextureCopyright.setAutoDelete(true);
 mTextureCopyright.clear();
 d->mGUI->mModelAuthor->setText("");
 d->mGUI->mModelLicense->setText("");
}

void BoModelPixmaps::retrievePixmaps()
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(mGLWidget->camera());

 QValueList<BoVector3Float> cameraPosList;
 QValueList<BoVector3Float> lookAtList;
 QValueList<BoVector3Float> upList;
 QValueList<QString> namesList;

 BoVector3Float center = (mGLWidget->model()->boundingBoxMin() + mGLWidget->model()->boundingBoxMax()) / 2;
 BoVector3Float dimensions = mGLWidget->model()->boundingBoxMax() - mGLWidget->model()->boundingBoxMin();
 float size = dimensions.length();

 cameraPosList.append(BoVector3Float(2.0f, 2.0f, 2.0f));
 lookAtList.append(BoVector3Float(0.0f, 0.0f, 0.0f));
 upList.append(BoVector3Float(0.0f, 0.0f, 50.0f));
 namesList.append("bird1");

 cameraPosList.append(BoVector3Float(-2.0f, 2.0f, 2.0f));
 lookAtList.append(BoVector3Float(0.0f, 0.0f, 0.0f));
 upList.append(BoVector3Float(0.0f, 0.0f, 50.0f));
 namesList.append("bird2");

 cameraPosList.append(BoVector3Float(2.0f, -2.0f, 2.0f));
 lookAtList.append(BoVector3Float(0.0f, 0.0f, 0.0f));
 upList.append(BoVector3Float(0.0f, 0.0f, 50.0f));
 namesList.append("bird3");

 cameraPosList.append(BoVector3Float(-2.0f, -2.0f, 2.0f));
 lookAtList.append(BoVector3Float(0.0f, 0.0f, 0.0f));
 upList.append(BoVector3Float(0.0f, 0.0f, 50.0f));
 namesList.append("bird4");

 cameraPosList.append(BoVector3Float(0.0f, 0.0f, 2.0f));
 lookAtList.append(BoVector3Float(0.0f, 0.0f, 0.0f));
 upList.append(BoVector3Float(0.0f, 50.0f, 0.0f));
 namesList.append("above");

 cameraPosList.append(BoVector3Float(0.0f, 0.0f, -2.0f));
 lookAtList.append(BoVector3Float(0.0f, 0.0f, 0.0f));
 upList.append(BoVector3Float(0.0f, 50.0f, 0.0f));
 namesList.append("below");

 cameraPosList.append(BoVector3Float(0.0f, 2.0f, 0.0f));
 lookAtList.append(BoVector3Float(0.0f, 0.0f, 0.0f));
 upList.append(BoVector3Float(0.0f, 0.0f, 50.0f));
 namesList.append("front");

 cameraPosList.append(BoVector3Float(0.0f, -2.0f, 0.0f));
 lookAtList.append(BoVector3Float(0.0f, 0.0f, 0.0f));
 upList.append(BoVector3Float(0.0f, 0.0f, 50.0f));
 namesList.append("rear");

 cameraPosList.append(BoVector3Float(2.0f, 0.0f, 0.0f));
 lookAtList.append(BoVector3Float(0.0f, 0.0f, 0.0f));
 upList.append(BoVector3Float(0.0f, 0.0f, 50.0f));
 namesList.append("right");

 cameraPosList.append(BoVector3Float(-2.0f, 0.0f, 0.0f));
 lookAtList.append(BoVector3Float(0.0f, 0.0f, 0.0f));
 upList.append(BoVector3Float(0.0f, 0.0f, 50.0f));
 namesList.append("left");

 for (unsigned int i = 0; i < namesList.count(); i++) {
	if (namesList.contains(namesList[i]) > 1) {
		boError() << k_funcinfo << "name " << namesList[i] << " used more than once" << endl;
		return;
	}
 }

 if (cameraPosList.count() != lookAtList.count()
		|| cameraPosList.count() != upList.count()
		|| cameraPosList.count() != namesList.count()) {
	boError() << k_funcinfo << endl;
	return;
 }
 displayLabels(cameraPosList.count());
 mModelPixmaps.setAutoDelete(true);
 mModelPixmaps.clear();
 mGLWidget->updateBaseProjectionMatrix();

 // unify the model to a default rotation
 BoMatrix unifyModelMatrix = BoMatrix();
 bool ok;
 float rotX = d->mGUI->mRotationX->text().toDouble(&ok);
 if (!ok) {
	boError() << k_funcinfo << "value for x rotation not a valid number" << endl;
	rotX = 0.0f;
 }
 float rotY = d->mGUI->mRotationY->text().toDouble(&ok);
 if (!ok) {
	boError() << k_funcinfo << "value for y rotation not a valid number" << endl;
	rotY = 0.0f;
 }
 float rotZ = d->mGUI->mRotationZ->text().toDouble(&ok);
 if (!ok) {
	boError() << k_funcinfo << "value for z rotation not a valid number" << endl;
	rotZ = 0.0f;
 }
 unifyModelMatrix.rotate(rotX, 1.0f, 0.0f, 0.0f);
 unifyModelMatrix.rotate(rotY, 0.0f, 1.0f, 0.0f);
 unifyModelMatrix.rotate(rotZ, 0.0f, 0.0f, 1.0f);

 for (unsigned int i = 0; i < cameraPosList.count(); i++) {
	BoVector3Float cameraPos_ = center + cameraPosList[i] * size;
	BoVector3Float lookAt_ = center + lookAtList[i];
	BoVector3Float up_ = upList[i];
	QString name = namesList[i];

	BoVector3Float cameraPos;
	BoVector3Float lookAt;
	BoVector3Float up;
	unifyModelMatrix.transform(&cameraPos, &cameraPos_);
	unifyModelMatrix.transform(&lookAt, &lookAt_);
	unifyModelMatrix.transform(&up, &up_);

	if (name.isEmpty()) {
		boError() << k_funcinfo << "empty name for pixmap " << i << endl;
		continue;
	}

	mGLWidget->camera()->setGluLookAt(cameraPos, lookAt, up);
	fitModelIntoView(cameraPos, lookAt, up);
	QPixmap p = mPixmapRenderer->getPixmap();

	QImage img = p.convertToImage();
	img = img.smoothScale(PIXMAP_THUMBNAIL_WIDTH, PIXMAP_THUMBNAIL_HEIGHT, QImage::ScaleMin);
	QPixmap thumbnail;
	thumbnail.convertFromImage(img);

	BoModelPixmapCollection* collection = new BoModelPixmapCollection();
	collection->setName(name);
	collection->setPixmap(p);
	collection->setThumbnail(thumbnail);
	mModelPixmaps.append(collection);
 }

 for (unsigned int i = 0; i < mModelPixmaps.count(); i++) {
	mModelPixmapLabels[i]->setPixmap(mModelPixmaps.at(i)->thumbnail());
 }
}

void BoModelPixmaps::fitModelIntoView(const BoVector3Float& cameraPos, const BoVector3Float& lookAt,
	const BoVector3Float& up)
{
 // Calculate model's bbox vertices
 BoVector3Float modelMin = mGLWidget->model()->boundingBoxMin();
 BoVector3Float modelMax = mGLWidget->model()->boundingBoxMax();
 BoVector3Float boundingBoxVertices[8];
 boundingBoxVertices[0].set(modelMin.x(), modelMin.y(), modelMin.z());
 boundingBoxVertices[1].set(modelMax.x(), modelMin.y(), modelMin.z());
 boundingBoxVertices[2].set(modelMax.x(), modelMax.y(), modelMin.z());
 boundingBoxVertices[3].set(modelMin.x(), modelMax.y(), modelMin.z());
 boundingBoxVertices[4].set(modelMin.x(), modelMin.y(), modelMax.z());
 boundingBoxVertices[5].set(modelMax.x(), modelMin.y(), modelMax.z());
 boundingBoxVertices[6].set(modelMax.x(), modelMax.y(), modelMax.z());
 boundingBoxVertices[7].set(modelMin.x(), modelMax.y(), modelMax.z());

 // Create projection and modelview matrices
 BoMatrix modelviewMatrix;
 modelviewMatrix.setLookAtRotation(cameraPos, lookAt, up);
 modelviewMatrix.translate(-cameraPos.x(), -cameraPos.y(), -cameraPos.z());
 BoMatrix projectionMatrix = mGLWidget->baseProjectionMatrix();

 // Transform the bbox vertices into clip space
 BoVector4Float E[8];
 for (int i = 0; i < 8; i++) {
	BoVector4Float tmp;
	BoVector4Float in(boundingBoxVertices[i].x(), boundingBoxVertices[i].y(), boundingBoxVertices[i].z(), 1.0f);
	modelviewMatrix.transform(&tmp, &in);
	projectionMatrix.transform(&E[i], &tmp);
	// Divide by w to get eucleidian coordinates
	E[i] = E[i] / E[i].w();
 }

 // Find min/max of the transformed points
 BoVector3Float Emin(E[0].x(), E[0].y(), E[0].z());
 BoVector3Float Emax(E[0].x(), E[0].y(), E[0].z());
 for (int i = 0; i < 8; i++) {
	Emin.setX(QMIN(Emin.x(), E[i].x()));
	Emin.setY(QMIN(Emin.y(), E[i].y()));
	Emin.setZ(QMIN(Emin.z(), E[i].z()));
	Emax.setX(QMAX(Emax.x(), E[i].x()));
	Emax.setY(QMAX(Emax.y(), E[i].y()));
	Emax.setZ(QMAX(Emax.z(), E[i].z()));
 }
 //boDebug() << "min: (" << Emin.x() << "; " << Emin.y() << "; " << Emin.z() << ")" << endl;
 //boDebug() << "max: (" << Emax.x() << "; " << Emax.y() << "; " << Emax.z() << ")" << endl;

 BoVector3Float Emid = (Emin + Emax) / 2;

 // Change the projection matrix
 float scale = QMIN(2 / (Emax.x() - Emin.x()),  2 / (Emax.y() - Emin.y()));
 scale /= 1.15f;  // To get some border
 float translateX = -Emid.x(); // Should be 0 or very close to it
 float translateY = -Emid.y(); // Should be 0 or very close to it
 //boDebug() << "scale: (" << scale << endl;
 //boDebug() << "trans: (" << translateX << "; " << translateY << ")" << endl;
 projectionMatrix.scale(scale, scale, 1.0f);
 projectionMatrix.translate(translateX, translateY, 0.0f);
 mGLWidget->setProjectionMatrix(projectionMatrix);
}

void BoModelPixmaps::addTextureCopyright(const QString& file)
{
 if (file.isEmpty()) {
	return;
 }
 QFileInfo textureFileInfo(file);
 if (!textureFileInfo.exists()) {
	boError() << k_funcinfo << "cannot find " << file << endl;
	return;
 }
 int index = file.findRev('.');
 if (index < 0) {
	boError() << k_funcinfo << "file " << file << " has no '.'" << endl;
	return;
 }

 BoTextureCopyright* copyrightWidget = new BoTextureCopyright(d->mTextureCopyrights);
 copyrightWidget->setTexture(file);
 copyrightWidget->show();
 mTextureCopyright.append(copyrightWidget);



 QString copyrightFile = file.left(index) + ".copyright";
 QFileInfo copyrightFileInfo(copyrightFile);
 if (copyrightFileInfo.exists()) {
	boDebug() << k_funcinfo << "found .copyright file for texture " << textureFileInfo.fileName() << endl;
	KSimpleConfig conf(copyrightFile);
	if (!conf.hasGroup("Copyright")) {
		boWarning() << k_funcinfo << "no Copyright group found in file " << copyrightFile << endl;
	} else {
		conf.setGroup("Copyright");
		copyrightWidget->setAuthor(conf.readEntry("Author"));
		copyrightWidget->setLicense(conf.readEntry("License"));
	}
 } else {
	boDebug() << k_funcinfo << "no .copyright file for texture " << textureFileInfo.fileName() << endl;
 }
}

bool BoModelPixmaps::slotCheckPackage()
{
 d->mGUI->mPackagingWarnings->clear();
 if (d->mGUI->mModelAuthor->text().isEmpty()) {
	d->mGUI->mPackagingWarnings->insertItem(i18n("Model has unknown author!"));
 }
 if (d->mGUI->mModelLicense->text().isEmpty()) {
	d->mGUI->mPackagingWarnings->insertItem(i18n("Model has unknown license!"));
 }
 if (d->mGUI->mTexturesNotFoundList->count() > 0) {
	d->mGUI->mPackagingWarnings->insertItem(i18n("Some textures not found!"));
 }
 for (QPtrListIterator<BoTextureCopyright> it(mTextureCopyright); it.current(); ++it) {
	if (it.current()->author().isEmpty()) {
		d->mGUI->mPackagingWarnings->insertItem(i18n("Texture %1 has unknown author!").arg(it.current()->textureFile()));
	}
	if (it.current()->license().isEmpty()) {
		d->mGUI->mPackagingWarnings->insertItem(i18n("Texture %1 has unknown license!").arg(it.current()->textureFile()));
	}
 }

 bool ret = false; // warnings found;
 if (d->mGUI->mPackagingWarnings->count() == 0) {
	d->mGUI->mPackagingWarnings->insertItem(i18n("No problems found."));
	ret = true; // no warnings found
 }
 return ret;
}

void BoModelPixmaps::slotPackageIt()
{
 if (!slotCheckPackage()) {
	int ret = KMessageBox::questionYesNo(this, i18n("Warnings were found. Package anyway?"));
	if (ret != KMessageBox::Yes) {
		return;
	}
 }
 QString fileName = BoFileDialog::getSaveFileName(QString::null, "*.tar.gz", this);
 if (fileName.isEmpty()) {
	return;
 }
 packageIt(fileName);
}

void BoModelPixmaps::packageIt(const QString& fileName)
{
 if (QFile::exists(fileName)) {
	int ret = KMessageBox::questionYesNo(this, i18n("The file already exists. Overwrite?"));
	if (ret != KMessageBox::Yes) {
		return;
	}
 }

 KTar tar(fileName, QString::fromLatin1("application/x-gzip"));
 if (!tar.open(IO_WriteOnly)) {
	KMessageBox::sorry(this, i18n("Could not open %1 for writing").arg(fileName));
	return;
 }
 QFileInfo info(fileName);
 QString mainDir = info.baseName() + "/";

 const KArchiveDirectory* top = tar.directory();
 BO_CHECK_NULL_RET(top);
 QString user = top->user();
 QString group = top->group();

 QFile file(mModelFileName);
 if (!file.open(IO_ReadOnly)) {
	KMessageBox::sorry(this, i18n("Could not open %1 for reading").arg(file.name()));
	return;
 }
 tar.writeFile(mainDir + QFileInfo(file).fileName(), user, group, file.size(), file.readAll());
 file.close();

 QByteArray modelCopyrightData;
 QTextStream modelCopyrightStream(modelCopyrightData, IO_WriteOnly);
 modelCopyrightStream << "[Copyright]\n";
 modelCopyrightStream << "Author=";
 if (d->mGUI->mModelAuthor->text().isEmpty()) {
	modelCopyrightStream << "???";
 } else {
	modelCopyrightStream << d->mGUI->mModelAuthor->text();
 }
 modelCopyrightStream << "\n";
 modelCopyrightStream << "License=";
 if (d->mGUI->mModelLicense->text().isEmpty()) {
	modelCopyrightStream << "???";
 } else {
	modelCopyrightStream << d->mGUI->mModelLicense->text();
 }
 modelCopyrightStream << "\n";
 modelCopyrightStream << "\n";
 tar.writeFile(mainDir + QFileInfo(file).baseName() + ".copyright", user, group, modelCopyrightData.size(), modelCopyrightData);

 QStringList textureLicenses;
 for (QPtrListIterator<BoTextureCopyright> it(mTextureCopyright); it.current(); ++it) {
	QString texture = it.current()->texture();
	file.setName(texture);
	if (!file.open(IO_ReadOnly)) {
		KMessageBox::sorry(this, i18n("Could not open %1 for reading").arg(file.name()));
		return;
	}
	tar.writeFile(mainDir + "textures/" + QFileInfo(file).fileName(), user, group, file.size(), file.readAll());
	file.close();

	QByteArray copyright;
	QTextStream s(copyright, IO_WriteOnly);
	s << "[Copyright]\n";
	s << "Author=";
	if (it.current()->author().isEmpty()) {
		s << "???";
	} else {
		s << it.current()->author();
	}
	s << "\n";
	s << "License=";
	if (it.current()->license().isEmpty()) {
		s << "???";
		QString license = i18n("Unclear");
		if (!textureLicenses.contains(license)) {
			textureLicenses.append(license);
		}
	} else {
		s << it.current()->license();
		if (!textureLicenses.contains(it.current()->license())) {
			textureLicenses.append(it.current()->license());
		}
	}
	s << "\n";
	s << "\n";
	tar.writeFile(mainDir + "textures/" + QFileInfo(file).baseName() + ".copyright", user, group, copyright.size(), copyright);
 }

 for (QPtrListIterator<BoModelPixmapCollection> it(mModelPixmaps); it.current(); ++it) {
	QString name = it.current()->name();
	QPixmap p = it.current()->pixmap();
	QPixmap thumb = it.current()->thumbnail();

	QByteArray pixmapData;
	QBuffer pixmapBuffer(pixmapData);
	pixmapBuffer.open(IO_WriteOnly);
	if (!p.save(&pixmapBuffer, "JPEG", 75)) {
		KMessageBox::sorry(this, i18n("An image could not be saved"));
		return;
	}
	pixmapBuffer.close();

	QByteArray thumbnailData;
	QBuffer thumbnailBuffer(thumbnailData);
	thumbnailBuffer.open(IO_WriteOnly);
	if (!thumb.save(&thumbnailBuffer, "JPEG", 75)) {
		KMessageBox::sorry(this, i18n("A thumbnail could not be saved"));
		return;
	}
	thumbnailBuffer.close();

	tar.writeFile(mainDir + "images/" + name + ".jpg", user, group, pixmapData.size(), pixmapData);
	tar.writeFile(mainDir + "thumbnails/" + name + ".jpg", user, group, thumbnailData.size(), thumbnailData);
 }

 QByteArray summaryData;
 QTextStream summary(summaryData, IO_WriteOnly);
 summary << "Summary\n";
 summary << "Model license: ";
 if (d->mGUI->mModelLicense->text().isEmpty()) {
	summary << i18n("Unclear");
 } else {
	summary << d->mGUI->mModelLicense->text();
 }
 summary << "\n";
 summary << "Texture licenses: " << textureLicenses.join(",") << "\n";
 summary << "\n";

 tar.writeFile(mainDir + "summary.txt", user, group, summaryData.size(), summaryData);

 tar.close();
}

void BoModelPixmaps::displayLabels(int count)
{
 QWidget* pixmapsWidget = d->mGUI->mTabWidget->page(0);
 BO_CHECK_NULL_RET(pixmapsWidget);
 for (int i = mModelPixmapLabels.count(); i < count; i++) {
	QLabel* l = new QLabel(pixmapsWidget);
	l->resize(PIXMAP_THUMBNAIL_WIDTH, PIXMAP_THUMBNAIL_HEIGHT);
	mModelPixmapLabelsLayout->addWidget(l, i / 3, i % 3);
	mModelPixmapLabels.append(l);
 }
 for (int i = 0; i < count; i++) {
	mModelPixmapLabels.at(i)->show();
 }
 for (int i = count; i < (int)mModelPixmapLabels.count(); i++) {
	mModelPixmapLabels.at(i)->hide();
 }
}



int main(int argc, char **argv)
{
 KAboutData about("bomodelpixmaps",
		I18N_NOOP("Boson Model Pixmaps"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 2005 Andreas Beckermann",
		0,
		"http://boson.eu.org");
 about.addAuthor( "Andreas Beckermann", I18N_NOOP("Coding & Current Maintainer"), "b_mann@gmx.de" );

 // we need to do extra stuff after BosonConfig's initialization
 BosonConfig::setPostInitFunction(&postBosonConfigInit);

 QCString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

 BoApplication app(argv0);
 QObject::connect(kapp, SIGNAL(lastWindowClosed()), kapp, SLOT(quit()));

 BoModelPixmaps* main = new BoModelPixmaps();
 kapp->setMainWidget(main);
 main->show();

 if (!main->parseCmdLineArgs(args)) {
	return 1;
 }

 if (args->isSet("package")) {
	for (int i = 0; i < args->count(); i++) {
		main->selectModelFile(QFile::decodeName(args->arg(i)));
		QString file = QFileInfo(args->arg(i)).baseName() + ".tar.gz";
		main->packageIt(file);
	}
 }

 args->clear();

 return app.exec();
}


void postBosonConfigInit()
{
 boConfig->setBoolValue("ForceDisableSound", true);
}
