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
#ifndef OPTIONSWIDGETS_H
#define OPTIONSWIDGETS_H

#include <qvbox.h>
#include <qstringlist.h>
#include <qmap.h>

#include "global.h"

class Player;
class Boson;
class BosonConfig;
class KIntNumInput;
class KDoubleNumInput;
class QComboBox;
class QCheckBox;

/**
 * @short Base widget for pages of the @ref OptionsDialog
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class OptionsWidget
{
public:
	OptionsWidget();
	virtual ~OptionsWidget();

	virtual void load() = 0; // load from current game values
	virtual void apply() = 0;
	virtual void setDefaults() = 0; // reset to hardcoded defaults

	Player* localPlayer() const { return mPlayer; }
	Boson* game() const { return mGame; }

	void setLocalPlayer(Player* p) { mPlayer = p; }
	void setGame(Boson* g) { mGame = g; }

private:
	Boson* mGame;
	Player* mPlayer;
};

class GeneralOptions : public QVBox, public OptionsWidget
{
	Q_OBJECT
public:
	GeneralOptions(QWidget* parent);
	~GeneralOptions();

	virtual void apply();
	virtual void setDefaults();
	virtual void load();

	/**
	 * Set the shown value for the game speed. Note that this value is the
	 * time between 2 advance calls in ms while the dialog
	 * does not show anything in ms. The dialog values are just the
	 * opposite: higher values mean higher speed.
	 **/
	void setGameSpeed(int ms);
	void setMiniMapScale(double s);
	void setCmdBackground(const QString& file);

signals:
	void signalMiniMapScaleChanged(double);
	void signalCmdBackgroundChanged(const QString& file);

private:
	KIntNumInput* mGameSpeed;
	KDoubleNumInput* mMiniMapScale;
	QComboBox* mCmdBackground;
	QStringList mCmdBackgrounds;
};

class CursorOptions : public QVBox, public OptionsWidget
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

class ScrollingOptions : public QVBox, public OptionsWidget
{
	Q_OBJECT
public:
	ScrollingOptions(QWidget* parent);
	~ScrollingOptions();

	virtual void apply();
	virtual void setDefaults();
	virtual void load();

	void setArrowScrollSpeed(int);
	void setCursorEdgeSensity(int);
	void setRMBScrolling(bool);
	void setMMBScrolling(bool);

private:
	KIntNumInput* mArrowSpeed;
	QCheckBox* mRMBScrolling;
	QCheckBox* mMMBScrolling;
	KIntNumInput* mCursorEdgeSensity;
};

class SoundOptions : public QVBox, public OptionsWidget
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
};


class OpenGLOptions : public QVBox, public OptionsWidget
{
	Q_OBJECT
public:
	OpenGLOptions(QWidget* parent);
	~OpenGLOptions();

	virtual void apply();
	virtual void setDefaults();
	virtual void load();

	void setUpdateInterval(int ms);

signals:
	void signalUpdateIntervalChanged(unsigned int);

protected:
	/**
	 * @return GL_NEAREST or GL_LINEAR for all textures.
	 **/
	int magnificationFilter() const;
	void setMagnificationFilter(int f);

	/**
	 * @return GL_NEAREST or GL_LINEAR for normal textures.
	 **/
	int minificationFilter() const;
	void setMinificationFilter(int f);

	/**
	 * @return GL_NEAREST, GL_LINEAR, GL_*_MIPMAP_* for mipmapped textures
	 **/
	int mipmapMinificationFilter() const;
	void setMipmapMinificationFilter(int f);

private:
	KIntNumInput* mUpdateInterval;
	QCheckBox* mModelTexturesMipmaps;
	QComboBox* mMagnificationFilter;
	QComboBox* mMinificationFilter;
	QComboBox* mMipmapMinificationFilter;
};

class ChatOptions : public QVBox, public OptionsWidget
{
	Q_OBJECT
public:
	ChatOptions(QWidget* parent);
	~ChatOptions();

	virtual void apply();
	virtual void setDefaults();
	virtual void load();

	void setScreenRemoveTime(unsigned int s);
	void setScreenMaxItems(int s);

private:
	KIntNumInput* mScreenRemoveTime;
	KIntNumInput* mScreenMaxItems;
};

#endif
