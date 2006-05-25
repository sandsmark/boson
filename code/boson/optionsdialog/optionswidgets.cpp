/*
    This file is part of the Boson game
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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

#include "../bomemory/bodummymemory.h"
#include "configoptionwidgets.h"
#include "bosonconfig.h"
#include "bosoncursor.h"
#include "modelrendering/bosonmodeltextures.h"
#include "defines.h"
#include "bodebug.h"
#include "bogltooltip.h"
#include "bogroundrenderermanager.h"
#include "bo3dtools.h"
#include "bofullscreen.h"
#include "modelrendering/bomeshrenderermanager.h"
#include "bowaterrenderer.h"
#include "info/boinfo.h"
#include "botexture.h"
#include "bosongroundthemedata.h"
#include "boshader.h"

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


OptionsWidget::OptionsWidget()
{
}

OptionsWidget::~OptionsWidget()
{
 mConfigOptionWidgets.clear();
}

void OptionsWidget::addConfigOptionWidget(ConfigOptionWidget* w)
{
 mConfigOptionWidgets.append(w);
}

void OptionsWidget::load()
{
 for (QValueList<ConfigOptionWidget*>::iterator it = mConfigOptionWidgets.begin(); it != mConfigOptionWidgets.end(); ++it) {
	(*it)->load();
 }
}

void OptionsWidget::apply()
{
 for (QValueList<ConfigOptionWidget*>::iterator it = mConfigOptionWidgets.begin(); it != mConfigOptionWidgets.end(); ++it) {
	(*it)->apply();
 }
}

void OptionsWidget::setDefaults()
{
 for (QValueList<ConfigOptionWidget*>::iterator it = mConfigOptionWidgets.begin(); it != mConfigOptionWidgets.end(); ++it) {
	(*it)->loadDefault();
 }
}


//////////////////////////////////////////////////////////////////////
// General Options
//////////////////////////////////////////////////////////////////////

GeneralOptions::GeneralOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 mGameSpeed = new ConfigOptionWidgetInt("GameSpeed", this);
 mGameSpeed->setLabel(i18n("Game Speed"));
 mGameSpeed->setRange(MIN_GAME_SPEED, MAX_GAME_SPEED);
 addConfigOptionWidget(mGameSpeed);

 mMiniMapScale = new ConfigOptionWidgetDouble("MiniMapScale", this);
 mMiniMapScale->setLabel(i18n("Mini Map scale factor"));
 mMiniMapScale->setRange(1.0, 5.0, 1);
 addConfigOptionWidget(mMiniMapScale);

 mRMBMovesWithAttack = new ConfigOptionWidgetBool("RMBMovesWithAttack", this);
 mRMBMovesWithAttack->setLabel(i18n("Units attack enemies in sight while moving"));
 addConfigOptionWidget(mRMBMovesWithAttack);
}

GeneralOptions::~GeneralOptions()
{
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
 setCursor((CursorMode)boConfig->intDefaultValue("CusorMode"));
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
 mRMBScrolling = new ConfigOptionWidgetBool("RMBMove", this);
 mRMBScrolling->setLabel(i18n("Enable right mouse button scrolling"));
 addConfigOptionWidget(mRMBScrolling);

 mMMBScrolling = new ConfigOptionWidgetBool("MMBMove", this);
 mMMBScrolling->setLabel(i18n("Enable middle mouse button scrolling"));
 addConfigOptionWidget(mMMBScrolling);

 mWheelMoveZoom = new ConfigOptionWidgetBool("WheelMoveZoom", this);
 mWheelMoveZoom->setLabel(i18n("Enable Zoom'nScroll"));
 addConfigOptionWidget(mWheelMoveZoom);

 mCursorEdgeSensity = new ConfigOptionWidgetUInt("CursorEdgeSensity", this);
 mCursorEdgeSensity->setLabel(i18n("Sensity of cursor at edge of the window scrolling (0 for disabled)"));
 mCursorEdgeSensity->setRange(0, 50);
 addConfigOptionWidget(mCursorEdgeSensity);

 mArrowSpeed = new ConfigOptionWidgetUInt("ArrowKeyStep", this);
 mArrowSpeed->setLabel(i18n("Arrow key steps"));
 mArrowSpeed->setRange(1, 200);
 addConfigOptionWidget(mArrowSpeed);

 QMap<int, QString> items;
 items.insert((int)CameraMove, i18n("Move camera"));
 items.insert((int)CameraZoom, i18n("Zoom camera"));
 items.insert((int)CameraRotate, i18n("Rotate camera"));

 QHBox* hbox;
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

 mMouseWheelAction->setCurrentItem(boConfig->intDefaultValue("MouseWheelAction"));
 mMouseWheelShiftAction->setCurrentItem(boConfig->intDefaultValue("MouseWheelShiftAction"));
}

ScrollingOptions::~ScrollingOptions()
{
 boDebug(210) << k_funcinfo << endl;
}

void ScrollingOptions::apply()
{
 boDebug(210) << k_funcinfo << endl;
 OptionsWidget::apply();

 boConfig->setIntValue("MouseWheelAction", (CameraAction)(mMouseWheelAction->currentItem()));
 boConfig->setIntValue("MouseWheelShiftAction", (CameraAction)(mMouseWheelShiftAction->currentItem()));
 boDebug(210) << k_funcinfo << "done" << endl;
}

void ScrollingOptions::setDefaults()
{
 OptionsWidget::setDefaults();
 mMouseWheelAction->setCurrentItem(boConfig->intDefaultValue("MouseWheelAction"));
 mMouseWheelShiftAction->setCurrentItem(boConfig->intDefaultValue("MouseWheelShiftAction"));
}

void ScrollingOptions::load()
{
 OptionsWidget::load();
 setMouseWheelAction((CameraAction)boConfig->intValue("MouseWheelAction"));
 setMouseWheelShiftAction((CameraAction)boConfig->intValue("MouseWheelShiftAction"));
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
 weaponsounds->setChecked(boConfig->boolDefaultValue("DeactivateWeaponSounds"));
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

 mAlignSelectBoxes = new ConfigOptionWidgetBool("AlignSelectionBoxes", this);
 mAlignSelectBoxes->setLabel(i18n("Align unit selection boxes to camera"));
 addConfigOptionWidget(mAlignSelectBoxes);

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

 mUpdateInterval = new ConfigOptionWidgetUInt("GLUpdateInterval", this);
 mUpdateInterval->setLabel(i18n("Update interval (low values hurt performance)"));
 mUpdateInterval->setRange(1, 100);
 QToolTip::add(mUpdateInterval, i18n("The update interval specifies after how many milli seconds the scene gets re-rendered and therefore directly influence the frames per seconds. But low values prevent boson from doing other important tasks and therefore you might end up in a game that takes several seconds until your units react to your commands! 20ms are usually a good value."));
 addConfigOptionWidget(mUpdateInterval);

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

 mUseCompressedTextures = new ConfigOptionWidgetBool("TextureCompression", mAdvanced);
 mUseCompressedTextures->setLabel(i18n("Use compressed textures"));
 QToolTip::add(mUseCompressedTextures, i18n("Compressing textures reduces amount of memory used\nat the expense of very slight quality loss."));
 addConfigOptionWidget(mUseCompressedTextures);

 mUseColoredMipmaps = new ConfigOptionWidgetBool("TextureColorMipmaps", mAdvanced);
 mUseColoredMipmaps->setLabel(i18n("Use colored mipmaps"));
 QToolTip::add(mUseColoredMipmaps, i18n("Colors different mipmap levels so you can make difference between them.\nDo not enable this unless you know what you're doing."));
 addConfigOptionWidget(mUseColoredMipmaps);

 mUseLight = new ConfigOptionWidgetBool("UseLight", mAdvanced);
 mUseLight->setLabel(i18n("Enable light"));
 addConfigOptionWidget(mUseLight);

 mUseMaterials = new ConfigOptionWidgetBool("UseMaterials", mAdvanced);
 mUseMaterials->setLabel(i18n("Use materials"));
 QToolTip::add(mUseMaterials, i18n("Materials influence the way in which models (like units) are lighted. You can disable them to gain some performance."));
 addConfigOptionWidget(mUseMaterials);

 hbox = new QHBox(mAdvanced);
 (void)new QLabel(i18n("Ground render:"), hbox);
 mGroundRenderer = new QComboBox(hbox);

 mUseGroundShaders = new ConfigOptionWidgetBool("UseGroundShaders", mAdvanced);
 mUseGroundShaders->setLabel(i18n("Use ground shaders (and shadows)"));
 mUseGroundShaders->setEnabled(BosonGroundThemeData::shadersSupported());
 addConfigOptionWidget(mUseGroundShaders);
 mUseUnitShaders = new ConfigOptionWidgetBool("UseUnitShaders", mAdvanced);
 mUseUnitShaders->setLabel(i18n("Use unit shaders (and shadows)"));
 mUseUnitShaders->setEnabled(BosonGroundThemeData::shadersSupported());
 addConfigOptionWidget(mUseUnitShaders);

 hbox = new QHBox(mAdvanced);
 (void)new QLabel(i18n("Shader quality:"), hbox);
 mShaderQuality = new QComboBox(hbox);
 mShaderQuality->setEnabled(BosonGroundThemeData::shadersSupported());
 mShaderQuality->insertItem(i18n("Extreme"));
 mShaderQuality->insertItem(i18n("High"));
 mShaderQuality->insertItem(i18n("Medium"));
 mShaderQuality->insertItem(i18n("Low"));
 mShaderQuality->setCurrentItem(2);
 hbox = new QHBox(mAdvanced);
 (void)new QLabel(i18n("Shadow quality:"), hbox);
 mShadowQuality = new QComboBox(hbox);
 mShadowQuality->insertItem(i18n("High"));
 mShadowQuality->insertItem(i18n("Medium"));
 mShadowQuality->insertItem(i18n("Low"));
 mShadowQuality->setCurrentItem(0);

 mUseLOD = new ConfigOptionWidgetBool("UseLOD", mAdvanced);
 mUseLOD->setLabel(i18n("Use level of detail"));
 addConfigOptionWidget(mUseLOD);

 hbox = new QHBox(mAdvanced);
 (void)new QLabel(i18n("Default level of detail:"), hbox);
 mDefaultLOD = new QComboBox(hbox);
 mDefaultLOD->insertItem(i18n("All details (default)"));
 mDefaultLOD->insertItem(i18n("Lowest details"));
 mDefaultLOD->setCurrentItem(0);
 connect(mUseLOD, SIGNAL(signalValueChanged(bool)), hbox, SLOT(setEnabled(bool)));
 mSmoothShading = new ConfigOptionWidgetBool("SmoothShading", mAdvanced);
 mSmoothShading->setLabel(i18n("Smooth shade model"));
 addConfigOptionWidget(mSmoothShading);

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
		mUpdateInterval->loadDefault();
		setTextureFilter(boConfig->intDefaultValue("TextureFilter"));
		mUseCompressedTextures->loadDefault();
		mUseColoredMipmaps->setChecked(false);
		mAlignSelectBoxes->setValue(boConfig->boolDefaultValue("AlignSelectionBoxes"));
		mUseLight->loadDefault();
		mUseMaterials->loadDefault();
		setCurrentGroundRenderer(boConfig->stringDefaultValue("GroundRendererClass"));
		mUseGroundShaders->setChecked(false);
		mUseUnitShaders->setChecked(false);
		mShaderQuality->setCurrentItem(shaderSuffixesToIndex("-med"));
		mShadowQuality->setCurrentItem(shadowMapResolutionToIndex(2048));
		setCurrentMeshRenderer(boConfig->stringDefaultValue("MeshRenderer"));
		mUseLOD->loadDefault();
		setDefaultLOD(0);
		mSmoothShading->setChecked(true);
		mResolution->setCurrentItem(0);
		break;
	case BestQuality:
		// we mostly use defaults here.
		mUpdateInterval->loadDefault();
		setTextureFilter(boConfig->intDefaultValue("TextureFilter"));
		mUseCompressedTextures->loadDefault();
		mUseColoredMipmaps->setChecked(false);
		mUseLight->loadDefault();
		mUseMaterials->loadDefault();
		setCurrentGroundRenderer(boConfig->stringDefaultValue("GroundRendererClass"));
		mUseGroundShaders->setChecked(true);
		mUseUnitShaders->setChecked(true);
		mShaderQuality->setCurrentItem(0); // upmost item is the best-quality one
		mShadowQuality->setCurrentItem(0); // upmost item is the best-quality one
		setCurrentMeshRenderer(boConfig->stringDefaultValue("MeshRenderer"));
		mUseLOD->setValue(true);
		setDefaultLOD(0);
		mSmoothShading->setChecked(true);
		break;
	case Fastest:
		mUpdateInterval->loadDefault();
		setTextureFilter(GL_LINEAR);  // Use bilinear maybe?
		mUseCompressedTextures->setValue(true);
		mUseColoredMipmaps->setChecked(false);
		mUseLight->setChecked(false);
		mUseMaterials->setChecked(false);
		setCurrentGroundRenderer("BoFastGroundRenderer");
		mUseGroundShaders->setChecked(false);
		mUseUnitShaders->setChecked(false);
		mShaderQuality->setCurrentItem(shaderSuffixesToIndex("-low"));
		mShadowQuality->setCurrentItem(shadowMapResolutionToIndex(512));
		setCurrentMeshRenderer(boConfig->stringDefaultValue("MeshRenderer"));
		mUseLOD->setValue(true);
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

QString OpenGLOptions::shaderSuffixes()
{
 QString suffixes;
 if (mShaderQuality->currentItem() <= 0) {
	suffixes += "-vhi,";
 }
 if (mShaderQuality->currentItem() <= 1) {
	suffixes += "-hi,";
 }
 if (mShaderQuality->currentItem() <= 2) {
	suffixes += "-med,";
 }
 if (mShaderQuality->currentItem() <= 3) {
	suffixes += "-low,";
 }

 return suffixes;
}

int OpenGLOptions::shaderSuffixesToIndex(const QString& suffixes)
{
 if (suffixes.contains("vhi")) {
	return 0;
 } else if (suffixes.contains("hi")) {
	return 1;
 } else if (suffixes.contains("med")) {
	return 2;
 } else if (suffixes.contains("low")) {
	return 3;
 } else {
	return 2;  // Fallback to medium
 }
}

int OpenGLOptions::shadowMapResolutionToIndex(int resolution)
{
 if (resolution == 2048) {
	return 0;
 } else if (resolution == 1024) {
	return 1;
 } else if (resolution == 512) {
	return 2;
 } else {
	return 0;  // Fallback to 2048
 }
}

int OpenGLOptions::indexToShadowMapResolution(int index)
{
 const int resolutions[] = { 2048, 1024, 512 };
 if (index > 2) {
	return 2048;  // default
 } else {
	return resolutions[index];
 }
}

void OpenGLOptions::apply()
{
 boDebug(210) << k_funcinfo << endl;
 OptionsWidget::apply();
 boConfig->setIntValue("RenderingSpeed", renderingSpeed());

 QString changesThatNeedRestart;
 bool resetTexParameter = false;
 bool reloadTextures = false;
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

 boConfig->setUIntValue("DefaultLOD", defaultLOD());

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
 BosonGroundThemeData::setUseGroundShaders(mUseGroundShaders->isChecked());

 if (shaderSuffixes() != boConfig->stringValue("ShaderSuffixes")) {
	boConfig->setStringValue("ShaderSuffixes", shaderSuffixes());
	boShaderManager->reloadShaders();
 }
 if (indexToShadowMapResolution(mShadowQuality->currentItem()) != boConfig->intValue("ShadowMapResolution")) {
	boConfig->setIntValue("ShadowMapResolution", indexToShadowMapResolution(mShadowQuality->currentItem()));
 }


 boDebug(210) << k_funcinfo << "done" << endl;
}

void OpenGLOptions::setDefaults()
{
 OptionsWidget::setDefaults();
 mRenderingSpeed->setCurrentItem(0);
 slotRenderingSpeedChanged(0);
}

void OpenGLOptions::load()
{
 OptionsWidget::load();
 setRenderingSpeed(boConfig->intValue("RenderingSpeed", Defaults));
 setTextureFilter(boConfig->intValue("TextureFilter"));
 setTextureAnisotropy(boConfig->intValue("TextureAnisotropy"));
 setDefaultLOD(boConfig->uintValue("DefaultLOD", 0));
 mResolution->setCurrentItem(0);

 setCurrentMeshRenderer(BoMeshRendererManager::manager()->currentRendererName());
 setCurrentGroundRenderer(BoGroundRendererManager::manager()->currentRendererName());
 mShaderQuality->setCurrentItem(shaderSuffixesToIndex(boConfig->stringValue("ShaderSuffixes")));
 mShadowQuality->setCurrentItem(shadowMapResolutionToIndex(boConfig->intValue("ShadowMapResolution")));
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
 mShaders = new ConfigOptionWidgetBool("WaterShaders", this);
 mShaders->setLabel(i18n("Enable shaders"));
 mShaders->setEnabled(boWaterRenderer->supportsShaders());
 addConfigOptionWidget(mShaders);

 mReflections = new ConfigOptionWidgetBool("WaterReflections", this);
 mReflections->setLabel(i18n("Enable reflections"));
 mReflections->setEnabled(boWaterRenderer->supportsReflections());
 addConfigOptionWidget(mReflections);

 mTranslucency = new ConfigOptionWidgetBool("WaterTranslucency", this);
 mTranslucency->setLabel(i18n("Enable translucent water"));
 mTranslucency->setEnabled(boWaterRenderer->supportsTranslucency());
 addConfigOptionWidget(mTranslucency);

 mBumpmapping = new ConfigOptionWidgetBool("WaterBumpmapping", this);
 mBumpmapping->setLabel(i18n("Enable bumpmapped water"));
 mBumpmapping->setEnabled(boWaterRenderer->supportsBumpmapping());
 addConfigOptionWidget(mBumpmapping);

 mAnimatedBumpmaps = new ConfigOptionWidgetBool("WaterAnimatedBumpmaps", this);
 mAnimatedBumpmaps->setLabel(i18n("Enable animated bumpmaps"));
 mAnimatedBumpmaps->setEnabled(boWaterRenderer->supportsBumpmapping());
 addConfigOptionWidget(mAnimatedBumpmaps);

 connect(mShaders, SIGNAL(toggled(bool)), this, SLOT(slotEnableShaders(bool)));
}

WaterOptions::~WaterOptions()
{
 boDebug(210) << k_funcinfo << endl;
}

void WaterOptions::apply()
{
 boDebug(210) << k_funcinfo << endl;
 OptionsWidget::apply();
 boWaterRenderer->reloadConfiguration();
 boDebug(210) << k_funcinfo << "done" << endl;
}

void WaterOptions::setDefaults()
{
 OptionsWidget::setDefaults();
 slotEnableShaders(mShaders->isChecked());
}

void WaterOptions::load()
{
 OptionsWidget::load();
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
 mScreenRemoveTime = new ConfigOptionWidgetUInt("ChatScreenRemoveTime", this);
 mScreenRemoveTime->setLabel(i18n("Remove from screen after seconds (0 to remove never)"));
 mScreenRemoveTime->setRange(0, 400);
 addConfigOptionWidget(mScreenRemoveTime);

 mScreenMaxItems = new ConfigOptionWidgetInt("ChatScreenMaxItems", this);
 mScreenMaxItems->setLabel(i18n("Maximal items on the screen (-1 is unlimited)"));
 mScreenMaxItems->setRange(-1, 40);
 addConfigOptionWidget(mScreenMaxItems);
}

ChatOptions::~ChatOptions()
{
 boDebug(210) << k_funcinfo << endl;
}

//////////////////////////////////////////////////////////////////////
// ToolTip Options
//////////////////////////////////////////////////////////////////////

ToolTipOptions::ToolTipOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 // you could reduce the update period to very low values to monitor changes
 // that change very often (you may want to use that in combination with pause),
 // such as waypoints (?), health, reload state, ...
 mUpdatePeriod = new ConfigOptionWidgetInt("ToolTipUpdatePeriod", this);
 mUpdatePeriod->setLabel(i18n("Update period. Low values lead to more current data. Debugging option - leave this at the default.)"));
 mUpdatePeriod->setRange(1, 500);
 addConfigOptionWidget(mUpdatePeriod);

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
 OptionsWidget::apply();
 BoToolTipCreatorFactory factory;
 QValueList<int> tips = factory.availableTipCreators();
 int index = mToolTipCreator->currentItem();
 if (index >= 0 && index < (int)tips.count()) {
	boConfig->setIntValue("ToolTipCreator", tips[index]);
 } else {
	boWarning() << k_funcinfo << "invalid tooltip creator index=" << index << endl;
 }
}

void ToolTipOptions::setDefaults()
{
 OptionsWidget::setDefaults();
 BoToolTipCreatorFactory factory;
 QValueList<int> tips = factory.availableTipCreators();
 int index = -1;
 for (unsigned int i = 0; i < tips.count(); i++) {
	if (tips[i] == boConfig->intDefaultValue("ToolTipCreator")) {
		index = i;
	}
 }
 mToolTipCreator->setCurrentItem(index);
}

void ToolTipOptions::load()
{
 OptionsWidget::load();
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

