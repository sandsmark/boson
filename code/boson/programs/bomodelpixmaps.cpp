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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bomodelpixmaps.h"
#include "bomodelpixmaps.moc"

#include "../../bomemory/bodummymemory.h"
#include "../bosonconfig.h"
#include "../modelrendering/bosonmodel.h"
#include "../modelrendering/bomeshrenderermanager.h"
#include "../modelrendering/bosonmodeltextures.h"
#include "borenderrendermodel.h"
#include "../bosonprofiling.h"
#include "bodebug.h"
#include "../boversion.h"
#include "../boapplication.h"
#include "../bocamera.h"
#include "../botexture.h"
#include "../bopixmaprenderer.h"
#include "../info/boinfo.h"
#include "../bosonviewdata.h"
#include "../bomaterial.h"
#include "../bolight.h"
#ifdef BOSON_USE_BOMEMORY
#include "../../bomemory/bomemorydialog.h"
#endif
#include "../bofiledialog.h"
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
#include <q3listbox.h>
#include <q3groupbox.h>
#include <qlineedit.h>
#include <qfile.h>
#include <qbuffer.h>
#include <qimage.h>
//Added by qt3to4:
#include <Q3TextStream>
#include <Q3HBoxLayout>
#include <Q3ValueList>
#include <Q3GridLayout>
#include <Q3CString>
#include <QPixmap>
#include <Q3VBoxLayout>

#include <math.h>
#include <stdlib.h>

#define NEAR 0.125
#define FAR 64.0

static KLocalizedString description =
    ki18n("Model Pixmaps for Boson");

static const char *version = BOSON_VERSION_STRING;

void postBosonConfigInit();


BoTextureCopyright::BoTextureCopyright(QWidget* parent)
	: QWidget(parent)
{
 Q3VBoxLayout* layout = new Q3VBoxLayout(this);
 mTexture = new QLabel(this);
 layout->addWidget(mTexture);

 QWidget* w;
 Q3HBoxLayout* h;

 w = new QWidget(this);
 h = new Q3HBoxLayout(w);
 h->setAutoAdd(true);
 new QLabel(i18n("Author"), w);
 mCopyrightOwner = new QLineEdit(w);
 layout->addWidget(w);

 w = new QWidget(this);
 h = new Q3HBoxLayout(w);
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
	: QGLWidget(QGLFormat(QGL::IndirectRendering), parent)
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
 glInit();
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
 QGLWidget::resizeGL(width, height);
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

 boConfig->addDynamicEntry(new BoConfigStringEntry(boConfig, "TexturePath", ""));

 d->mGUI = new BoModelPixmapsGUI(this);
 d->mGUI->mTextureCopyrights->setOrientation(Qt::Vertical);
 d->mGUI->mTextureCopyrights->setColumnLayout(1, Qt::Vertical);
 Q3ScrollView* textureCopyrightScroll = new Q3ScrollView(d->mGUI->mTextureCopyrights);
 textureCopyrightScroll->setResizePolicy(Q3ScrollView::AutoOneFit);
 d->mTextureCopyrights = new QWidget(textureCopyrightScroll->viewport());
 Q3VBoxLayout* textureCopyrightsLayout = new Q3VBoxLayout(d->mTextureCopyrights, 5, 5);
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
 mModelPixmapLabelsLayout = new Q3GridLayout(pixmapsWidget, -1, 3);
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
	KConfig conf(copyrightFile, KConfig::SimpleConfig);
	if (!conf.hasGroup("Copyright")) {
		boWarning() << k_funcinfo << "no Copyright group found in file " << copyrightFile << endl;
	} else {
		KConfigGroup group = conf.group("Copyright");
		d->mGUI->mModelAuthor->setText(group.readEntry("Author", QString()));
		d->mGUI->mModelLicense->setText(group.readEntry("License", QString()));
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
 for (QStringList::iterator it = textures.begin(); it != textures.end(); ++it) {
	BoTexture* tex = BosonModelTextures::modelTextures()->texture(*it);
	bool foundTexture = true;
	QFileInfo textureInfo;
	if (!tex) {
		foundTexture = false;
	} else {
		textureInfo.setFile(tex->filePath());
		if (!textureInfo.exists()) {
			foundTexture = false;
		}
	}

	if (!foundTexture) {
		notFound.append(*it);
		boDebug() << k_funcinfo << "texture not found: " << *it << endl;
	} else {
		found.append(*it);
		addTextureCopyright(textureInfo.dirPath() + "/" + *it);
	}
 }
 d->mGUI->mTexturesFoundList->insertStringList(found);
 d->mGUI->mTexturesNotFoundList->insertStringList(notFound);
}

void BoModelPixmaps::slotSelectTextureDirectory()
{
 QString old = BosonModelTextures::modelTextures()->additionalTexturePath();
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
 BosonModelTextures::modelTextures()->setAdditionalTexturePath(dir);
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
 for (int i = 0; i < mModelPixmapLabels.count(); i++) {
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

 Q3ValueList<BoVector3Float> cameraPosList;
 Q3ValueList<BoVector3Float> lookAtList;
 Q3ValueList<BoVector3Float> upList;
 Q3ValueList<QString> namesList;

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

 for (int i = 0; i < namesList.count(); i++) {
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

 for (int i = 0; i < cameraPosList.count(); i++) {
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
	img = img.smoothScale(PIXMAP_THUMBNAIL_WIDTH, PIXMAP_THUMBNAIL_HEIGHT, Qt::ScaleMin);
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
	Emin.setX(qMin(Emin.x(), E[i].x()));
	Emin.setY(qMin(Emin.y(), E[i].y()));
	Emin.setZ(qMin(Emin.z(), E[i].z()));
	Emax.setX(qMax(Emax.x(), E[i].x()));
	Emax.setY(qMax(Emax.y(), E[i].y()));
	Emax.setZ(qMax(Emax.z(), E[i].z()));
 }
 //boDebug() << "min: (" << Emin.x() << "; " << Emin.y() << "; " << Emin.z() << ")" << endl;
 //boDebug() << "max: (" << Emax.x() << "; " << Emax.y() << "; " << Emax.z() << ")" << endl;

 BoVector3Float Emid = (Emin + Emax) / 2;

 // Change the projection matrix
 float scale = qMin(2 / (Emax.x() - Emin.x()),  2 / (Emax.y() - Emin.y()));
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
	KConfig conf(copyrightFile, KConfig::SimpleConfig);
	if (!conf.hasGroup("Copyright")) {
		boWarning() << k_funcinfo << "no Copyright group found in file " << copyrightFile << endl;
	} else {
		KConfigGroup group = conf.group("Copyright");
		copyrightWidget->setAuthor(group.readEntry("Author", QString()));
		copyrightWidget->setLicense(group.readEntry("License", QString()));
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
 for (Q3PtrListIterator<BoTextureCopyright> it(mTextureCopyright); it.current(); ++it) {
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
 if (!tar.open(QIODevice::WriteOnly)) {
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
 if (!file.open(QIODevice::ReadOnly)) {
	KMessageBox::sorry(this, i18n("Could not open %1 for reading").arg(file.name()));
	return;
 }
 QByteArray fileData = file.readAll();
 tar.writeFile(mainDir + QFileInfo(file).fileName(), user, group, fileData.data(), fileData.size());
 file.close();

 QByteArray modelCopyrightData;
 Q3TextStream modelCopyrightStream(modelCopyrightData, QIODevice::WriteOnly);
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
 tar.writeFile(mainDir + QFileInfo(file).baseName() + ".copyright", user, group, modelCopyrightData.data(), modelCopyrightData.size());

 QStringList textureLicenses;
 for (Q3PtrListIterator<BoTextureCopyright> it(mTextureCopyright); it.current(); ++it) {
	QString texture = it.current()->texture();
	file.setName(texture);
	if (!file.open(QIODevice::ReadOnly)) {
		KMessageBox::sorry(this, i18n("Could not open %1 for reading").arg(file.name()));
		return;
	}
	QByteArray fileData = file.readAll();
	tar.writeFile(mainDir + "textures/" + QFileInfo(file).fileName(), user, group, fileData.data(), fileData.size());
	file.close();

	QByteArray copyright;
	Q3TextStream s(copyright, QIODevice::WriteOnly);
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
	tar.writeFile(mainDir + "textures/" + QFileInfo(file).baseName() + ".copyright", user, group, copyright.data(), copyright.size());
 }

 for (Q3PtrListIterator<BoModelPixmapCollection> it(mModelPixmaps); it.current(); ++it) {
	QString name = it.current()->name();
	QPixmap p = it.current()->pixmap();
	QPixmap thumb = it.current()->thumbnail();

	QByteArray pixmapData;
	QBuffer pixmapBuffer(&pixmapData);
	pixmapBuffer.open(QIODevice::WriteOnly);
	if (!p.save(&pixmapBuffer, "JPEG", 75)) {
		KMessageBox::sorry(this, i18n("An image could not be saved"));
		return;
	}
	pixmapBuffer.close();

	QByteArray thumbnailData;
	QBuffer thumbnailBuffer(&thumbnailData);
	thumbnailBuffer.open(QIODevice::WriteOnly);
	if (!thumb.save(&thumbnailBuffer, "JPEG", 75)) {
		KMessageBox::sorry(this, i18n("A thumbnail could not be saved"));
		return;
	}
	thumbnailBuffer.close();

	tar.writeFile(mainDir + "images/" + name + ".jpg", user, group, pixmapData.data(), pixmapData.size());
	tar.writeFile(mainDir + "thumbnails/" + name + ".jpg", user, group, thumbnailData.data(), thumbnailData.size());
 }

 QByteArray summaryData;
 Q3TextStream summary(summaryData, QIODevice::WriteOnly);
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

 tar.writeFile(mainDir + "summary.txt", user, group, summaryData.data(), summaryData.size());

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
		QByteArray(),
		ki18n("Boson Model Pixmaps"),
		version,
		description,
		KAboutData::License_GPL,
		ki18n("(C) 2005 Andreas Beckermann"),
		KLocalizedString(),
		"http://boson.eu.org");
 about.addAuthor( ki18n("Andreas Beckermann"), ki18n("Coding & Current Maintainer"), "b_mann@gmx.de" );

 KCmdLineOptions options;
 options.add("package", ki18n("Automatically package all files loaded from cmd line"));
 options.add("+[files]", ki18n("Files for --package"));
 options.add("xrotation <rotation>", ki18n("Initial X rotation"));
 options.add("yrotation <rotation>", ki18n("Initial Y rotation"));
 options.add("zrotation <rotation>", ki18n("Initial Z rotation"));

 // we need to do extra stuff after BosonConfig's initialization
 BosonConfig::setPostInitFunction(&postBosonConfigInit);

 Q3CString argv0(argv[0]);
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
		main->selectModelFile(QFile::decodeName(args->arg(i).toLatin1()));
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
