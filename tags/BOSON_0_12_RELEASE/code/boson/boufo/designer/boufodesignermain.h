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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef BOUFODESIGNERMAIN_H
#define BOUFODESIGNERMAIN_H

#include "bodebugdcopiface.h"

#include <qmainwindow.h>
#include <qdom.h>

class QListBox;
class QListBoxItem;
class QListView;
class QListViewItem;
class QLabel;
class QPushButton;
class QComboBox;
class QLineEdit;
class QWidgetStack;
class QVBoxLayout;
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
class BoUfoDesignerMain : public QMainWindow
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

