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
#include "bosonmodeltextures.h"
#include "bosontexturearray.h"
#include "boson.h"
#include "defines.h"
#include "bodebug.h"
#include "bogltooltip.h"
#include "bogroundrenderer.h"
#include "bo3dtools.h"
#include "bosonfont/bosonglfont.h"
#include "bosonfont/bosonglfontchooser.h"

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

#include "optionswidgets.moc"

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


 QHBox* hbox = new QHBox(this);
 (void)new QLabel(i18n("Command frame background pixmap"), hbox);
 mCmdBackground = new QComboBox(hbox);
 mCmdBackgrounds = KGlobal::dirs()->findAllResources("data", "boson/themes/ui/*/cmdpanel*.png");
 mCmdBackground->insertItem(i18n("None"));
 //TODO: display filename only... - not the complete path
 mCmdBackground->insertStringList(mCmdBackgrounds);

 mRMBMovesWithAttack = new QCheckBox("Units attack enemies in sight while moving", this);
 mRMBMovesWithAttack->setChecked(boConfig->RMBMovesWithAttack());
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
 boConfig->setMiniMapScale(mMiniMapScale->value());
 QString file;
 if (mCmdBackground->currentItem() > 0) {
	file = mCmdBackgrounds[mCmdBackground->currentItem() - 1];
 }
 emit signalCmdBackgroundChanged(file);
 boConfig->setRMBMovesWithAttack(mRMBMovesWithAttack->isChecked());
 boDebug(210) << k_funcinfo << "done" << endl;
}

void GeneralOptions::load()
{
 if (!game()) {
	boError(210) << k_funcinfo << "NULL game" << endl;
	return;
 }
 setGameSpeed(game()->gameSpeed());
 setMiniMapScale(boConfig->miniMapScale());
 setRMBMovesWithAttack(boConfig->RMBMovesWithAttack());
 // TODO: cmdbackground
}

void GeneralOptions::setDefaults()
{
 setGameSpeed(DEFAULT_GAME_SPEED);
 setMiniMapScale(DEFAULT_MINIMAP_SCALE);
 setCmdBackground(QString::null);
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

void GeneralOptions::setCmdBackground(const QString& file)
{
 if (file.isEmpty() || mCmdBackgrounds.findIndex(file) < 0) {
	mCmdBackground->setCurrentItem(0);
 } else {
	int index = mCmdBackgrounds.findIndex(file);
	mCmdBackground->setCurrentItem(index + 1);
 }
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
 setCursor((CursorMode)boConfig->cursorMode());
 int dirIndex = -1;
 if (boConfig->cursorDir().isNull()) {
	dirIndex = 0;
 } else {
	dirIndex = BosonCursor::availableThemes().findIndex(boConfig->cursorDir());
 }
 if (dirIndex < 0) {
	boWarning() << k_funcinfo << "could not find cusor theme " << boConfig->cursorDir() << endl;
	dirIndex = 0;
 }
 mCursorTheme->setCurrentItem(dirIndex);
 QString dir = boConfig->cursorDir();
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
 boConfig->setMouseWheelAction((CameraAction)(mMouseWheelAction->currentItem()));
 boConfig->setMouseWheelShiftAction((CameraAction)(mMouseWheelShiftAction->currentItem()));
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
 setArrowScrollSpeed(boConfig->arrowKeyStep());
 setCursorEdgeSensity(boConfig->cursorEdgeSensity());
 setRMBScrolling(boConfig->rmbMove());
 setMMBScrolling(boConfig->mmbMove());
 setMouseWheelAction(boConfig->mouseWheelAction());
 setMouseWheelShiftAction(boConfig->mouseWheelShiftAction());
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
 boConfig->setDeactivateWeaponSounds(weaponsounds->isChecked());
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
 weaponsounds->setChecked(boConfig->deactivateWeaponSounds());
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

 QHBox* fontBox = new QHBox(this);
 (void)new QLabel(i18n("Font: "), fontBox);
 mFont = new QPushButton(fontBox);
 mFontChanged = false;
 mFontInfo = new BoFontInfo();
 connect(mFont, SIGNAL(clicked()), this, SLOT(slotChangeFont()));

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
 mModelTexturesMipmaps = new QCheckBox(mAdvanced);
 mModelTexturesMipmaps->setText(i18n("Use mipmaps for model textures"));
 QToolTip::add(mModelTexturesMipmaps, i18n("With mipmapping disabled units often look ugly - but it consumes less memory and therefore might improve rendering speed."));

 hbox = new QHBox(mAdvanced);
 (void)new QLabel(i18n("Texture Magnification Filter"), hbox);
 mMagnificationFilter = new QComboBox(hbox);
 mMagnificationFilter->insertItem(i18n("Use GL_NEAREST (fastest)"));
 mMagnificationFilter->insertItem(i18n("Use GL_LINEAR (best quality)"));
 QToolTip::add(mMagnificationFilter, i18n("This selects which filter method is used when the textures need to be displayed at a bigger size than they are stored at. This applies to all textures.")); // AB: also note how big the impact on speed is!

 hbox = new QHBox(mAdvanced);
 (void)new QLabel(i18n("Texture Minification Filter"), hbox);
 mMinificationFilter = new QComboBox(hbox);
 mMinificationFilter->insertItem(i18n("Use GL_NEAREST (fastest)"));
 mMinificationFilter->insertItem(i18n("Use GL_LINEAR (best quality)"));
 QToolTip::add(mMinificationFilter, i18n("This selects which filter method is used when the textures need to be displayed at a smaller size than they are stored at. This applies to not-mipmapped textures only.")); // AB: also note how big the impact on speed is!

 hbox = new QHBox(mAdvanced);
 (void)new QLabel(i18n("Texture Mipmap Minification Filter"), hbox);
 mMipmapMinificationFilter = new QComboBox(hbox);
 mMipmapMinificationFilter->insertItem(i18n("Use GL_NEAREST (fastest)"));
 mMipmapMinificationFilter->insertItem(i18n("Use GL_LINEAR"));
 mMipmapMinificationFilter->insertItem(i18n("Use GL_NEAREST_MIPMAP_NEAREST"));
 mMipmapMinificationFilter->insertItem(i18n("Use GL_NEAREST_MIPMAP_LINEAR"));
 mMipmapMinificationFilter->insertItem(i18n("Use GL_LINEAR_MIPMAP_NEAREST"));
 mMipmapMinificationFilter->insertItem(i18n("Use GL_LINEAR_MIPMAP_LINEAR (best quality)"));
 QToolTip::add(mMipmapMinificationFilter, i18n("This selects which filter method is used when the textures need to be displayed at a smaller size than they are stored at. This applies to mipmapped textures (i.e. model textures) only.\nNote: The speed GL_*_MIPMAP_* will probably be noticebly slower, but it's quality is way better!"));

 mUseLight = new QCheckBox(mAdvanced);
 mUseLight->setText(i18n("Enable light"));

 mUseMaterials = new QCheckBox(mAdvanced);
 mUseMaterials->setText(i18n("Use materials"));
 QToolTip::add(mUseMaterials, i18n("Materials influence the way in which models (like units) are lighted. You can disable them to gain some performance."));

 hbox = new QHBox(mAdvanced);
 (void)new QLabel(i18n("Ground rendering method"), hbox);
 mGroundRenderer = new QComboBox(hbox);
 for (int i = 0; i < BoGroundRenderer::Last; i++) {
	mGroundRenderer->insertItem(BoGroundRenderer::rttiToName(i));
 }
 mGroundRenderer->setCurrentItem(0);

 mUseLOD = new QCheckBox(i18n("Use level of detail"), mAdvanced);
 hbox = new QHBox(mAdvanced);
 (void)new QLabel(i18n("Default level of detail"), hbox);
 mDefaultLOD = new QComboBox(hbox);
 mDefaultLOD->insertItem(i18n("All details (default)"));
 mDefaultLOD->insertItem(i18n("Lowest details"));
 mDefaultLOD->setCurrentItem(0);
 connect(mUseLOD, SIGNAL(toggled(bool)), hbox, SLOT(setEnabled(bool)));
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
		mModelTexturesMipmaps->setChecked(DEFAULT_USE_MIPMAPS_FOR_MODELS);
		setMagnificationFilter(DEFAULT_MAGNIFICATION_FILTER);
		setMinificationFilter(DEFAULT_MINIFICATION_FILTER);
		setMipmapMinificationFilter(DEFAULT_MIPMAP_MINIFICATION_FILTER);
		mUseLight->setChecked(DEFAULT_USE_LIGHT);
		mUseMaterials->setChecked(DEFAULT_USE_MATERIALS);
		mGroundRenderer->setCurrentItem(DEFAULT_GROUND_RENDERER);
		setUseLOD(true);
		setDefaultLOD(0);
	case BestQuality:
		// we mostly use defaults here.
		setUpdateInterval(DEFAULT_UPDATE_INTERVAL);
		mModelTexturesMipmaps->setChecked(true);
		setMagnificationFilter(DEFAULT_MAGNIFICATION_FILTER);
		setMinificationFilter(DEFAULT_MINIFICATION_FILTER);
		setMipmapMinificationFilter(DEFAULT_MIPMAP_MINIFICATION_FILTER);
		mUseLight->setChecked(DEFAULT_USE_LIGHT);
		mUseMaterials->setChecked(DEFAULT_USE_MATERIALS);
		mGroundRenderer->setCurrentItem(DEFAULT_GROUND_RENDERER);
		setUseLOD(true);
		setDefaultLOD(0);
		break;
	case Fastest:
		setUpdateInterval(DEFAULT_UPDATE_INTERVAL);
		mModelTexturesMipmaps->setChecked(false);
		setMagnificationFilter(GL_NEAREST);
		setMinificationFilter(GL_NEAREST);
		setMipmapMinificationFilter(GL_NEAREST);
		mUseLight->setChecked(false);
		mUseMaterials->setChecked(false);
		mGroundRenderer->setCurrentItem(BoGroundRenderer::Fast);
		setUseLOD(true);
		setDefaultLOD(5000);
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
 BoFontInfo f = *mFontInfo;
 int result = BosonGLFontChooser::getFont(f, this);
 if (result == QDialog::Accepted) {
	*mFontInfo = f;
	mFontChanged = true;
	mFont->setText(mFontInfo->guiName());
 }
}

void OpenGLOptions::apply()
{
 boDebug(210) << k_funcinfo << endl;
 boConfig->setIntValue("RenderingSpeed", renderingSpeed());

 bool reloadModelTextures = false;
 bool resetTexParameter = false;
 boConfig->setUpdateInterval((unsigned int)mUpdateInterval->value());
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
		boDebug(210) << k_funcinfo << "reloading all textures" << endl;
		BosonModelTextures::modelTextures()->reloadTextures();
	}
 }
 if (resetTexParameter) {
	// maybe display a message box now, asking for permission to reset the
	// parameters. currently we just reset them - leave it at this as long
	// as no problems appear
	if (resetTexParameter) {
		boDebug(210) << k_funcinfo << "resetting all textures parameters" << endl;
		BosonTextureArray::resetAllTexParameter();
	}
 }

 boConfig->setUIntValue("GroundRenderer", mGroundRenderer->currentItem());
 emit signalGroundRendererChanged(mGroundRenderer->currentItem());

 boConfig->setAlignSelectionBoxes(mAlignSelectBoxes->isChecked());
 boConfig->setUseLight(mUseLight->isChecked());
 boConfig->setUseMaterials(mUseMaterials->isChecked());
 boConfig->setUseLOD(useLOD());
 boConfig->setUIntValue("DefaultLOD", defaultLOD());

 if (mFontChanged) {
	boConfig->setStringValue("GLFont", mFontInfo->toString());
	mFontChanged = false;
	emit signalFontChanged(*mFontInfo);
 }


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
 boDebug(210) << k_funcinfo << "done" << endl;
}

void OpenGLOptions::setDefaults()
{
 mRenderingSpeed->setCurrentItem(0);
 slotRenderingSpeedChanged(0);

 setUpdateInterval(DEFAULT_UPDATE_INTERVAL);
 mModelTexturesMipmaps->setChecked(DEFAULT_USE_MIPMAPS_FOR_MODELS);
 setMagnificationFilter(DEFAULT_MAGNIFICATION_FILTER);
 setMinificationFilter(DEFAULT_MINIFICATION_FILTER);
 setMipmapMinificationFilter(DEFAULT_MIPMAP_MINIFICATION_FILTER);
 setAlignSelectionBoxes(DEFAULT_ALIGN_SELECTION_BOXES);
 mUseLight->setChecked(DEFAULT_USE_LIGHT);
 mUseMaterials->setChecked(DEFAULT_USE_MATERIALS);
 mGroundRenderer->setCurrentItem(DEFAULT_GROUND_RENDERER);
 setUseLOD(DEFAULT_USE_LOD);
 setDefaultLOD(0);
}

void OpenGLOptions::load()
{
 setRenderingSpeed(boConfig->intValue("RenderingSpeed", Defaults));
 setUpdateInterval(boConfig->updateInterval());
 mModelTexturesMipmaps->setChecked(boConfig->modelTexturesMipmaps());
 setMagnificationFilter(boConfig->magnificationFilter());
 setMinificationFilter(boConfig->minificationFilter());
 setMipmapMinificationFilter(boConfig->mipmapMinificationFilter());
 setAlignSelectionBoxes(boConfig->alignSelectionBoxes());
 mUseLight->setChecked(boConfig->useLight());
 mUseMaterials->setChecked(boConfig->useMaterials());
 mGroundRenderer->setCurrentItem(boConfig->uintValue("GroundRenderer"));
 setUseLOD(boConfig->useLOD());
 setDefaultLOD(boConfig->uintValue("DefaultLOD", 0));
 mEnableATIDepthWorkaround->setChecked(boConfig->boolValue("EnableATIDepthWorkaround"));
 mATIDepthWorkaroundValue->setText(QString::number(boConfig->doubleValue("ATIDepthWorkaroundValue")));
 if (!mFontInfo->fromString(boConfig->stringValue("GLFont", QString::null))) {
	boError() << k_funcinfo << "Could not load font " << boConfig->stringValue("GLFont", QString::null) << endl;
	*mFontInfo = BoFontInfo();
 }
 mFont->setText(mFontInfo->guiName());
 mFontChanged = false;
}

void OpenGLOptions::setUpdateInterval(int ms)
{
 boDebug(210) << k_funcinfo << ms << endl;
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
 boConfig->setChatScreenRemoveTime(mScreenRemoveTime->value());
 boConfig->setChatScreenMaxItems(mScreenMaxItems->value());
 boDebug(210) << k_funcinfo << "done" << endl;
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
	boConfig->setToolTipCreator(tips[index]);
 } else {
	boWarning() << k_funcinfo << "invalid tooltip creator index=" << index << endl;
 }
 boConfig->setToolTipUpdatePeriod(mUpdatePeriod->value());
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
 mUpdatePeriod->setValue(boConfig->toolTipUpdatePeriod());
 BoToolTipCreatorFactory factory;
 QValueList<int> tips = factory.availableTipCreators();
 int index = -1;
 for (unsigned int i = 0; i < tips.count(); i++) {
	if (tips[i] == boConfig->toolTipCreator()) {
		index = i;
	}
 }
 mToolTipCreator->setCurrentItem(index);
}

