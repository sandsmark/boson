/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOMODELPIXMAPS_H
#define BOMODELPIXMAPS_H

#include "bo3dtools.h"
#include <QGLWidget>

#include <kmainwindow.h>

#include <q3valuevector.h>
#include <q3valuelist.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <QLabel>
#include <Q3PtrList>

class BosonModel;
class BoPixmapRenderer;
class BoCamera;
class KCmdLineArgs;
class QPushButton;
class QLabel;
class Q3GridLayout;
class QLineEdit;

class BoTextureCopyright : public QWidget
{
	Q_OBJECT
public:
	BoTextureCopyright(QWidget* parent);
	~BoTextureCopyright();

	void setTexture(const QString& absoluteFileName);
	void setAuthor(const QString& name);
	void setLicense(const QString& name);

	const QString& texture() const
	{
		return mFile;
	}
	QString textureFile() const;
	QString author() const;
	QString license() const;

private:
	QString mFile;
	QLabel* mTexture;
	QLineEdit* mCopyrightOwner;
	QLineEdit* mLicense;
};

class BoModelPixmapCollection
{
public:
	BoModelPixmapCollection()
	{
	}
	~BoModelPixmapCollection()
	{
	}

	void setName(const QString& name)
	{
		mName = name;
	}
	const QString& name() const
	{
		return mName;
	}

	void setPixmap(const QPixmap& p)
	{
		mPixmap = p;
	}
	const QPixmap& pixmap() const
	{
		return mPixmap;
	}

	void setThumbnail(const QPixmap& p)
	{
		mThumbnail = p;
	}
	const QPixmap& thumbnail() const
	{
		return mThumbnail;
	}

private:
	QString mName;
	QPixmap mPixmap;
	QPixmap mThumbnail;
};

class BoModelPixmapsGLWidgetPrivate;
class BoModelPixmapsGLWidget : public QGLWidget
{
	Q_OBJECT
public:
	BoModelPixmapsGLWidget(QWidget* parent = 0);
	~BoModelPixmapsGLWidget();

	void initWidget();
	bool parseCmdLineArgs(KCmdLineArgs* args);
	bool loadCamera(KCmdLineArgs* args);

	bool loadModel(const QString& file);
	void resetModel();

	const BosonModel* model() const;
	BoCamera* camera() const;

	void updateBaseProjectionMatrix();
	const BoMatrix& baseProjectionMatrix() const;
	void setProjectionMatrix(const BoMatrix& matrix);

protected:
	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int width, int height);

private:
	BoModelPixmapsGLWidgetPrivate* d;
};

class BoModelPixmapsPrivate;
/**
 * This widget is here mainly for historic reasons. It just contains a @ref
 * Modelpreview widget. Most interesting things are done there.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoModelPixmaps : public KMainWindow
{
	Q_OBJECT
public:
	BoModelPixmaps();
	~BoModelPixmaps();

	bool parseCmdLineArgs(KCmdLineArgs*);

	void selectModelFile(const QString& file);
	void packageIt(const QString& file);

public slots:
	void slotPackageIt();

protected slots:
	void slotSelectModelFile();
	void slotSelectTextureDirectory();
	bool slotCheckPackage();

protected:
	void selectTextureDirectory(const QString& file);
	void retrievePixmaps();
	void reset();
	void addTextureCopyright(const QString&);
	void fitModelIntoView(const BoVector3Float& cameraPos, const BoVector3Float& lookAt,
			const BoVector3Float& up);

	void displayLabels(int count);

private:
	BoModelPixmapsPrivate* d;
	BoModelPixmapsGLWidget* mGLWidget;
	BoPixmapRenderer* mPixmapRenderer;

	QString mModelFileName;
	QPushButton* mModelFile;
	Q3PtrList<BoModelPixmapCollection> mModelPixmaps;
	Q3GridLayout* mModelPixmapLabelsLayout;
	Q3ValueVector<QLabel*> mModelPixmapLabels;
	Q3PtrList<BoTextureCopyright> mTextureCopyright;
};


#include "ui_bomodelpixmapsgui.h"
class BoModelPixmapsGUI : public QWidget, public Ui::BoModelPixmapsGUI
{
	Q_OBJECT
public:
	BoModelPixmapsGUI(QWidget* parent) : QWidget(parent)
	{
	}
};


#endif

