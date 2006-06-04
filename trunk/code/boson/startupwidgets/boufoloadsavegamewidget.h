/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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

#ifndef BOUFOLOADSAVEGAMEWIDGET_H
#define BOUFOLOADSAVEGAMEWIDGET_H

#include <boufo/boufo.h>

class QDateTime;
class BoUfoSaveGameWidget;

class BoUfoLoadSaveGameWidgetPrivate;
/**
 * This class provides a widget for loading/saving games. It was created because
 * loading a game using e.g. @ref KFileDialog where you have to click on a
 * specific file just sucks. You rarely want this in modern games.
 *
 * In this class you have a collection of buttons, every button represents a
 * savegame. When the user clicks on a button it gets highlighted. In load mode
 * the user can now click on "load game" which causes this class to emit @ref
 * signalLoadGame and the game to start loading the game. In save mode the user
 * can enter a name for the game (levelname and timestamp are added by the game
 * or by this class automatically) and then click on "save game" to save the
 * game (i.e. emit @ref signalSaveGame)
 *
 * TODO: will the games simply be overwritten or are new files added?
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoLoadSaveGameWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	/**
	 * @param directory See @ref setDirectory
	 * @param suffix See @ref setSuffix
	 * @param save Initial mode - see @ref setSaveMode
	 **/
	BoUfoLoadSaveGameWidget(bool save = true, const QString& suffix = QString("savegame"), const QString& directory = QString::null);

	virtual ~BoUfoLoadSaveGameWidget();

	static QString defaultDir();

	/**
	 * Search in @p dir for games. All files matching suffix @ref
	 * setSuffix will be displayed in this widget. Note that the list of
	 * available buttons/games is <em>not</em> regenerated when you call 
	 * this! You need to call @ref updateGames to make the changes take
	 * effect!
	 *
	 * By default we use $KDEHOME/share/apps/app_name/savegames/ (see @ref
	 * KStandardDirs::saveLocation())
	 *
	 * Note that providing an empty dir reverts to the default dir.
	 **/
	void setDirectory(const QString& dir);

	/**
	 * Display only the files in the directory set by @ref setDirectory
	 * that have this @p suffix.
	 *
	 * The @p suffix shouldn't contain a leading ".", as it is added automatically.
	 * Note that you also have to call @ref updateGames after changing the
	 * suffix.
	 *
	 * The default is no suffix at all.
	 **/
	void setSuffix(const QString& suffix);

	/**
	 * @return The value set by @ref setSuffix, i.e. the file suffix without
	 * leading dot. This would be e.g. "bsg" for boson.
	 **/
	const QString& suffix() const;

	/**
	 * Create the buttons and therefore display the available games 
	 * according to the values provided by @ref setDirectory and @ref
	 * setSuffix
	 **/
	void updateGames();

	BoUfoSaveGameWidget* selectedGame() const;

	/**
	 * Switch the load/save mode. If @p save is TRUE the widget is in save
	 * mode and will emit @ref signalSaveGame. Otherwise It will allow
	 * loading a game. By default the widget is in save mode.
	 **/
	void setSaveMode(bool save);

	/**
	 * @return The current directory (i.e. where the games are stored). See
	 * @ref QDir::path
	 **/
	QString directory() const;

	/**
	 * @return All files that meet the specified conditions from @ref
	 * setDirectory and @ref setSuffix. See also @ref QDir::entryList
	 **/
	QStringList entryList(); // cant be const cause of a qt workaround

signals:
	void signalLoadGame(const QString& file);

	/**
	 * Save the game under @p fileName. Note that currently this class does
	 * not warn the user when the filename is already used - that means you
	 * should display a messagebox asking the player whether he wants to
	 * overwrite the file or not.
	 *
	 * Also note that in most cases the player will oribably select an already
	 * existing file, but not always. @p fileName will be @ref saveFileName
	 * if the user selected a new file.
	 *
	 * @param description A description (entered by the player) for this
	 * savegame. You should store it in your file format and display it here
	 * again, when you display the loadgame widget.
	 **/
	void signalSaveGame(const QString& fileName, const QString& description);

	/**
	 * Cancel the dialog. You should close the widget and return to the game
	 * when this signal is emitted.
	 **/
	void signalCancel();

protected:
	/**
	 * Prepare a file to be added to the widget. This gets called for every
	 * file that meets the conditions of (currently) @ref setDirectory and
	 * @ref setSuffix whenever @ref updateGames gets called.
	 *
	 * You may want to pre-read the file here and e.g. provide correct
	 * date, time, description and level values. By default this function
	 * calls @ref addFile with the @ref QFileInfo::lastModified of the file
	 * (very bad, btw! will change e.g. whenever it is copied around), an
	 * empty level name and as description the filename is used.
	 *
	 * Note that you also could subclass @ref BoUfoSaveGameWidget and
	 * reimplement @ref createButton. After calling @ref addFile here you
	 * might provide additional data for your custom button.
	 *
	 * @param position The index of the position of the button that should
	 * get added. Usually you'll just forward this to @ref addFile
	 **/
	virtual void readFile(const QString& file, int position);

	/**
	 * Called internally when a new button has to be created. You can
	 * reimplement this if you want to subclass @ref BoUfoSaveGameWidget.
	 * @return A @ref BoUfoSaveGameWidget button that represents a savegame.
	 **/
	virtual BoUfoSaveGameWidget* createButton();

	/**
	 * Create a @ref BoUfoSaveGameWidget button for the specified file and
	 * use the specified data.
	 * @param file Absolute filename. Will be used for @ref signalLoadGame
	 * and/or @ref signalSaveGame
	 * @param dateTime Which date and time values will be shown for this
	 * file
	 * @param level The name of the level or map or whatever is useful to
	 * your game
	 * @param description This is the custom description that should be
	 * editable by the user when saving a game.
	 * @param position Where to add this button. Usuall simply the same as
	 * the position from @ref readFile
	 * @return A pointer to the widget that has been updated. Note that if
	 * @p position is invalid a new button will be created using @ref
	 * createButton.
	 **/
	BoUfoSaveGameWidget* addFile(const QString& file, const QString& description, const QString& level, const QDateTime& dateTime, int position);

	virtual QString saveFileName();//cant be const - see entryList()

protected slots:
	void slotDelete();
	void slotLoadSave();
	void slotLoadSaveTimer();
	void slotBrowse();
	void slotClicked(BoUfoSaveGameWidget*);
	void slotCancel();

private:
	void init();
	void setDefaultDir();
	void setDefaultSuffix();

private:
	BoUfoLoadSaveGameWidgetPrivate* d;
};

// a button for the widget above. displays a lot of information about the saved
// game, e.g. level (map for boson), description (how the user will name the
// game), timestamp
class BoUfoSaveGameWidgetPrivate;
class BoUfoSaveGameWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoSaveGameWidget();
	~BoUfoSaveGameWidget();

	void setDateTime(const QDateTime&);
	void setLevel(const QString& level);
	void setDescription(const QString& description);
	void setFile(const QString& file);

	const QString& file() const;
	QString description() const;

	bool isSelected() const;
	void unselect();

signals:
	void signalClicked(BoUfoSaveGameWidget*);

private slots:
	void slotClicked() { emit signalClicked(this); }

private:
	BoUfoSaveGameWidgetPrivate* d;
};

#endif
