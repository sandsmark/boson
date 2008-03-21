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

#ifndef BOSONCURSOREDITOR_H
#define BOSONCURSOREDITOR_H

#include <qwidget.h>
#include <qstringlist.h>
#include <qvgroupbox.h>

class QLabel;
class QPushButton;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QVBox;
class KIntNumInput;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class SpriteConfig : public QVGroupBox
{
	Q_OBJECT
public:
	SpriteConfig(QWidget*);
	~SpriteConfig();

	void load(const QString& file);
	void save(const QString& file);

signals:
	void apply();

private:
	QLineEdit* mFilePrefix;
	KIntNumInput* mHotspotX;
	KIntNumInput* mHotspotY;
	QCheckBox* mIsAnimated;

	QVBox* mAnimationSettings;
	KIntNumInput* mFrameCount;
	KIntNumInput* mRotateDegree;
	KIntNumInput* mAnimationSpeed;
};

/**
 * Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonCursorEditor : public QWidget
{
	Q_OBJECT
public:
	BosonCursorEditor(QWidget* parent);
	~BosonCursorEditor();

	void setCursor(int);

	void loadInitialCursor();

signals:
	void signalCursorChanged(int index, const QString& cursorDir);
	void signalCursorTypeChanged(int mode);

protected:
	void changeBaseDirectory(const QString& dir);
	void loadSpriteConfig(const QString& theme);
	QStringList findCursorThemes(const QString& dir);

	void addType(const QString& theme, const QString& type);

protected slots:
	void slotChangeBaseDirectory();
	void slotCursorModeChanged(int);
	void slotCursorThemeChanged(int);
	void slotCursorTypeChanged(int);
	void slotApplySpriteConfig();

private:
	void init();

private:
	QPushButton* mBaseDirectory;
	QComboBox* mCursorMode;
	QComboBox* mCursorType;
	QStringList mCursorTypes;
	QComboBox* mCursorTheme;
	QStringList mCursorThemes;
	SpriteConfig* mSpriteConfig;
};

#endif

