/*
    This file is part of the Boson game
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosoncursoreditor.h"
#include "bosoncursoreditor.moc"

#include "../bomemory/bodummymemory.h"
#include "global.h"
#include "bosoncursor.h"
#include "bodebug.h"
#include "bofiledialog.h"

#include <KConfig>
#include <KConfigGroup>
#include <klocale.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <kglobal.h>

#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <q3hbox.h>
#include <q3vbox.h>
#include <qcombobox.h>
#include <q3vgroupbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qdir.h>
//Added by qt3to4:
#include <Q3VBoxLayout>


// FIXME: rename -> OpenGLConfig or so
SpriteConfig::SpriteConfig(QWidget* w) : Q3VGroupBox(i18n("OpenGL Cursor"), w)
{
 Q3HBox* hbox = new Q3HBox(this);
 (void)new QLabel(i18n("File Prefix:"), hbox);
 mFilePrefix = new QLineEdit(hbox);

 hbox = new Q3HBox(this);
 (void)new QLabel(i18n("HotspotX"), hbox);
 mHotspotX = new KIntNumInput(0, hbox);
 mHotspotX->setRange(0, 100);

 hbox = new Q3HBox(this);
 (void)new QLabel(i18n("HotspotY"), hbox);
 mHotspotY = new KIntNumInput(0, hbox);
 mHotspotY->setRange(0, 100);

 hbox = new Q3HBox(this);
 (void)new QLabel(i18n("Animated"), hbox);
 mIsAnimated = new QCheckBox(hbox);

 mAnimationSettings = new Q3VBox(this);
 connect(mIsAnimated, SIGNAL(toggled(bool)), mAnimationSettings, SLOT(setEnabled(bool)));

 hbox = new Q3HBox(mAnimationSettings);
 (void)new QLabel(i18n("Animation Speed (ms)"), hbox);
 mAnimationSpeed= new KIntNumInput(0, hbox);
 mAnimationSpeed->setRange(0, 2000);

 hbox = new Q3HBox(mAnimationSettings);
 (void)new QLabel(i18n("FrameCount"), hbox);
 mFrameCount = new KIntNumInput(1, hbox);
 mFrameCount->setRange(1, 100);

 hbox = new Q3HBox(mAnimationSettings);
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
 boDebug() << k_funcinfo << file << endl;
 KConfig cfg(file, KConfig::SimpleConfig);
 if (!cfg.hasGroup("Boson Cursor")) {
	boError() << k_funcinfo << file << " has no Boson Cursor group" << endl;
	return;
 }
 KConfigGroup group = cfg.group("Boson Cursor");
 unsigned int hotspotX = group.readEntry("HotspotX", (unsigned int)0);
 unsigned int hotspotY = group.readEntry("HotspotY", (unsigned int)0);
 QString prefix = group.readEntry("FilePrefix", QString("Cursor-"));
 bool animated = group.readEntry("IsAnimated", false);
 unsigned int speed = 0;
 unsigned int frames = 1;
 int rotateDegree = 0;
 if (animated) {
	KConfigGroup animationGroup = cfg.group("Animation");
	speed = animationGroup.readEntry("Speed", (unsigned int)0);
	frames = animationGroup.readEntry("FrameCount", (unsigned int)1);
	rotateDegree = animationGroup.readEntry("RotateDegree", (int)0);
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
 boDebug() << k_funcinfo << file << endl;
 KConfig cfg(file, KConfig::SimpleConfig);
 KConfigGroup group = cfg.group("Boson Cursor");
 group.writeEntry("HotspotX", (unsigned int)mHotspotX->value());
 group.writeEntry("HotspotY", (unsigned int)mHotspotY->value());
 group.writeEntry("FilePrefix", mFilePrefix->text());
 group.writeEntry("IsAnimated", mIsAnimated->isChecked());
 if (mIsAnimated->isChecked()) {
	KConfigGroup animationGroup = cfg.group("Animation");
	animationGroup.writeEntry("Speed", (unsigned int)mAnimationSpeed->value());
	animationGroup.writeEntry("FrameCount", (unsigned int)mFrameCount->value());
	animationGroup.writeEntry("RotateDegree", (int)mRotateDegree->value());
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
 Q3VBoxLayout* vLayout = new Q3VBoxLayout(this);
 vLayout->setAutoAdd(true);
 Q3HBox* hbox = new Q3HBox(this);
 (void)new QLabel(i18n("Base Directory"), hbox);
 mBaseDirectory = new QPushButton(hbox);
 connect(mBaseDirectory, SIGNAL(clicked()), this, SLOT(slotChangeBaseDirectory()));

 hbox = new Q3HBox(this);
 (void)new QLabel(i18n("Cursor Mode"), hbox);
 mCursorMode = new QComboBox(hbox);
 mCursorMode->insertItem(i18n("OpenGL Cursor"), CursorOpenGL);
 mCursorMode->insertItem(i18n("KDE Standard Cursor"), CursorKDE);
 connect(mCursorMode, SIGNAL(activated(int)), this, SLOT(slotCursorModeChanged(int)));

 hbox = new Q3HBox(this);
 (void)new QLabel(i18n("Cursor Type"), hbox);
 mCursorType = new QComboBox(hbox);
 connect(mCursorType, SIGNAL(activated(int)), this, SLOT(slotCursorTypeChanged(int)));

 hbox = new Q3HBox(this);
 (void)new QLabel(i18n("Cursor Theme"), hbox);
 mCursorTheme = new QComboBox(hbox);
 connect(mCursorTheme, SIGNAL(activated(int)),
		this, SLOT(slotCursorThemeChanged(int)));

 mSpriteConfig = new SpriteConfig(this);
 connect(mSpriteConfig, SIGNAL(apply()), this, SLOT(slotApplySpriteConfig()));
}

void BosonCursorEditor::loadInitialCursor()
{
 QString defaultTheme = BosonCursor::defaultTheme();
 if (defaultTheme.isEmpty()) {
	boError() << k_funcinfo << "no default theme available" << endl;
	return;
 }
 if (defaultTheme[defaultTheme.length() - 1] == '/') {
	defaultTheme = defaultTheme.left(defaultTheme.length() - 1);
 }
 int i = defaultTheme.findRev('/');
 if (i < 0) {
		boError() << k_funcinfo << "no / found in cursor dir \"" << defaultTheme << "\", cannot be a valid directory" << endl;
	return;
 }
 QString dir = defaultTheme.left(i);
 setCursor(CursorKDE);
 mSpriteConfig->setEnabled(false);
 changeBaseDirectory(dir);
}

QStringList BosonCursorEditor::findCursorThemes(const QString& directory)
{
 boDebug() << k_funcinfo << directory << endl;
 QDir dir(directory);
 QStringList subdirs = dir.entryList(QDir::Dirs);
 subdirs.remove(QString::fromLatin1("."));
 subdirs.remove(QString::fromLatin1("..")); // umm.. iirc there was a function to do this automatically... can't remember it :(
 QStringList list;
 for (int i = 0; i < subdirs.count(); i++) {
	QString path = dir.absPath() + QString::fromLatin1("/") + subdirs[i];
	if (QFile::exists(path + QString::fromLatin1("/index.cursor"))) {
		list.append(path);
	}
 }
 return list;
}

void BosonCursorEditor::loadSpriteConfig(const QString& dir)
{
 QString file(dir + QString::fromLatin1("/index.cursor"));
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
 KConfig cfg(path + type + QString::fromLatin1("/index.cursor"), KConfig::SimpleConfig);
 if (!cfg.hasGroup("Boson Cursor")) {
	boWarning() << k_funcinfo << "Invalid index.cursor file for " << path + type << endl;
	name += QString::fromLatin1(" (Invalid index.cursor)");
 }
 mCursorType->insertItem(name);
}

void BosonCursorEditor::slotCursorTypeChanged(int index)
{
 boDebug() << k_funcinfo << index << endl;
 if (index < 0 || index > (int)mCursorTypes.count()) {
	boError() << k_funcinfo << "index out of range: " << index << endl;
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
 QString file = mCursorTypes[mCursorType->currentItem()] + QString::fromLatin1("/index.cursor");
 mSpriteConfig->save(file);
 int mode = mCursorMode->currentItem();
 QString theme = mCursorThemes[mCursorTheme->currentItem()];
 emit signalCursorChanged(mode, theme);
 slotCursorTypeChanged(mCursorType->currentItem());
}

void BosonCursorEditor::slotChangeBaseDirectory()
{
 QString dir = BoFileDialog::getExistingDirectory();
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
	KConfig cfg(themes[i] + QString::fromLatin1("/index.cursor"), KConfig::SimpleConfig);
	if (!cfg.hasGroup("Boson Cursor")) {
		boWarning() << "invalid cursor " << themes[i] << endl;
	} else {
		KConfigGroup group = cfg.group("Boson Cursor");
		QString name = group.readEntry("Name", i18n("Unknown"));
		mCursorTheme->insertItem(name);
		mCursorThemes.append(themes[i]);
	}
 }

 slotCursorThemeChanged(0);
}

