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

#include "bosoncursoreditor.h"
#include "bosoncursoreditor.moc"

#include "global.h"
#include "bosoncursor.h"

#include <ksimpleconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kfiledialog.h>

#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qcombobox.h>
#include <qvgroupbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qdir.h>


// FIXME: rename -> OpenGLConfig or so
SpriteConfig::SpriteConfig(QWidget* w) : QVGroupBox(i18n("OpenGL Cursor"), w)
{
 QHBox* hbox = new QHBox(this);
 (void)new QLabel(i18n("File Prefix:"), hbox);
 mFilePrefix = new QLineEdit(hbox);

 hbox = new QHBox(this);
 (void)new QLabel(i18n("HotspotX"), hbox);
 mHotspotX = new KIntNumInput(0, hbox);
 mHotspotX->setRange(0, 100);

 hbox = new QHBox(this);
 (void)new QLabel(i18n("HotspotY"), hbox);
 mHotspotY = new KIntNumInput(0, hbox);
 mHotspotY->setRange(0, 100);

 hbox = new QHBox(this);
 (void)new QLabel(i18n("Animated"), hbox);
 mIsAnimated = new QCheckBox(hbox);

 mAnimationSettings = new QVBox(this);
 connect(mIsAnimated, SIGNAL(toggled(bool)), mAnimationSettings, SLOT(setEnabled(bool)));

 hbox = new QHBox(mAnimationSettings);
 (void)new QLabel(i18n("Animation Speed (ms)"), hbox);
 mAnimationSpeed= new KIntNumInput(0, hbox);
 mAnimationSpeed->setRange(0, 2000);

 hbox = new QHBox(mAnimationSettings);
 (void)new QLabel(i18n("FrameCount"), hbox);
 mFrameCount = new KIntNumInput(1, hbox);
 mFrameCount->setRange(1, 100);

 hbox = new QHBox(mAnimationSettings);
 (void)new QLabel(i18n("Rotate (degree)"), hbox);
 mRotateDegree = new KIntNumInput(0, hbox);
 mRotateDegree->setRange(-360, 360);

 QPushButton* apply = new QPushButton(i18n("&Apply"), this);
 connect(apply, SIGNAL(clicked()), this, SIGNAL(apply()));
}

SpriteConfig::~SpriteConfig()
{
}

void SpriteConfig::load(const QString& file)
{
 kdDebug() << k_funcinfo << file << endl;
 KSimpleConfig cfg(file);
 if (!cfg.hasGroup("Boson Cursor")) {
	kdError() << k_funcinfo << file << " has no Boson Cursor group" << endl;
	return;
 }
 cfg.setGroup("Boson Cursor");
 unsigned int hotspotX = cfg.readUnsignedNumEntry("HotspotX", 0);
 unsigned int hotspotY = cfg.readUnsignedNumEntry("HotspotY", 0);
 QString prefix = cfg.readEntry("FilePrefix", QString("Cursor-"));
 bool animated = cfg.readBoolEntry("IsAnimated", false);
 unsigned int speed = 0;
 unsigned int frames = 1;
 int rotateDegree = 0;
 if (animated) {
	cfg.setGroup("Animation");
	speed = cfg.readUnsignedNumEntry("Speed", 0);
	frames = cfg.readUnsignedNumEntry("FrameCount", 1);
	rotateDegree = cfg.readNumEntry("RotateDegree", 0);
 }
 mHotspotX->setValue(hotspotX);
 mHotspotY->setValue(hotspotY);
 mFilePrefix->setText(prefix);
 mIsAnimated->setChecked(animated);
 mAnimationSettings->setEnabled(animated);
 mAnimationSpeed->setValue(speed);
 mFrameCount->setValue(frames);
 mRotateDegree->setValue(rotateDegree);
}

void SpriteConfig::save(const QString& file)
{
 kdDebug() << k_funcinfo << file << endl;
 KSimpleConfig cfg(file);
 cfg.setGroup("Boson Cursor");
 cfg.writeEntry("HotspotX", (unsigned int)mHotspotX->value());
 cfg.writeEntry("HotspotY", (unsigned int)mHotspotY->value());
 cfg.writeEntry("FilePrefix", mFilePrefix->text());
 cfg.writeEntry("IsAnimated", mIsAnimated->isChecked());
 if (mIsAnimated->isChecked()) {
	cfg.setGroup("Animation");
	cfg.writeEntry("Speed", (unsigned int)mAnimationSpeed->value());
	cfg.writeEntry("FrameCount", (unsigned int)mFrameCount->value());
	cfg.writeEntry("RotateDegree", (int)mRotateDegree->value());
 }
}


BosonCursorEditor::BosonCursorEditor(QWidget* parent) : QWidget(parent)
{
 init();
}

BosonCursorEditor::~BosonCursorEditor()
{
}

void BosonCursorEditor::init()
{
 QVBoxLayout* vLayout = new QVBoxLayout(this);
 vLayout->setAutoAdd(true);
 QHBox* hbox = new QHBox(this);
 (void)new QLabel(i18n("Base Directory"), hbox);
 mBaseDirectory = new QPushButton(hbox);
 connect(mBaseDirectory, SIGNAL(clicked()), this, SLOT(slotChangeBaseDirectory()));

 hbox = new QHBox(this);
 (void)new QLabel(i18n("Cursor Mode"), hbox);
 mCursorMode = new QComboBox(hbox);
 mCursorMode->insertItem(i18n("OpenGL Cursor"), CursorOpenGL);
 mCursorMode->insertItem(i18n("B/W Cursor"), CursorNormal);
 mCursorMode->insertItem(i18n("KDE Standard Cursor"), CursorKDE);
 connect(mCursorMode, SIGNAL(activated(int)), this, SLOT(slotCursorModeChanged(int)));

 hbox = new QHBox(this);
 (void)new QLabel(i18n("Cursor Type"), hbox);
 mCursorType = new QComboBox(hbox);
 connect(mCursorType, SIGNAL(activated(int)), this, SLOT(slotCursorTypeChanged(int)));

 hbox = new QHBox(this);
 (void)new QLabel(i18n("Cursor Theme"), hbox);
 mCursorTheme = new QComboBox(hbox);
 connect(mCursorTheme, SIGNAL(activated(int)),
		this, SLOT(slotCursorThemeChanged(int)));

 mSpriteConfig = new SpriteConfig(this);
 connect(mSpriteConfig, SIGNAL(apply()), this, SLOT(slotApplySpriteConfig()));
}

void BosonCursorEditor::loadInitialCursor()
{
 QString dir = KGlobal::dirs()->findResourceDir("data", "boson/themes/cursors/default/index.desktop"); // there must be a cleaner way!
 dir += QString::fromLatin1("boson/themes/cursors/");
 setCursor(CursorKDE);
 mSpriteConfig->setEnabled(false);
 changeBaseDirectory(dir);
}

QStringList BosonCursorEditor::findCursorThemes(const QString& directory)
{
 kdDebug() << k_funcinfo << directory << endl;
 QDir dir(directory);
 QStringList subdirs = dir.entryList(QDir::Dirs);
 subdirs.remove(QString::fromLatin1("."));
 subdirs.remove(QString::fromLatin1("..")); // umm.. iirc there was a function to do this automatically... can't remember it :(
 QStringList list;
 for (unsigned int i = 0; i < subdirs.count(); i++) {
	QString path = dir.absPath() + QString::fromLatin1("/") + subdirs[i];
	if (QFile::exists(path + QString::fromLatin1("/index.desktop"))) {
		list.append(path);
	}
 }
 return list;
}

void BosonCursorEditor::loadSpriteConfig(const QString& dir)
{
 QString file(dir + QString::fromLatin1("/index.desktop"));
 mSpriteConfig->load(file);
}

void BosonCursorEditor::slotCursorThemeChanged(int index)
{
 if (index < 0) {
	return;
 }

 QString theme = mCursorThemes[index];
 int mode;
 if (mCursorMode->currentItem() >= 0) {
	mode = mCursorMode->currentItem();
 } else {
	mode = CursorKDE;
 }

 // update the types since we changed the theme
 mCursorType->clear();
 mCursorTypes.clear();
 // warning! the order here is important! look at CursorType enum in global.h
 addType(theme, "attack");
 addType(theme, "move");
 addType(theme, "default");
// mCursorType->setCurrentItem(CursorDefault);

 emit signalCursorChanged(mode, theme);
 slotCursorTypeChanged(mCursorType->currentItem());
}

void BosonCursorEditor::slotCursorModeChanged(int index)
{
 if (index < 0) {
	return;
 }
 QString theme;
 if (mCursorTheme->currentItem() >= 0) {
	theme = mCursorThemes[mCursorTheme->currentItem()];
 } else {
	theme = BosonCursor::defaultTheme();
 }
 emit signalCursorChanged(index, theme);
 mSpriteConfig->setEnabled(index == CursorOpenGL);
 slotCursorTypeChanged(mCursorType->currentItem());
}

void BosonCursorEditor::addType(const QString& theme, const QString& type)
{
 QString path = theme;
 if (path.right(1) != QString::fromLatin1("/")) {
	path += QString::fromLatin1("/");
 }
 mCursorTypes.append(path + type);

 QString name = type;
 KSimpleConfig cfg(path + type + QString::fromLatin1("/index.desktop"));
 if (!cfg.hasGroup("Boson Cursor")) {
	kdWarning() << k_funcinfo << "Invalid index.desktop file for " << path + type << endl;
	name += QString::fromLatin1(" (Invalid index.desktop)");
 }
 mCursorType->insertItem(name);
}

void BosonCursorEditor::slotCursorTypeChanged(int index)
{
 kdDebug() << k_funcinfo << index << endl;
 if (index < 0 || index > (int)mCursorTypes.count()) {
	kdError() << k_funcinfo << "index out of range: " << index << endl;
	return;
 }
 QString name = mCursorTypes[index];
 if (mCursorMode->currentItem() == CursorOpenGL) {
	loadSpriteConfig(name);
 }

 emit signalCursorTypeChanged(index);
}

void BosonCursorEditor::setCursor(int mode)
{
 mCursorMode->setCurrentItem(mode);
}


void BosonCursorEditor::slotApplySpriteConfig()
{
 QString file = mCursorTypes[mCursorType->currentItem()] + QString::fromLatin1("/index.desktop");
 mSpriteConfig->save(file);
 int mode = mCursorMode->currentItem();
 QString theme = mCursorThemes[mCursorTheme->currentItem()];
 emit signalCursorChanged(mode, theme);
 slotCursorTypeChanged(mCursorType->currentItem());
}

void BosonCursorEditor::slotChangeBaseDirectory()
{
 QString dir = KFileDialog::getExistingDirectory();
 if (dir != QString::null) {
	changeBaseDirectory(dir);
 }
}

void BosonCursorEditor::changeBaseDirectory(const QString& dir)
{
 QStringList themes = findCursorThemes(dir);
 if (themes.count() == 0) {
	return;
 }
 mCursorTheme->clear();
 mCursorThemes.clear();
 mCursorType->clear();
 mCursorTypes.clear();
 mBaseDirectory->setText(dir);
 for (int i = 0; i < (int)themes.count(); i++) {
	KSimpleConfig cfg(themes[i] + QString::fromLatin1("/index.desktop"));
	if (!cfg.hasGroup("Boson Cursor")) {
		kdWarning() << "invalid cursor " << themes[i] << endl;
	} else {
		cfg.setGroup("Boson Cursor");
		QString name = cfg.readEntry("Name", i18n("Unknown"));
		mCursorTheme->insertItem(name);
		mCursorThemes.append(themes[i]);
	}
 }

 slotCursorThemeChanged(0);
}

