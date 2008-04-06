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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef OPTIONSWIDGETS_H
#define OPTIONSWIDGETS_H

#include <q3vbox.h>
#include <qstringlist.h>
#include <qmap.h>
#include <q3valuelist.h>

#include "global.h"

class Player;
class Boson;
class BosonConfig;
class BosonConfigScript;
class BoFontInfo;
class KIntNumInput;
class KDoubleNumInput;
class QComboBox;
class QCheckBox;
class QPushButton;
class ConfigOptionWidget;
class ConfigOptionWidgetInt;
class ConfigOptionWidgetUInt;
class ConfigOptionWidgetDouble;
class ConfigOptionWidgetBool;

/**
 * @short Base widget for pages of the @ref OptionsDialog
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class OptionsWidget
{
public:
	OptionsWidget();
	virtual ~OptionsWidget();

	void addConfigOptionWidget(ConfigOptionWidget* w);

	virtual void loadFromConfigScript(const BosonConfigScript* script);
	virtual void load(); // load from current game values
	virtual void apply();
	virtual void setDefaults(); // reset to hardcoded defaults

private:
	Q3ValueList<ConfigOptionWidget*> mConfigOptionWidgets;
};

class GeneralOptions : public Q3VBox, public OptionsWidget
{
	Q_OBJECT
public:
	GeneralOptions(QWidget* parent);
	~GeneralOptions();

private:
	ConfigOptionWidgetInt* mGameSpeed;
	ConfigOptionWidgetDouble* mMiniMapScale;
	ConfigOptionWidgetBool* mRMBMovesWithAttack;
};

class CursorOptions : public Q3VBox, public OptionsWidget
{
	Q_OBJECT
public:
	CursorOptions(QWidget* parent);
	~CursorOptions();

	virtual void apply();
	virtual void setDefaults();
	virtual void load();

	void setCursor(CursorMode mode);

signals:
	void signalCursorChanged(int index, const QString& cursorDir);

private:
	QComboBox* mCursor;
	QComboBox* mCursorTheme;
	QStringList mCursorThemes;
};

class ScrollingOptions : public Q3VBox, public OptionsWidget
{
	Q_OBJECT
public:
	ScrollingOptions(QWidget* parent);
	~ScrollingOptions();

	virtual void apply();
	virtual void setDefaults();
	virtual void load();

	void setMouseWheelAction(CameraAction action);
	void setMouseWheelShiftAction(CameraAction action);

private:
	ConfigOptionWidgetBool* mRMBScrolling;
	ConfigOptionWidgetBool* mMMBScrolling;
	ConfigOptionWidgetBool* mWheelMoveZoom;
	ConfigOptionWidgetUInt* mArrowSpeed;
	ConfigOptionWidgetUInt* mCursorEdgeSensity;
	QComboBox* mMouseWheelAction;
	QComboBox* mMouseWheelShiftAction;
};

class SoundOptions : public Q3VBox, public OptionsWidget
{
	Q_OBJECT
public:
	SoundOptions(QWidget* parent);
	~SoundOptions();

	virtual void apply();
	virtual void setDefaults();
	virtual void load();

	void setUnitSoundsDeactivated(BosonConfig* conf);

private:
	QMap<QCheckBox*, UnitSoundEvent> mCheckBox2UnitSoundEvent;
	QCheckBox* weaponsounds;
};


class ChatOptions : public Q3VBox, public OptionsWidget
{
	Q_OBJECT
public:
	ChatOptions(QWidget* parent);
	~ChatOptions();

private:
	ConfigOptionWidgetUInt* mScreenRemoveTime;
	ConfigOptionWidgetInt* mScreenMaxItems;
};

class ToolTipOptions : public Q3VBox, public OptionsWidget
{
	Q_OBJECT
public:
	ToolTipOptions(QWidget* parent);
	~ToolTipOptions();

	virtual void apply();
	virtual void setDefaults();
	virtual void load();

private:
	ConfigOptionWidgetInt* mUpdatePeriod;
	QComboBox* mToolTipCreator;
};

#endif
