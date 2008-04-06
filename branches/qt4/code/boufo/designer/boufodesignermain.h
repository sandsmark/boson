/*
    This file is part of the Boson game
    Copyright (C) 2004-2006 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOUFODESIGNERMAIN_H
#define BOUFODESIGNERMAIN_H

#include "bodebugdcopiface.h"

#include <q3mainwindow.h>
#include <qdom.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <QLabel>
#include <QCloseEvent>

class Q3ListBox;
class Q3ListBoxItem;
class Q3ListView;
class Q3ListViewItem;
class QLabel;
class QPushButton;
class QComboBox;
class QLineEdit;
class Q3WidgetStack;
class Q3VBoxLayout;
class BoUfoWidget;
class OptionsDialog;
class FormPreview;
class BoWidgetList;
class BoWidgetTree;
class BoPropertiesWidget;

class BoUfoDesignerMainPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoDesignerMain : public Q3MainWindow
{
	Q_OBJECT
public:
	BoUfoDesignerMain();
	~BoUfoDesignerMain();

public slots:
	bool slotCreateNew();
	bool slotLoadFromFile(const QString& fileName);
	bool slotLoadFromXML(const QByteArray& xml, bool resetFileName = true);
	bool slotSaveAsFile(const QString& fileName);

	void slotLoad();
	void slotSave();
	void slotSaveAs();
	void slotEditSignalsSlots();

protected:
	void initProperties(QDomElement& widget, const QString& className);
	void provideProperty(QDomElement& element, const QString& property);

	void initActions();

	virtual void closeEvent(QCloseEvent*);

protected slots:
	void slotUpdateGUI();
	void slotDebugUfo();
	void slotConfigure();
	void slotApplyOptions();

private slots:
	void slotWidgetClassSelected(const QString&);
	void slotPlaceWidget(const QDomElement& parent);
	void slotRemoveWidget(const QDomElement& parent);
	void slotSelectWidget(const QDomElement& parent);
	void slotPropertiesChanged(const QDomElement& parent);
	void slotTreeHierarchyChanged();

private:
	BoUfoDesignerMainPrivate* d;
	FormPreview* mPreview;
	BoDebugDCOPIface* mIface;
	BoWidgetList* mWidgets;
	BoWidgetTree* mWidgetTree;
	BoPropertiesWidget* mProperties;
	OptionsDialog* mOptionsDialog;

	QDomDocument mDocument;
	QString mPlaceWidgetClass;
};

#endif

