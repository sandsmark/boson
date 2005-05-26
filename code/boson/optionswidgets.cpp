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
#include "optionswidgets.moc"

#include "bosonconfig.h"
#include "bosoncursor.h"
#include "bosonmodeltextures.h"
#include "boson.h"
#include "defines.h"
#include "bodebug.h"
#include "bogltooltip.h"
#include "bogroundrenderermanager.h"
#include "bo3dtools.h"
#include "bofullscreen.h"
#include "bomeshrenderermanager.h"
#include "bowater.h"
#include "info/boinfo.h"
#include "botexture.h"
#include "bosongroundtheme.h"

#include <kapplication.h>
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
#include <qpushbutton.h>
#include <qlineedit.h>


// we use libufo fonts now.
// TODO: make them configurable
// TODO: port bosonfont to libufo
#define BOSONFONT 0
#if BOSONFONT
#include "bosonfont/bosonglfont.h"
#include "bosonfont/bosonglfontchooser.h"
#endif

OptionsWidget::OptionsWidget()
{
 mGame = 0;
 mPlayer = 0;
}

OptionsWidget::~OptionsWidget()
{
 boDebug(210) << k_funcinfo << endl;
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


 mRMBMovesWithAttack = new QCheckBox("Units attack enemies in sight while moving", this);
 mRMBMovesWithAttack->setChecked(boConfig->boolValue("RMBMovesWithAttack"));
}

GeneralOptions::~GeneralOptions()
{
 boDebug(210) << k_funcinfo << endl;
}

void GeneralOptions::apply()
{
 boDebug(210) << k_funcinfo << endl;
 if (!game()) {
	boError(210) << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (mGameSpeed->value() != game()->gameSpeed()) {
	game()->slotSetGameSpeed(mGameSpeed->value());
 }
 boConfig->setDoubleValue("MiniMapScale", mMiniMapScale->value());
 QString file;

 boConfig->setBoolValue("RMBMovesWithAttack", mRMBMovesWithAttack->isChecked());
 boDebug(210) << k_funcinfo << "done" << endl;
}

void GeneralOptions::load()
{
 if (!game()) {
	boError(210) << k_funcinfo << "NULL game" << endl;
	return;
 }
 setGameSpeed(game()->gameSpeed());
 setMiniMapScale(boConfig->doubleValue("MiniMapScale"));
 setRMBMovesWithAttack(boConfig->boolValue("RMBMovesWithAttack"));
}

void GeneralOptions::setDefaults()
{
 setGameSpeed(DEFAULT_GAME_SPEED);
 setMiniMapScale(DEFAULT_MINIMAP_SCALE);
 setRMBMovesWithAttack(DEFAULT_RMB_MOVES_WITH_ATTACK);
}

void GeneralOptions::setGameSpeed(int ms)
{
 mGameSpeed->setValue(ms);
}

void GeneralOptions::setMiniMapScale(double scale)
{
 mMiniMapScale->setValue(scale);
}

void GeneralOptions::setRMBMovesWithAttack(bool attack)
{
 mRMBMovesWithAttack->setChecked(attack);
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
		boWarning(210) << "invalid cursor " << list[i] << endl;
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
 boDebug(210) << k_funcinfo << endl;
}

void CursorOptions::apply()
{
 boDebug(210) << k_funcinfo << endl;
 int mode = mCursor->currentItem();
 if (mode < 0) {
	boWarning(210) << k_funcinfo << "Invalid cursor mode " << mode << endl;
	return;
 }
 QString theme;
 if (mCursorTheme->currentItem() >= 0) {
	theme = mCursorThemes[mCursorTheme->currentItem()];
 } else {
	theme = BosonCursor::defaultTheme();
 }
 emit signalCursorChanged(mode, theme);
 boDebug(210) << k_funcinfo << "done" << endl;
}

void CursorOptions::setDefaults()
{
 setCursor(DEFAULT_CURSOR);
 mCursorTheme->setCurrentItem(0);
}

void CursorOptions::load()
{
 setCursor((CursorMode)boConfig->intValue("CursorMode"));
 int dirIndex = -1;
 if (boConfig->stringValue("CursorDir").isNull()) {
	dirIndex = 0;
 } else {
	dirIndex = BosonCursor::availableThemes().findIndex(boConfig->stringValue("CursorDir"));
 }
 if (dirIndex < 0) {
	boWarning() << k_funcinfo << "could not find cusor theme " << boConfig->stringValue("CursorDir") << endl;
	dirIndex = 0;
 }
 mCursorTheme->setCurrentItem(dirIndex);
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

 QMap<int, QString> items;
 items.insert((int)CameraMove, i18n("Move camera"));
 items.insert((int)CameraZoom, i18n("Zoom camera"));
 items.insert((int)CameraRotate, i18n("Rotate camera"));
 hbox = new QHBox(this);
 (void)new QLabel(i18n("Mouse wheel action"), hbox);
 mMouseWheelAction = new QComboBox(hbox);

 hbox = new QHBox(this);
 (void)new QLabel(i18n("Mouse wheel + shift action"), hbox);
 mMouseWheelShiftAction = new QComboBox(hbox);

 QMap<int, QString>::Iterator it = items.begin();
 for (; it != items.end(); ++it) {
	mMouseWheelAction->insertItem(it.data(), it.key());
	mMouseWheelShiftAction->insertItem(it.data(), it.key());
 }

 mMouseWheelAction->setCurrentItem((int)DEFAULT_MOUSE_WHEEL_ACTION);
 mMouseWheelShiftAction->setCurrentItem((int)DEFAULT_MOUSE_WHEEL_SHIFT_ACTION);
}

ScrollingOptions::~ScrollingOptions()
{
 boDebug(210) << k_funcinfo << endl;
}

void ScrollingOptions::apply()
{
 boDebug(210) << k_funcinfo << endl;
 boConfig->setBoolValue("RMBMove", mRMBScrolling->isChecked());
 boConfig->setBoolValue("MMBMove", mMMBScrolling->isChecked());
 if (mCursorEdgeSensity->value() < 0) {
	mCursorEdgeSensity->setValue(0);
 }
 boConfig->setUIntValue("CursorEdgeSensity", mCursorEdgeSensity->value());
 if (mArrowSpeed->value() < 0) {
	mArrowSpeed->setValue(0);
 }
 boConfig->setUIntValue("ArrowKeyStep", mArrowSpeed->value());
 boConfig->setIntValue("MouseWheelAction", (CameraAction)(mMouseWheelAction->currentItem()));
 boConfig->setIntValue("MouseWheelShiftAction", (CameraAction)(mMouseWheelShiftAction->currentItem()));
 boDebug(210) << k_funcinfo << "done" << endl;
}

void ScrollingOptions::setDefaults()
{
 setArrowScrollSpeed(DEFAULT_ARROW_SCROLL_SPEED);
 setCursorEdgeSensity(DEFAULT_CURSOR_EDGE_SENSITY);
 setRMBScrolling(DEFAULT_USE_RMB_MOVE);
 setMMBScrolling(DEFAULT_USE_MMB_MOVE);
 mMouseWheelAction->setCurrentItem((int)DEFAULT_MOUSE_WHEEL_ACTION);
 mMouseWheelShiftAction->setCurrentItem((int)DEFAULT_MOUSE_WHEEL_SHIFT_ACTION);
}

void ScrollingOptions::load()
{
 setArrowScrollSpeed(boConfig->uintValue("ArrowKeyStep"));
 setCursorEdgeSensity(boConfig->uintValue("CursorEdgeSensity"));
 setRMBScrolling(boConfig->boolValue("RMBMove"));
 setMMBScrolling(boConfig->boolValue("MMBMove"));
 setMouseWheelAction((CameraAction)boConfig->intValue("MouseWheelAction"));
 setMouseWheelShiftAction((CameraAction)boConfig->intValue("MouseWheelShiftAction"));
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

void ScrollingOptions::setMouseWheelAction(CameraAction action)
{
 mMouseWheelAction->setCurrentItem((int)action);
}

void ScrollingOptions::setMouseWheelShiftAction(CameraAction action)
{
 mMouseWheelShiftAction->setCurrentItem((int)action);
}


//////////////////////////////////////////////////////////////////////
// Sound Options
//////////////////////////////////////////////////////////////////////

SoundOptions::SoundOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 (void)new QLabel(i18n("Disable the following unit sounds (please send a bug report if you can think of more descriptive names):"), this);
 QCheckBox* c;
 weaponsounds = new QCheckBox(i18n("Weapon sounds"), this);
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
 boDebug(210) << k_funcinfo << endl;
}

void SoundOptions::apply()
{
 boDebug(210) << k_funcinfo << endl;
 QMap<QCheckBox*, UnitSoundEvent>::Iterator it = mCheckBox2UnitSoundEvent.begin();
 for (; it != mCheckBox2UnitSoundEvent.end(); ++it) {
	boConfig->setUnitSoundActivated(it.data(), !it.key()->isChecked());
 }
 boConfig->setBoolValue("DeactivateWeaponSounds", weaponsounds->isChecked());
 boDebug(210) << k_funcinfo << "done" << endl;
}

void SoundOptions::setDefaults()
{
 QMap<QCheckBox*, UnitSoundEvent>::Iterator it = mCheckBox2UnitSoundEvent.begin();
 for (; it != mCheckBox2UnitSoundEvent.end(); ++it) {
	it.key()->setChecked(false);
 }
 weaponsounds->setChecked(DEFAULT_DEACTIVATE_WEAPON_SOUNDS);
}

void SoundOptions::load()
{
 setUnitSoundsDeactivated(boConfig);
 weaponsounds->setChecked(boConfig->boolValue("DeactivateWeaponSounds"));
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
 QHBox* hbox = new QHBox(this);

 if (!boConfig->hasKey("RenderingSpeed")) {
	boConfig->addDynamicEntry(new BoConfigIntEntry(boConfig, "RenderingSpeed", (int)Defaults));
 }
 (void)new QLabel(i18n("Rendering speed/quality"), hbox);
 mRenderingSpeed = new QComboBox(hbox);
 mRenderingSpeed->insertItem(i18n("Defaults"));
 mRenderingSpeed->insertItem(i18n("Highest quality"));
 mRenderingSpeed->insertItem(i18n("Lowest quality (e.g. for software rendering)"));
 connect(mRenderingSpeed, SIGNAL(activated(int)), this, SLOT(slotRenderingSpeedChanged(int)));

 mAlignSelectBoxes = new QCheckBox(this);
 mAlignSelectBoxes->setText(i18n("Align unit selection boxes to camera"));

 QVBox* atiWorkaround = new QVBox(this);
 mEnableATIDepthWorkaround = new QCheckBox(atiWorkaround);
 mEnableATIDepthWorkaround->setText(i18n("Enable ATI depth workaround"));
 connect(mEnableATIDepthWorkaround, SIGNAL(toggled(bool)), this, SLOT(slotEnableATIDepthWorkaround(bool)));
 QToolTip::add(mEnableATIDepthWorkaround, i18n("Use this if you own a ATI card and you have <em>extreme</em> problems at selecting units"));
 QHBox* atiValue = new QHBox(atiWorkaround);
 (void)new QLabel(i18n("Value: "), atiValue);
 mATIDepthWorkaroundValue = new QLineEdit(atiValue);
 QPushButton* atiDefaultValue = new QPushButton(i18n("Default"), atiValue);
 connect(atiDefaultValue, SIGNAL(clicked()), this, SLOT(slotATIDepthWorkaroundDefaultValue()));
 slotATIDepthWorkaroundDefaultValue();
 mEnableATIDepthWorkaround->setChecked(false);
 slotEnableATIDepthWorkaround(false);

#if BOSONFONT
 QHBox* fontBox = new QHBox(this);
 (void)new QLabel(i18n("Font: "), fontBox);
 mFont = new QPushButton(fontBox);
 mFontChanged = false;
 mFontInfo = new BoFontInfo();
 connect(mFont, SIGNAL(clicked()), this, SLOT(slotChangeFont()));
#endif

 QHBox* resolutionBox = new QHBox(this);
 (void)new QLabel(i18n("Resolution:"), resolutionBox);
 mResolution = new QComboBox(resolutionBox);
 mResolution->insertItem(i18n("Keep unchanged"));
 if (BoFullScreen::availableModes().count() != 0) {
	mResolution->insertStringList(BoFullScreen::availableModes());
	mResolution->setEnabled(true);
} else {
	mResolution->setEnabled(false);
 }
 mResolution->setCurrentItem(0);

 QPushButton* showDetails = new QPushButton(i18n("Show &Details"), this);
 showDetails->setToggleButton(true);
 connect(showDetails, SIGNAL(toggled(bool)), this, SLOT(slotShowDetails(bool)));

 mAdvanced = new QVBox(this);
 showDetails->setOn(false);
 slotShowDetails(false);

 mUpdateInterval = new KIntNumInput(DEFAULT_UPDATE_INTERVAL, mAdvanced);
 mUpdateInterval->setRange(1, 100);
 mUpdateInterval->setLabel(i18n("Update interval (low values hurt performance)"));
 QToolTip::add(mUpdateInterval, i18n("The update interval specifies after how many milli seconds the scene gets re-rendered and therefore directly influence the frames per seconds. But low values prevent boson from doing other important tasks and therefore you might end up in a game that takes several seconds until your units react to your commands! 20ms are usually a good value."));

 hbox = new QHBox(mAdvanced);
 (void)new QLabel(i18n("Texture Filter"), hbox);
 mTextureFilter = new QComboBox(hbox);
 //mTextureFilter->insertItem(i18n("Use GL_NEAREST (fastest)"));
 mTextureFilter->insertItem(i18n("Use GL_LINEAR"));
 //mTextureFilter->insertItem(i18n("Use GL_NEAREST_MIPMAP_NEAREST"));
 //mTextureFilter->insertItem(i18n("Use GL_NEAREST_MIPMAP_LINEAR"));
 mTextureFilter->insertItem(i18n("Use GL_LINEAR_MIPMAP_NEAREST (bilinear)"));
 mTextureFilter->insertItem(i18n("Use GL_LINEAR_MIPMAP_LINEAR (trilinear)"));
 if (boTextureManager->maxTextureAnisotropy() >= 2) {
	mTextureFilter->insertItem(i18n("Low anisotropic"));
 }
 if (boTextureManager->maxTextureAnisotropy() >= 4) {
	mTextureFilter->insertItem(i18n("Medium anisotropic"));
 }
 if (boTextureManager->maxTextureAnisotropy() >= 8) {
	mTextureFilter->insertItem(i18n("High anisotropic"));
 }
 if (boTextureManager->maxTextureAnisotropy() >= 16) {
	mTextureFilter->insertItem(i18n("Very high anisotropic"));
 }

 QToolTip::add(mTextureFilter, i18n("Selects texture filtering method\nGL_LINEAR_MIPMAP_LINEAR looks very good.\nAnisotropic modes look even better but are slower."));

 mUseCompressedTextures = new QCheckBox(mAdvanced);
 mUseCompressedTextures->setText(i18n("Use compressed textures"));
 QToolTip::add(mUseCompressedTextures, i18n("Compressing textures reduces amount of memory used\nat the expense of very slight quality loss."));

 mUseColoredMipmaps = new QCheckBox(mAdvanced);
 mUseColoredMipmaps->setText(i18n("Use colored mipmaps"));
 QToolTip::add(mUseColoredMipmaps, i18n("Colors different mipmap levels so you can make difference between them.\nDo not enable this unless you know what you're doing."));

 mUseLight = new QCheckBox(mAdvanced);
 mUseLight->setText(i18n("Enable light"));

 mUseMaterials = new QCheckBox(mAdvanced);
 mUseMaterials->setText(i18n("Use materials"));
 QToolTip::add(mUseMaterials, i18n("Materials influence the way in which models (like units) are lighted. You can disable them to gain some performance."));

 hbox = new QHBox(mAdvanced);
 (void)new QLabel(i18n("Ground render:"), hbox);
 mGroundRenderer = new QComboBox(hbox);
 mUseGroundShaders = new QCheckBox(i18n("Use shaders for ground rendering"), mAdvanced);
 mUseGroundShaders->setEnabled(BosonGroundTheme::shadersSupported());

 mUseLOD = new QCheckBox(i18n("Use level of detail"), mAdvanced);
 hbox = new QHBox(mAdvanced);
 (void)new QLabel(i18n("Default level of detail:"), hbox);
 mDefaultLOD = new QComboBox(hbox);
 mDefaultLOD->insertItem(i18n("All details (default)"));
 mDefaultLOD->insertItem(i18n("Lowest details"));
 mDefaultLOD->setCurrentItem(0);
 connect(mUseLOD, SIGNAL(toggled(bool)), hbox, SLOT(setEnabled(bool)));
 mSmoothShading = new QCheckBox(i18n("Smooth shade model"), mAdvanced);
 mSmoothShading->setChecked(true);
 hbox = new QHBox(mAdvanced);
 (void)new QLabel(i18n("Mesh renderer:"), hbox);
 mMeshRenderer = new QComboBox(hbox);
}

OpenGLOptions::~OpenGLOptions()
{
 boDebug(210) << k_funcinfo << endl;
}

void OpenGLOptions::slotRenderingSpeedChanged(int index)
{
 setRenderingSpeed(indexToRenderingSpeed(index));
}

int OpenGLOptions::renderingSpeed() const
{
 return indexToRenderingSpeed(mRenderingSpeed->currentItem());
}

int OpenGLOptions::indexToRenderingSpeed(int index)
{
 switch (index) {
	default:
		boWarning(210) << k_funcinfo << "invalid index: " << index << endl;
	case 0:
		return Defaults;
	case 1:
		return BestQuality;
	case 2:
		return Fastest;
 }
 return BestQuality;
}

int OpenGLOptions::renderingSpeedToIndex(int speed)
{
 switch (speed) {
	default:
		boWarning(210) << k_funcinfo << "invalid value: " << speed << endl;
	case Defaults:
		return 0;
	case BestQuality:
		return 1;
	case Fastest:
		return 2;
 }
 return 0;
}

void OpenGLOptions::setRenderingSpeed(int speed)
{
 switch ((RenderingSpeed)speed) {
	default:
		boWarning(210) << k_funcinfo << "invalid value: " << speed << endl;
	case Defaults:
		setUpdateInterval(DEFAULT_UPDATE_INTERVAL);
		setTextureFilter(DEFAULT_TEXTURE_FILTER);
		mUseCompressedTextures->setChecked(DEFAULT_TEXTURE_COMPRESSION);
		mUseColoredMipmaps->setChecked(false);
		mUseLight->setChecked(DEFAULT_USE_LIGHT);
		mUseMaterials->setChecked(DEFAULT_USE_MATERIALS);
		setCurrentGroundRenderer(DEFAULT_GROUND_RENDERER);
		mUseGroundShaders->setChecked(true);
		setCurrentMeshRenderer(DEFAULT_MESH_RENDERER);
		setUseLOD(true);
		setDefaultLOD(0);
		mSmoothShading->setChecked(true);
		break;
	case BestQuality:
		// we mostly use defaults here.
		setUpdateInterval(DEFAULT_UPDATE_INTERVAL);
		setTextureFilter(DEFAULT_TEXTURE_FILTER);
		mUseCompressedTextures->setChecked(DEFAULT_TEXTURE_COMPRESSION);
		mUseColoredMipmaps->setChecked(false);
		mUseLight->setChecked(DEFAULT_USE_LIGHT);
		mUseMaterials->setChecked(DEFAULT_USE_MATERIALS);
		setCurrentGroundRenderer(DEFAULT_GROUND_RENDERER);
		mUseGroundShaders->setChecked(true);
		setCurrentMeshRenderer(DEFAULT_MESH_RENDERER);
		setUseLOD(true);
		setDefaultLOD(0);
		mSmoothShading->setChecked(true);
		break;
	case Fastest:
		setUpdateInterval(DEFAULT_UPDATE_INTERVAL);
		setTextureFilter(GL_LINEAR);  // Use bilinear maybe?
		mUseCompressedTextures->setChecked(true);
		mUseColoredMipmaps->setChecked(false);
		mUseLight->setChecked(false);
		mUseMaterials->setChecked(false);
		setCurrentGroundRenderer("BoFastGroundRenderer");
		mUseGroundShaders->setChecked(false);
		setCurrentMeshRenderer(DEFAULT_MESH_RENDERER);
		setUseLOD(true);
		setDefaultLOD(5000);
		mSmoothShading->setChecked(false);
		break;

 }
 mRenderingSpeed->setCurrentItem(renderingSpeedToIndex(speed));
}

void OpenGLOptions::slotShowDetails(bool show)
{
 if (show) {
	mAdvanced->show();
 } else {
	mAdvanced->hide();
 }
}

void OpenGLOptions::slotChangeFont()
{
#if BOSONFONT
 BoFontInfo f = *mFontInfo;
 int result = BosonGLFontChooser::getFont(f, this);
 if (result == QDialog::Accepted) {
	*mFontInfo = f;
	mFontChanged = true;
	mFont->setText(mFontInfo->guiName());
 }
#endif
}

void OpenGLOptions::apply()
{
 boDebug(210) << k_funcinfo << endl;
 boConfig->setIntValue("RenderingSpeed", renderingSpeed());

 QString changesThatNeedRestart;
 bool resetTexParameter = false;
 bool reloadTextures = false;
 boConfig->setUIntValue("GLUpdateInterval", (unsigned int)mUpdateInterval->value());
 if (boConfig->intValue("TextureFilter") != textureFilter()) {
	boConfig->setIntValue("TextureFilter", textureFilter());
	resetTexParameter = true;
	// TODO: reload only if mipmapping was turned on/off
	reloadTextures = true;
 }
 if (boConfig->intValue("TextureAnisotropy") != textureAnisotropy()) {
	boConfig->setIntValue("TextureAnisotropy", textureAnisotropy());
	resetTexParameter = true;
 }

 if(boConfig->boolValue("TextureCompression") != mUseCompressedTextures->isChecked()) {
	boConfig->setBoolValue("TextureCompression", mUseCompressedTextures->isChecked());
	reloadTextures = true;
 }
 if(boConfig->boolValue("TextureColorMipmaps") != mUseColoredMipmaps->isChecked()) {
	boConfig->setBoolValue("TextureColorMipmaps", mUseColoredMipmaps->isChecked());
	reloadTextures = true;
 }

 if(!changesThatNeedRestart.isEmpty()) {
	KMessageBox::information(this, i18n("Following changes take effect after restart:\n%1").arg(changesThatNeedRestart));
 }
 if (resetTexParameter) {
	// maybe display a message box now, asking for permission to reset the
	// parameters. currently we just reset them - leave it at this as long
	// as no problems appear
	boDebug(210) << k_funcinfo << "resetting all textures parameters" << endl;
	boTextureManager->textureFilterChanged();
 }
 if (reloadTextures) {
	boDebug(210) << k_funcinfo << "reloading all textures" << endl;
	boTextureManager->reloadTextures();
 }

 boConfig->setBoolValue("AlignSelectionBoxes", mAlignSelectBoxes->isChecked());
 boConfig->setBoolValue("UseLight", mUseLight->isChecked());
 boConfig->setBoolValue("UseMaterials", mUseMaterials->isChecked());
 boConfig->setBoolValue("UseLOD", useLOD());
 boConfig->setUIntValue("DefaultLOD", defaultLOD());

#if BOSONFONT
 if (mFontChanged) {
	boConfig->setStringValue("GLFont", mFontInfo->toString());
	mFontChanged = false;
	emit signalFontChanged(*mFontInfo);
 }
#endif


 boConfig->setBoolValue("EnableATIDepthWorkaround", mEnableATIDepthWorkaround->isChecked());
 if (mEnableATIDepthWorkaround->isChecked()) {
	bool ok = false;
	double d = mATIDepthWorkaroundValue->text().toDouble(&ok);
	if (!ok) {
		d = 0.00390625;
	}
	boConfig->setDoubleValue("ATIDepthWorkaroundValue", d);
	Bo3dTools::enableReadDepthBufferWorkaround((float)d);
 } else {
	Bo3dTools::disableReadDepthBufferWorkaround();
 }
 boConfig->setBoolValue("SmoothShading", mSmoothShading->isChecked());

 if (mResolution->isEnabled() && mResolution->currentItem() > 0) {
	// first entry is "do not change", then a list, as provided by
	// BoFullScreen
	int index = mResolution->currentItem() - 1;
	if (!BoFullScreen::enterMode(index)) {
		boError() << k_funcinfo << "could not enter mode" << index << endl;
	 }
 }
 mResolution->setCurrentItem(0);

 if (!BoMeshRendererManager::manager()->makeRendererCurrent(mMeshRenderer->currentText())) {
	KMessageBox::sorry(this, i18n("Failed at making mesh renderer %1 current. Trying to use a default renderer (expect a crash, if we fail)...").arg(mMeshRenderer->currentText()));
	if (!BoMeshRendererManager::manager()->makeRendererCurrent(QString::null)) {
		KMessageBox::sorry(this, i18n("Failed at setting a default mesh renderer. Quitting now"));
		kapp->exit(1);
	}
 }
 if (!BoGroundRendererManager::manager()->makeRendererCurrent(mGroundRenderer->currentText())) {
	KMessageBox::sorry(this, i18n("Failed at making ground renderer %1 current. Trying to use a default renderer (expect a crash, if we fail)...").arg(mGroundRenderer->currentText()));
	if (!BoGroundRendererManager::manager()->makeRendererCurrent(QString::null)) {
		KMessageBox::sorry(this, i18n("Failed at setting a default ground renderer. Quitting now"));
		kapp->exit(1);
	}
 }
 boConfig->setBoolValue("UseGroundShaders", mUseGroundShaders->isChecked());
 BosonGroundTheme::setUseGroundShaders(mUseGroundShaders->isChecked());


 emit signalOpenGLSettingsUpdated();

 boDebug(210) << k_funcinfo << "done" << endl;
}

void OpenGLOptions::setDefaults()
{
 mRenderingSpeed->setCurrentItem(0);
 slotRenderingSpeedChanged(0);

 setUpdateInterval(DEFAULT_UPDATE_INTERVAL);
 setTextureFilter(DEFAULT_TEXTURE_FILTER);
 mUseCompressedTextures->setChecked(DEFAULT_TEXTURE_COMPRESSION);
 mUseColoredMipmaps->setChecked(false);
 setAlignSelectionBoxes(DEFAULT_ALIGN_SELECTION_BOXES);
 mUseLight->setChecked(DEFAULT_USE_LIGHT);
 mUseMaterials->setChecked(DEFAULT_USE_MATERIALS);
 setCurrentGroundRenderer(DEFAULT_GROUND_RENDERER);
 mUseGroundShaders->setChecked(true);
 setCurrentMeshRenderer(DEFAULT_MESH_RENDERER);
 setUseLOD(DEFAULT_USE_LOD);
 setDefaultLOD(0);
 mSmoothShading->setChecked(true);
 mResolution->setCurrentItem(0);
}

void OpenGLOptions::load()
{
 setRenderingSpeed(boConfig->intValue("RenderingSpeed", Defaults));
 setUpdateInterval(boConfig->uintValue("GLUpdateInterval"));
 setTextureFilter(boConfig->intValue("TextureFilter"));
 setTextureAnisotropy(boConfig->intValue("TextureAnisotropy"));
 mUseCompressedTextures->setChecked(boConfig->boolValue("TextureCompression"));
 mUseColoredMipmaps->setChecked(boConfig->boolValue("TextureColorMipmaps"));
 setAlignSelectionBoxes(boConfig->boolValue("AlignSelectionBoxes"));
 mUseLight->setChecked(boConfig->boolValue("UseLight"));
 mUseMaterials->setChecked(boConfig->boolValue("UseMaterials"));
 setUseLOD(boConfig->boolValue("UseLOD"));
 setDefaultLOD(boConfig->uintValue("DefaultLOD", 0));
 mEnableATIDepthWorkaround->setChecked(boConfig->boolValue("EnableATIDepthWorkaround"));
 mATIDepthWorkaroundValue->setText(QString::number(boConfig->doubleValue("ATIDepthWorkaroundValue")));
#if BOSONFONT
 if (!mFontInfo->fromString(boConfig->stringValue("GLFont", QString::null))) {
	boError() << k_funcinfo << "Could not load font " << boConfig->stringValue("GLFont", QString::null) << endl;
	*mFontInfo = BoFontInfo();
 }
 mFont->setText(mFontInfo->guiName());
 mFontChanged = false;
#endif
 mSmoothShading->setChecked(boConfig->boolValue("SmoothShading", true));
 mResolution->setCurrentItem(0);

 setCurrentMeshRenderer(BoMeshRendererManager::manager()->currentRendererName());
 setCurrentGroundRenderer(BoGroundRendererManager::manager()->currentRendererName());
 mUseGroundShaders->setChecked(boConfig->boolValue("UseGroundShaders"));
}

void OpenGLOptions::setUpdateInterval(int ms)
{
 boDebug(210) << k_funcinfo << ms << endl;
 mUpdateInterval->setValue(ms);
}

int OpenGLOptions::textureFilter() const
{
 GLenum e;
 switch (mTextureFilter->currentItem()) {
	case 0:
		e = GL_LINEAR;
		break;
	case 1:
		e = GL_LINEAR_MIPMAP_NEAREST;
		break;
	default:
	case 2:
		e = GL_LINEAR_MIPMAP_LINEAR;
		break;
 }
 return (int)e;
}

void OpenGLOptions::setTextureFilter(int f)
{
 switch(f) {
	case GL_LINEAR:
		mTextureFilter->setCurrentItem(0);
		break;
	case GL_LINEAR_MIPMAP_NEAREST:
		mTextureFilter->setCurrentItem(1);
		break;
	default:
	case GL_LINEAR_MIPMAP_LINEAR:
		mTextureFilter->setCurrentItem(2);
		break;
 }
}

int OpenGLOptions::textureAnisotropy() const
{
 switch (mTextureFilter->currentItem()) {
	case 3:
		return 2;
		break;
	case 4:
		return 4;
		break;
	case 5:
		return 8;
		break;
	case 6:
		return 16;
		break;
	default:
		return 1;
		break;
 }
}

void OpenGLOptions::setTextureAnisotropy(int a)
{
 switch(a) {
	case 2:
		mTextureFilter->setCurrentItem(3);
		break;
	case 4:
		mTextureFilter->setCurrentItem(4);
		break;
	case 8:
		mTextureFilter->setCurrentItem(5);
		break;
	case 16:
		mTextureFilter->setCurrentItem(6);
		break;
	default:
		if (mTextureFilter->currentItem() >= 3) {
			mTextureFilter->setCurrentItem(2);
		}
		break;
 }
}

void OpenGLOptions::setAlignSelectionBoxes(bool align)
{
 mAlignSelectBoxes->setChecked(align);
}

bool OpenGLOptions::useLOD() const
{
 return mUseLOD->isChecked();
}

void OpenGLOptions::setUseLOD(bool use)
{
 mUseLOD->setChecked(use);
}

unsigned int OpenGLOptions::defaultLOD() const
{
 switch (mDefaultLOD->currentItem()) {
	case 0:
		return 0;
	default:
	case 1:
		// just a random, very high, number.
		// this will result in lowest details possible.
		return 5000;
 }
 return 0;
}

void OpenGLOptions::setDefaultLOD(unsigned int l)
{
 switch (l) {
	case 0:
		mDefaultLOD->setCurrentItem(0);
		break;
	default:
		mDefaultLOD->setCurrentItem(1);
		break;
 }
}

void OpenGLOptions::slotEnableATIDepthWorkaround(bool e)
{
 mATIDepthWorkaroundValue->setEnabled(e);
}

void OpenGLOptions::slotATIDepthWorkaroundDefaultValue()
{
 mATIDepthWorkaroundValue->setText(QString::number(0.00390625));
}

void OpenGLOptions::setCurrentMeshRenderer(const QString& renderer)
{
 mMeshRenderer->clear();
 QStringList renderers = BoMeshRendererManager::manager()->availableRenderers();
 mMeshRenderer->insertStringList(renderers);

 for (int i = 0; i < mMeshRenderer->count(); i++) {
	if (mMeshRenderer->text(i) == renderer) {
		mMeshRenderer->setCurrentItem(i);
		return;
	}
 }
}

void OpenGLOptions::setCurrentGroundRenderer(const QString& renderer)
{
 mGroundRenderer->clear();
 QStringList renderers = BoGroundRendererManager::manager()->availableRenderers();
 mGroundRenderer->insertStringList(renderers);
 for (int i = 0; i < mGroundRenderer->count(); i++) {
	if (mGroundRenderer->text(i) == renderer) {
		mGroundRenderer->setCurrentItem(i);
		return;
	}
 }
}

//////////////////////////////////////////////////////////////////////
// Water Options
//////////////////////////////////////////////////////////////////////

WaterOptions::WaterOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
// QHBox* hbox = new QHBox(this);
 mShaders = new QCheckBox(i18n("Enable shaders"), this);
 mShaders->setEnabled(boWaterManager->supportsShaders());
 mReflections = new QCheckBox(i18n("Enable reflections"), this);
 mReflections->setEnabled(boWaterManager->supportsReflections());
 mTranslucency = new QCheckBox(i18n("Enable translucent water"), this);
 mTranslucency->setEnabled(boWaterManager->supportsTranslucency());
 mBumpmapping = new QCheckBox(i18n("Enable bumpmapped water"), this);
 mBumpmapping->setEnabled(boWaterManager->supportsBumpmapping());
 mAnimatedBumpmaps = new QCheckBox(i18n("Enable animated bumpmaps"), this);
 mAnimatedBumpmaps->setEnabled(boWaterManager->supportsBumpmapping());

 connect(mShaders, SIGNAL(toggled(bool)), this, SLOT(slotEnableShaders(bool)));
}

WaterOptions::~WaterOptions()
{
 boDebug(210) << k_funcinfo << endl;
}

void WaterOptions::apply()
{
 boDebug(210) << k_funcinfo << endl;
 boConfig->setBoolValue("WaterShaders", mShaders->isChecked());
 boConfig->setBoolValue("WaterReflections", mReflections->isChecked());
 boConfig->setBoolValue("WaterTranslucency", mTranslucency->isChecked());
 boConfig->setBoolValue("WaterBumpmapping", mBumpmapping->isChecked());
 boConfig->setBoolValue("WaterAnimatedBumpmaps", mAnimatedBumpmaps->isChecked());
 boWaterManager->reloadConfiguration();
 boDebug(210) << k_funcinfo << "done" << endl;
}

void WaterOptions::setDefaults()
{
 mShaders->setChecked(DEFAULT_WATER_SHADERS);
 mReflections->setChecked(DEFAULT_WATER_REFLECTIONS);
 mTranslucency->setChecked(DEFAULT_WATER_TRANSLUCENCY);
 mBumpmapping->setChecked(DEFAULT_WATER_BUMPMAPPING);
 mAnimatedBumpmaps->setChecked(DEFAULT_WATER_ANIMATED_BUMPMAPS);
 slotEnableShaders(mShaders->isChecked());
}

void WaterOptions::load()
{
 mShaders->setChecked(boConfig->boolValue("WaterShaders"));
 mReflections->setChecked(boConfig->boolValue("WaterReflections"));
 mTranslucency->setChecked(boConfig->boolValue("WaterTranslucency"));
 mBumpmapping->setChecked(boConfig->boolValue("WaterBumpmapping"));
 mAnimatedBumpmaps->setChecked(boConfig->boolValue("WaterAnimatedBumpmaps"));
 slotEnableShaders(mShaders->isChecked());
}

void WaterOptions::slotEnableShaders(bool)
{
 mReflections->setEnabled(!mShaders->isChecked());
 mTranslucency->setEnabled(!mShaders->isChecked());
 mBumpmapping->setEnabled(!mShaders->isChecked());
}

//////////////////////////////////////////////////////////////////////
// Chat Options
//////////////////////////////////////////////////////////////////////

ChatOptions::ChatOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
// QHBox* hbox = new QHBox(this);
 mScreenRemoveTime = new KIntNumInput(DEFAULT_CHAT_SCREEN_REMOVE_TIME, this);
 mScreenRemoveTime->setRange(0, 400);
 mScreenRemoveTime->setLabel(i18n("Remove from screen after seconds (0 to remove never)"));

// hbox = new QHBox(this);
 mScreenMaxItems = new KIntNumInput(DEFAULT_CHAT_SCREEN_REMOVE_TIME, this);
 mScreenMaxItems->setRange(-1, 40);
 mScreenMaxItems->setLabel(i18n("Maximal items on the screen (-1 is unlimited)"));
}

ChatOptions::~ChatOptions()
{
 boDebug(210) << k_funcinfo << endl;
}

void ChatOptions::apply()
{
 boDebug(210) << k_funcinfo << endl;
 boConfig->setUIntValue("ChatScreenRemoveTime", mScreenRemoveTime->value());
 boConfig->setIntValue("ChatScreenMaxItems", mScreenMaxItems->value());
 boDebug(210) << k_funcinfo << "done" << endl;
}

void ChatOptions::setDefaults()
{
 setScreenRemoveTime(DEFAULT_CHAT_SCREEN_REMOVE_TIME);
 setScreenMaxItems(DEFAULT_CHAT_SCREEN_REMOVE_TIME);
}

void ChatOptions::load()
{
 setScreenRemoveTime(boConfig->uintValue("ChatScreenRemoveTime"));
 setScreenMaxItems(boConfig->intValue("ChatScreenMaxItems"));
}

void ChatOptions::setScreenRemoveTime(unsigned int s)
{
 mScreenRemoveTime->setValue(s);
}

void ChatOptions::setScreenMaxItems(int m)
{
 mScreenMaxItems->setValue(m);
}

//////////////////////////////////////////////////////////////////////
// ToolTip Options
//////////////////////////////////////////////////////////////////////

ToolTipOptions::ToolTipOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 // you could reduce the update period to very low values to monitor changes
 // that change very often (you may want to use that in combination with pause),
 // such as waypoints (?), health, reload state, ...
 mUpdatePeriod = new KIntNumInput(DEFAULT_TOOLTIP_UPDATE_PERIOD, this);
 mUpdatePeriod->setRange(1, 500);
 mUpdatePeriod->setLabel(i18n("Update period. Low values lead to more current data. Debugging option - leave this at the default.)"));

 // TODO: the tooltip delay!

 QHBox* hbox = new QHBox(this);
 (void)new QLabel(i18n("Tooltip kind:"), hbox);
 mToolTipCreator = new QComboBox(hbox);
 BoToolTipCreatorFactory factory;
 QValueList<int> tips = factory.availableTipCreators();
 for (unsigned int i = 0; i < tips.count(); i++) {
	mToolTipCreator->insertItem(factory.tipCreatorName(tips[i]));
 }
}

ToolTipOptions::~ToolTipOptions()
{
 boDebug(210) << k_funcinfo << endl;
}

void ToolTipOptions::apply()
{
 BoToolTipCreatorFactory factory;
 QValueList<int> tips = factory.availableTipCreators();
 int index = mToolTipCreator->currentItem();
 if (index >= 0 && index < (int)tips.count()) {
	boConfig->setIntValue("ToolTipCreator", tips[index]);
 } else {
	boWarning() << k_funcinfo << "invalid tooltip creator index=" << index << endl;
 }
 boConfig->setIntValue("ToolTipUpdatePeriod", mUpdatePeriod->value());
}

void ToolTipOptions::setDefaults()
{
 mUpdatePeriod->setValue(DEFAULT_TOOLTIP_UPDATE_PERIOD);
 BoToolTipCreatorFactory factory;
 QValueList<int> tips = factory.availableTipCreators();
 int index = -1;
 for (unsigned int i = 0; i < tips.count(); i++) {
	if (tips[i] == DEFAULT_TOOLTIP_CREATOR) {
		index = i;
	}
 }
 mToolTipCreator->setCurrentItem(index);
}

void ToolTipOptions::load()
{
 mUpdatePeriod->setValue(boConfig->intValue("ToolTipUpdatePeriod"));
 BoToolTipCreatorFactory factory;
 QValueList<int> tips = factory.availableTipCreators();
 int index = -1;
 for (unsigned int i = 0; i < tips.count(); i++) {
	if (tips[i] == boConfig->intValue("ToolTipCreator")) {
		index = i;
	}
 }
 mToolTipCreator->setCurrentItem(index);
}

