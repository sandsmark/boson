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

#ifndef BOSONSTARTEDITORWIDGET_H
#define BOSONSTARTEDITORWIDGET_H

#include <qwidget.h>
#include <qptrdict.h>
#include <qcolor.h>
#include <qvaluelist.h>

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class KColorCombo;
class KGameChat;
class QComboBox;
class QFrame;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListBox;
class QListBoxItem;
class QPushButton;
class QColor;

class Boson;
class Player;
class KPlayer;
class KGamePropertyBase;
class TopWidget;
class BosonPlayField;

class BosonStartEditorWidget : public QWidget
{ 
  Q_OBJECT
  public:
    BosonStartEditorWidget(TopWidget* top, QWidget* parent);
    ~BosonStartEditorWidget();

  public slots:
    void slotStart();
    /** 
     * Called when user clicks on "Cancel" button
     * Cancels game starting and closes (hides) this widget 
     **/
    void slotCancel();

  signals:
    void signalStartGame();
    void signalCancelled();

  protected:
    void sendNewGame();

  private:
    void initKGame();
    void initPlayer();
    Player* player() const;
    BosonPlayField* playField() const;

  private:
    QVBoxLayout* mBosonStartEditorWidgetLayout;
    QVBoxLayout* mMainLayout;
    QHBoxLayout* mStartGameLayout;

    QPushButton* mCancelButton;
    QPushButton* mStartGameButton;

    TopWidget* mTop;
    QColor mPlayercolor;
    QValueList<QColor> mAvailableColors;
    int mMap;
    int mMinPlayers;
    int mMaxPlayers;
    
};

#endif // BOSONNEWGAMEWIDGET_H
