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

#ifndef BOSONNETWORKOPTIONSWIDGET_H
#define BOSONNETWORKOPTIONSWIDGET_H

#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class KIntNumInput;
class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpacerItem;

class TopWidget;
class Boson;

class BosonNetworkOptionsWidget : public QWidget
{ 
  Q_OBJECT
  public:
    BosonNetworkOptionsWidget(TopWidget* top, QWidget* parent);
    ~BosonNetworkOptionsWidget();

    void setLogoSpacer(int height);

  public slots:
    void slotDisconnect();
    void slotStartNetwork();
    void slotConnectionTypeChanged(int);
    void slotConnectionBroken();
    
  signals:
    void signalOkClicked();

  private:
    QLabel* mHeader;
    QLabel* mNetStatusText;
    QLabel* mNetStatusLabel;
    QPushButton* mDisconnectButton;
    QGroupBox* mNetConfGroupBox;
    QButtonGroup* mConnectionStyleGroup;
    QRadioButton* mCreateGameButton;
    QRadioButton* mJoinGameButton;
    QLabel* mPortLabel;
    QLineEdit* mHostEdit;
    KIntNumInput* mPortEdit;
    QLabel* mHostLabel;
    QPushButton* mStartNetworkButton;
    QPushButton* mCancelButton;
    QPushButton* mOkButton;

    QVBoxLayout* mBosonNetworkOptionsWidgetLayout;
    QHBoxLayout* mLayout10;
    QVBoxLayout* mLayout8;
    QHBoxLayout* mLayout1;
    QHBoxLayout* mLayout2;
    QVBoxLayout* mNetConfGroupBoxLayout;
    QVBoxLayout* mLayout7;
    QVBoxLayout* mConnectionStyleGroupLayout;
    QGridLayout* mLayout6;
    QHBoxLayout* mLayout13;
    QSpacerItem* mLogoSpacer;

  private:
    void setConnected(bool connected, bool master);
    Boson* game();

    TopWidget* mTop;
};

#endif // BOSONNETWORKOPTIONSWIDGET_H
