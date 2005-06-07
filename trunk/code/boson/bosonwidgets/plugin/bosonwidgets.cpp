/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "dummyclasses.h"
#include "../bosonplayfieldview.h"
#include "../bonuminput.h"
#include "../bocolorchooser.h"

#include <kdeversion.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <qwidgetplugin.h>

#include <qlayout.h>
#include <qlabel.h>

// AB: most code is from kdelibs/kdewidgets/kdewidgets.cpp

class BosonWidgetsPlugin : public QWidgetPlugin
{
public:
  BosonWidgetsPlugin();
  virtual ~BosonWidgetsPlugin();

  virtual QStringList keys() const
  {
    QStringList list;
    QMap<QString, WidgetInfo>::ConstIterator it;
    for (it = mWidgets.begin(); it != mWidgets.end(); ++it) {
      list << it.key();
    }
    return list;
  }
  virtual QWidget* create(const QString& key, QWidget* parent = 0, const char* name = 0);
  virtual QIconSet iconSet(const QString& key) const
  {
    QString path = locate("data", "bosonwidgets/pics/" + mWidgets[key].iconSet);
    return QIconSet(path);
  }
  virtual bool isContainer(const QString& key) const
  {
    return mWidgets[key].isContainer;
  }
  virtual QString group(const QString& key) const
  {
    return mWidgets[key].group;
  }
  virtual QString includeFile(const QString& key) const
  {
    return mWidgets[key].includeFile;
  }
  virtual QString tooltip(const QString& key) const
  {
    return mWidgets[key].toolTip;
  }
  virtual QString whatsThis(const QString& key) const
  {
    return mWidgets[key].whatsThis;
  }

protected:
  void addBosonWidget(const QString& className, const QString& includeFile, const QString& toolTip, const QString& whatsThis, const QString& iconSet = QString::null);

private:
  struct WidgetInfo
  {
    QString group;
    QString iconSet;
    QString includeFile;
    QString toolTip;
    QString whatsThis;
    bool isContainer;
  };
  QMap<QString, WidgetInfo> mWidgets;
};


BosonWidgetsPlugin::BosonWidgetsPlugin()
{
 addBosonWidget("BosonMiniMap", "bosonminimap.h", "Mini map widget", QString::null, QString::null);
 addBosonWidget("BosonPlayFieldView", "bosonwidgets/bosonplayfieldview.h", "PlayField view", QString::null, QString::null);
 addBosonWidget("BoIntNumInput", "bosonwidgets/bonuminput.h", "Boson's int num input", QString::null, QString::null);
 addBosonWidget("BoColorChooser", "bosonwidgets/bocolorchooser.h", "Boson's color chooser", QString::null, QString::null);

//kdDebug() << k_funcinfo << endl;
 new KInstance("bosonwidgets");
}

BosonWidgetsPlugin::~BosonWidgetsPlugin()
{
}

void BosonWidgetsPlugin::addBosonWidget(const QString& className, const QString& includeFile, const QString& toolTip, const QString& whatsThis, const QString& iconSet)
{
 WidgetInfo widget;
 widget.group = "Boson";

 widget.isContainer = false; // no boson widget is a container atm. if we ever make one, we need to add it directly in the c'tor.
 widget.iconSet = iconSet;
 widget.includeFile = includeFile;
 widget.toolTip = toolTip;
 widget.whatsThis = whatsThis;

 mWidgets.insert(className, widget);
}

QWidget* BosonWidgetsPlugin::create(const QString& key, QWidget* parent, const char* name)
{
 if (key == "BosonMiniMap") {
  // we cannot create an actual minimap widget, as it depends on Unit,
  // Player, ...
  // uic should generate code that does use BosonMiniMap.
  return new BosonMiniMap(parent, name);
 } else if (key == "BosonPlayFieldView") {
  return new BosonPlayFieldView(parent, name);
 } else if (key == "BoIntNumInput") {
  return new BoIntNumInput(parent, name);
 } else if (key == "BoColorChooser") {
  return new BoColorChooser(parent, name);
 }
 return 0;
}

Q_EXPORT_PLUGIN(BosonWidgetsPlugin)

