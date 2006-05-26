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

void OptionsWidget::loadFromConfigScript(const BosonConfigScript* script)
{
 BO_CHECK_NULL_RET(script);
 for (QValueList<ConfigOptionWidget*>::iterator it = mConfigOptionWidgets.begin(); it != mConfigOptionWidgets.end(); ++it) {
	(*it)->loadFromConfigScript(script);
 }
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

