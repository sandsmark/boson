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

class TopWidget;
class Boson;

class BosonNetworkOptionsWidget : public QWidget
{ 
  Q_OBJECT
  public:
    BosonNetworkOptionsWidget(TopWidget* top, QWidget* parent);
    ~BosonNetworkOptionsWidget();

    QLabel* header;
    QLabel* netstatustext;
    QLabel* netstatuslabel;
    QPushButton* disconnectbutton;
    QGroupBox* netconfgroupbox;
    QButtonGroup* connectionstylegroup;
    QRadioButton* creategamebutton;
    QRadioButton* joingamebutton;
    QLabel* portlabel;
    QLineEdit* hostedit;
    KIntNumInput* portedit;
    QLabel* hostlabel;
    QPushButton* startnetworkbutton;
    QPushButton* cancelbutton;
    QPushButton* okbutton;

  public slots:
    void slotDisconnect();
    void slotStartNetwork();
    void slotConnectionTypeChanged(int);
    void slotConnectionBroken();
    
  signals:
    void signalOkClicked();

  protected:
    QVBoxLayout* BosonNetworkOptionsWidgetLayout;
    QHBoxLayout* Layout10;
    QVBoxLayout* Layout8;
    QHBoxLayout* Layout1;
    QHBoxLayout* Layout2;
    QVBoxLayout* netconfgroupboxLayout;
    QVBoxLayout* Layout7;
    QVBoxLayout* connectionstylegroupLayout;
    QGridLayout* Layout6;
    QHBoxLayout* Layout13;

  private:
    void setConnected(bool connected, bool master);
    Boson* game();

    TopWidget* mTop;
};

#endif // BOSONNETWORKOPTIONSWIDGET_H
