/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "optionswidgets.h"
#include "bosonconfig.h"

#include "bosoncursor.h"
#include "bosonmodel.h"
#include "bosontexturearray.h"
#include "boson.h"
#include "defines.h"
#include "bodebug.h"

#include <klocale.h>
#include <knuminput.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qtooltip.h>

#include "optionswidgets.moc"

OptionsWidget::OptionsWidget()
{
 mGame = 0;
 mPlayer = 0;
}

OptionsWidget::~OptionsWidget()
{
}


//////////////////////////////////////////////////////////////////////
// General Options
//////////////////////////////////////////////////////////////////////

GeneralOptions::GeneralOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 mGameSpeed = new KIntNumInput(DEFAULT_GAME_SPEED, this);
 mGameSpeed->setRange(MIN_GAME_SPEED, MAX_GAME_SPEED);
 mGameSpeed->setLabel(i18n("Game speed"));

 mMiniMapScale = new KDoubleNumInput(DEFAULT_MINIMAP_SCALE, this);
 mMiniMapScale->setRange(1.0, 5.0, 1);
 mMiniMapScale->setLabel(i18n("Mini Map scale factor"));


 QHBox* hbox = new QHBox(this);
 (void)new QLabel(i18n("Command frame background pixmap"), hbox);
 mCmdBackground = new QComboBox(hbox);
 mCmdBackgrounds = KGlobal::dirs()->findAllResources("data", "boson/themes/ui/*/cmdpanel*.png");
 mCmdBackground->insertItem(i18n("None"));
 //TODO: display filename only... - not the complete path
 mCmdBackground->insertStringList(mCmdBackgrounds);
}

GeneralOptions::~GeneralOptions()
{
}

void GeneralOptions::apply()
{
 if (!game()) {
	boError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (mGameSpeed->value() != game()->gameSpeed()) {
	game()->slotSetGameSpeed(mGameSpeed->value());
 }
 emit signalMiniMapScaleChanged(mMiniMapScale->value());
 QString file;
 if (mCmdBackground->currentItem() > 0) {
	file = mCmdBackgrounds[mCmdBackground->currentItem() - 1];
 }
 emit signalCmdBackgroundChanged(file);
}

void GeneralOptions::load()
{
 if (!game()) {
	boError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 setGameSpeed(game()->gameSpeed());
 setMiniMapScale(boConfig->miniMapScale());
 // TODO: cmdbackground
}

void GeneralOptions::setDefaults()
{
 setGameSpeed(DEFAULT_GAME_SPEED);
 setMiniMapScale(DEFAULT_MINIMAP_SCALE);
 setCmdBackground(QString::null);
}

void GeneralOptions::setGameSpeed(int ms)
{
 mGameSpeed->setValue(ms);
}

void GeneralOptions::setMiniMapScale(double scale)
{
 mMiniMapScale->setValue(scale);
}

void GeneralOptions::setCmdBackground(const QString& file)
{
 if (file.isEmpty() || mCmdBackgrounds.findIndex(file) < 0) {
	mCmdBackground->setCurrentItem(0);
 } else {
	int index = mCmdBackgrounds.findIndex(file);
	mCmdBackground->setCurrentItem(index + 1);
 }
}


//////////////////////////////////////////////////////////////////////
// Cursor Options
//////////////////////////////////////////////////////////////////////

CursorOptions::CursorOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 QHBox* hbox = new QHBox(this);
 (void)new QLabel(i18n("Cursor"), hbox);
 mCursor = new QComboBox(hbox);
 mCursor->insertItem(i18n("OpenGL Cursor"), CursorOpenGL);
 mCursor->insertItem(i18n("KDE Standard Cursor"), CursorKDE);

 hbox = new QHBox(this);
 (void)new QLabel(i18n("Cursor theme"), hbox);
 mCursorTheme = new QComboBox(hbox);
 QStringList list = BosonCursor::availableThemes();
 for (int i = 0; i < (int)list.count(); i++) {
	KSimpleConfig cfg(list[i] + QString::fromLatin1("/index.cursor"));
	if (!cfg.hasGroup("Boson Cursor")) {
		boWarning() << "invalid cursor " << list[i] << endl;
	} else {
		cfg.setGroup("Boson Cursor");
		QString name = cfg.readEntry("Name", i18n("Unknown"));
		mCursorTheme->insertItem(name);
		mCursorThemes.append(list[i]);
	}
 }
}

CursorOptions::~CursorOptions()
{
}

void CursorOptions::apply()
{
 int mode = mCursor->currentItem();
 if (mode < 0) {
	boWarning() << k_funcinfo << "Invalid cursor mode " << mode << endl;
	return;
 }
 QString theme;
 if (mCursorTheme->currentItem() >= 0) {
	theme = mCursorThemes[mCursorTheme->currentItem()];
 } else {
	theme = BosonCursor::defaultTheme();
 }
 emit signalCursorChanged(mode, theme);
}

void CursorOptions::setDefaults()
{
 setCursor(DEFAULT_CURSOR);
 mCursorTheme->setCurrentItem(0);
}

void CursorOptions::load()
{
#warning TODO
}

void CursorOptions::setCursor(CursorMode mode)
{
 mCursor->setCurrentItem(mode);
}


//////////////////////////////////////////////////////////////////////
// Scrolling Options
// "Scrolling" applies to both arrow scrolling and mouse scrolling!
//////////////////////////////////////////////////////////////////////

ScrollingOptions::ScrollingOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 QHBox* hbox = new QHBox(this);
 (void)new QLabel(i18n("Enable right mouse button scrolling"), hbox);
 mRMBScrolling = new QCheckBox(hbox);

 hbox = new QHBox(this);
 (void)new QLabel(i18n("Enable middle mouse button scrolling"), hbox);
 mMMBScrolling = new QCheckBox(hbox);

 hbox = new QHBox(this);
 (void)new QLabel(i18n("Sensity of cursor at edge of the window scrolling (0 for disabled)"), hbox);
 mCursorEdgeSensity = new KIntNumInput(hbox);
 mCursorEdgeSensity->setRange(0, 50);

 mArrowSpeed = new KIntNumInput(DEFAULT_ARROW_SCROLL_SPEED, this);
 mArrowSpeed->setRange(1, 200);
 mArrowSpeed->setLabel(i18n("Arrow key steps"));

}

ScrollingOptions::~ScrollingOptions()
{
}

void ScrollingOptions::apply()
{
 boConfig->setRMBMove(mRMBScrolling->isChecked());
 boConfig->setMMBMove(mMMBScrolling->isChecked());
 if (mCursorEdgeSensity->value() < 0) {
	mCursorEdgeSensity->setValue(0);
 }
 boConfig->setCursorEdgeSensity(mCursorEdgeSensity->value());
 if (mArrowSpeed->value() < 0) {
	mArrowSpeed->setValue(0);
 }
 boConfig->setArrowKeyStep(mArrowSpeed->value());
}

void ScrollingOptions::setDefaults()
{
 setArrowScrollSpeed(DEFAULT_ARROW_SCROLL_SPEED);
 setCursorEdgeSensity(DEFAULT_CURSOR_EDGE_SENSITY);
 setRMBScrolling(DEFAULT_USE_RMB_MOVE);
 setMMBScrolling(DEFAULT_USE_MMB_MOVE);
}

void ScrollingOptions::load()
{
 setArrowScrollSpeed(boConfig->arrowKeyStep());
 setCursorEdgeSensity(boConfig->cursorEdgeSensity());
 setRMBScrolling(boConfig->rmbMove());
 setMMBScrolling(boConfig->mmbMove());
}

void ScrollingOptions::setCursorEdgeSensity(int s)
{
 mCursorEdgeSensity->setValue(s);
}

void ScrollingOptions::setArrowScrollSpeed(int s)
{
 mArrowSpeed->setValue(s);
}

void ScrollingOptions::setRMBScrolling(bool on)
{
 mRMBScrolling->setChecked(on);
}

void ScrollingOptions::setMMBScrolling(bool on)
{
 mMMBScrolling->setChecked(on);
}


//////////////////////////////////////////////////////////////////////
// Sound Options
//////////////////////////////////////////////////////////////////////

SoundOptions::SoundOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 (void)new QLabel(i18n("Disable the following unit sounds (please send a bug report if you can think of more descriptive names):"), this);
 QCheckBox* c;
 c = new QCheckBox(i18n("Shoot"), this);
 mCheckBox2UnitSoundEvent.insert(c, SoundShoot);
 c = new QCheckBox(i18n("Order Move"), this);
 mCheckBox2UnitSoundEvent.insert(c, SoundOrderMove);
 c = new QCheckBox(i18n("Order Attack"), this);
 mCheckBox2UnitSoundEvent.insert(c, SoundOrderAttack);
 c = new QCheckBox(i18n("Order Select"), this);
 mCheckBox2UnitSoundEvent.insert(c, SoundOrderSelect);
 c = new QCheckBox(i18n("Report Produced"), this);
 mCheckBox2UnitSoundEvent.insert(c, SoundReportProduced);
 c = new QCheckBox(i18n("Report Destroyed"), this);
 mCheckBox2UnitSoundEvent.insert(c, SoundReportDestroyed);
 c = new QCheckBox(i18n("Report Under Attack"), this);
 mCheckBox2UnitSoundEvent.insert(c, SoundReportUnderAttack);
}

SoundOptions::~SoundOptions()
{
}

void SoundOptions::apply()
{
 QMap<QCheckBox*, UnitSoundEvent>::Iterator it = mCheckBox2UnitSoundEvent.begin();
 for (; it != mCheckBox2UnitSoundEvent.end(); ++it) {
	boConfig->setUnitSoundActivated(it.data(), !it.key()->isChecked());
 }
}

void SoundOptions::setDefaults()
{
 QMap<QCheckBox*, UnitSoundEvent>::Iterator it = mCheckBox2UnitSoundEvent.begin();
 for (; it != mCheckBox2UnitSoundEvent.end(); ++it) {
	it.key()->setChecked(false);
 }
}

void SoundOptions::load()
{
 setUnitSoundsDeactivated(boConfig);
}

void SoundOptions::setUnitSoundsDeactivated(BosonConfig* conf)
{
 if (!conf) {
	return;
 }
 QMap<QCheckBox*, UnitSoundEvent>::Iterator it = mCheckBox2UnitSoundEvent.begin();
 for (; it != mCheckBox2UnitSoundEvent.end(); ++it) {
	it.key()->setChecked(!boConfig->unitSoundActivated(it.data()));
 }
}


//////////////////////////////////////////////////////////////////////
// OpenGL Options
//////////////////////////////////////////////////////////////////////

OpenGLOptions::OpenGLOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 // AB: most entries here are very technical (especially the texture filters).
 // put them into a separate widget which can be reached using a "advanced"
 // button. then add some default options (i.e. best speed, average speed, best
 // quality)
 mUpdateInterval = new KIntNumInput(DEFAULT_UPDATE_INTERVAL, this);
 mUpdateInterval->setRange(1, 100);
 mUpdateInterval->setLabel(i18n("Update interval (low values hurt performance)"));
 QToolTip::add(mUpdateInterval, i18n("The update interval specifies after how many milli seconds the scene gets re-rendered and therefore directly influence the frames per seconds. But low values prevent boson from doing other important tasks and therefore you might end up in a game that takes several seconds until your units react to your commands! 20ms are usually a good value."));
 mModelTexturesMipmaps = new QCheckBox(this);
 mModelTexturesMipmaps->setText(i18n("Use mipmaps for model textures"));
 QToolTip::add(mModelTexturesMipmaps, i18n("With mipmapping disabled units often look ugly - but it consumes less memory and therefore might improve rendering speed."));

 QHBox* hbox = new QHBox(this);
 (void)new QLabel(i18n("Texture Magnification Filter"), hbox);
 mMagnificationFilter = new QComboBox(hbox);
 mMagnificationFilter->insertItem(i18n("Use GL_NEAREST (fastest)"));
 mMagnificationFilter->insertItem(i18n("Use GL_LINEAR (best quality)"));
 QToolTip::add(mMagnificationFilter, i18n("This selects which filter method is used when the textures need to be displayed at a bigger size than they are stored at. This applies to all textures.")); // AB: also note how big the impact on speed is!

 hbox = new QHBox(this);
 (void)new QLabel(i18n("Texture Minification Filter"), hbox);
 mMinificationFilter = new QComboBox(hbox);
 mMinificationFilter->insertItem(i18n("Use GL_NEAREST (fastest)"));
 mMinificationFilter->insertItem(i18n("Use GL_LINEAR (best quality)"));
 QToolTip::add(mMinificationFilter, i18n("This selects which filter method is used when the textures need to be displayed at a smaller size than they are stored at. This applies to not-mipmapped textures only.")); // AB: also note how big the impact on speed is!

 hbox = new QHBox(this);
 (void)new QLabel(i18n("Texture Mipmap Minification Filter"), hbox);
 mMipmapMinificationFilter = new QComboBox(hbox);
 mMipmapMinificationFilter->insertItem(i18n("Use GL_NEAREST (fastest)"));
 mMipmapMinificationFilter->insertItem(i18n("Use GL_LINEAR"));
 mMipmapMinificationFilter->insertItem(i18n("Use GL_NEAREST_MIPMAP_NEAREST"));
 mMipmapMinificationFilter->insertItem(i18n("Use GL_NEAREST_MIPMAP_LINEAR"));
 mMipmapMinificationFilter->insertItem(i18n("Use GL_LINEAR_MIPMAP_NEAREST"));
 mMipmapMinificationFilter->insertItem(i18n("Use GL_LINEAR_MIPMAP_LINEAR (best quality)"));
 QToolTip::add(mMipmapMinificationFilter, i18n("This selects which filter method is used when the textures need to be displayed at a smaller size than they are stored at. This applies to mipmapped textures (i.e. model textures) only.\nNote: The speed GL_*_MIPMAP_* will probably be noticebly slower, but it's quality is way better!"));

 mAlignSelectBoxes = new QCheckBox(this);
 mAlignSelectBoxes->setText(i18n("Align unit selection boxes to camera"));
}

OpenGLOptions::~OpenGLOptions()
{
}

void OpenGLOptions::apply()
{
 bool reloadModelTextures = false;
 bool resetTexParameter = false;
 emit signalUpdateIntervalChanged((unsigned int)mUpdateInterval->value());
 if (boConfig->modelTexturesMipmaps() != mModelTexturesMipmaps->isChecked()) {
	boConfig->setModelTexturesMipmaps(mModelTexturesMipmaps->isChecked());
	reloadModelTextures = true;
 }
 if (boConfig->magnificationFilter() != magnificationFilter()) {
	boConfig->setMagnificationFilter(magnificationFilter());
	resetTexParameter = true;
 }
 if (boConfig->minificationFilter() != minificationFilter()) {
	boConfig->setMinificationFilter(minificationFilter());
	resetTexParameter = true;
 }
 if (boConfig->mipmapMinificationFilter() != mipmapMinificationFilter()) {
	boConfig->setMipmapMinificationFilter(mipmapMinificationFilter());
	// only models use mipmaps (if at all)
	if (boConfig->modelTexturesMipmaps()) {
		resetTexParameter = true;
	}
 }

 if (reloadModelTextures) {
	int r = KMessageBox::questionYesNo(this, i18n("You need to reload the model textures to see your changes. Do you want to reload now (takes some time)?"));
	reloadModelTextures = (r == KMessageBox::Yes);
	if (reloadModelTextures) {
		BosonModel::reloadAllTextures();
	}
 }
 if (resetTexParameter) {
	// maybe display a message box now, asking for permission to reset the
	// parameters. currently we just reset them - leave it at this as long
	// as no problems appear
	if (resetTexParameter) {
		BosonTextureArray::resetAllTexParameter();
	}
 }
 
 boConfig->setAlignSelectionBoxes(mAlignSelectBoxes->isChecked());
}

void OpenGLOptions::setDefaults()
{
 setUpdateInterval(DEFAULT_UPDATE_INTERVAL);
 mModelTexturesMipmaps->setChecked(true);
 setMagnificationFilter(DEFAULT_MAGNIFICATION_FILTER);
 setMinificationFilter(DEFAULT_MINIFICATION_FILTER);
 setMipmapMinificationFilter(DEFAULT_MIPMAP_MINIFICATION_FILTER);
 setAlignSelectionBoxes(DEFAULT_ALIGN_SELECTION_BOXES);
}

void OpenGLOptions::load()
{
 setUpdateInterval(boConfig->updateInterval());
 mModelTexturesMipmaps->setChecked(boConfig->modelTexturesMipmaps());
 setMagnificationFilter(boConfig->magnificationFilter());
 setMinificationFilter(boConfig->minificationFilter());
 setMipmapMinificationFilter(boConfig->mipmapMinificationFilter());
 setAlignSelectionBoxes(boConfig->alignSelectionBoxes());
}

void OpenGLOptions::setUpdateInterval(int ms)
{
 boDebug() << k_funcinfo << ms << endl;
 mUpdateInterval->setValue(ms);
}

void OpenGLOptions::setMagnificationFilter(int f)
{
 switch(f) {
	case GL_NEAREST:
	default:
		mMagnificationFilter->setCurrentItem(0);
		break;
	case GL_LINEAR:
		mMagnificationFilter->setCurrentItem(1);
		break;
 }
}

int OpenGLOptions::magnificationFilter() const
{
 if (mMagnificationFilter->currentItem() == 1) {
	return GL_LINEAR;
 }
 // index is either 0 or -1
 return GL_NEAREST;
}

int OpenGLOptions::minificationFilter() const
{
 if (mMinificationFilter->currentItem() == 1) {
	return GL_LINEAR;
 }
 // index is either 0 or -1
 return GL_NEAREST;
}

void OpenGLOptions::setMinificationFilter(int f)
{
 switch(f) {
	case GL_NEAREST:
	default:
		mMinificationFilter->setCurrentItem(0);
		break;
	case GL_LINEAR:
		mMinificationFilter->setCurrentItem(1);
		break;
 }
}

int OpenGLOptions::mipmapMinificationFilter() const
{
 GLenum e;
 switch (mMipmapMinificationFilter->currentItem()) {
	case 0:
		e = GL_NEAREST;
		break;
	case 1:
		e = GL_LINEAR;
		break;
	default:
	case 2:
		e = GL_NEAREST_MIPMAP_NEAREST;
		break;
	case 3:
		e = GL_NEAREST_MIPMAP_LINEAR;
		break;
	case 4:
		e = GL_LINEAR_MIPMAP_NEAREST;
		break;
	case 5:
		e = GL_LINEAR_MIPMAP_LINEAR;
		break;
 }
 return (int)e;
}

void OpenGLOptions::setMipmapMinificationFilter(int f)
{
 switch(f) {
	case GL_NEAREST:
		mMipmapMinificationFilter->setCurrentItem(0);
		break;
	case GL_LINEAR:
		mMipmapMinificationFilter->setCurrentItem(1);
		break;
	default:
	case GL_NEAREST_MIPMAP_NEAREST:
		mMipmapMinificationFilter->setCurrentItem(2);
		break;
	case GL_NEAREST_MIPMAP_LINEAR:
		mMipmapMinificationFilter->setCurrentItem(3);
		break;
	case GL_LINEAR_MIPMAP_NEAREST:
		mMipmapMinificationFilter->setCurrentItem(4);
		break;
	case GL_LINEAR_MIPMAP_LINEAR:
		mMipmapMinificationFilter->setCurrentItem(5);
		break;
 }
}

void OpenGLOptions::setAlignSelectionBoxes(bool align)
{
 mAlignSelectBoxes->setChecked(align);
}

//////////////////////////////////////////////////////////////////////
// Chat Options
//////////////////////////////////////////////////////////////////////

ChatOptions::ChatOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 QHBox* hbox = new QHBox(this);
 mScreenRemoveTime = new KIntNumInput(DEFAULT_CHAT_SCREEN_REMOVE_TIME, this);
 mScreenRemoveTime->setRange(0, 400);
 mScreenRemoveTime->setLabel(i18n("Remove from screen after seconds (0 to remove never)"));

 hbox = new QHBox(this);
 mScreenMaxItems = new KIntNumInput(DEFAULT_CHAT_SCREEN_REMOVE_TIME, this);
 mScreenMaxItems->setRange(-1, 40);
 mScreenMaxItems->setLabel(i18n("Maximal items on the screen (-1 is unlimited)"));
}

ChatOptions::~ChatOptions()
{
}

void ChatOptions::apply()
{
 boConfig->setChatScreenRemoveTime(mScreenRemoveTime->value());
 boConfig->setChatScreenMaxItems(mScreenMaxItems->value());
}

void ChatOptions::setDefaults()
{
 setScreenRemoveTime(DEFAULT_CHAT_SCREEN_REMOVE_TIME);
 setScreenMaxItems(DEFAULT_CHAT_SCREEN_REMOVE_TIME);
}

void ChatOptions::load()
{
 setScreenRemoveTime(boConfig->chatScreenRemoveTime());
 setScreenMaxItems(boConfig->chatScreenMaxItems());
}

void ChatOptions::setScreenRemoveTime(unsigned int s)
{
 mScreenRemoveTime->setValue(s);
}

void ChatOptions::setScreenMaxItems(int m)
{
 mScreenMaxItems->setValue(m);
}

