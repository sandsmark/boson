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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef BOMODELPIXMAPS_H
#define BOMODELPIXMAPS_H

#include "bodebugdcopiface.h"
#include "bo3dtools.h"
#include "bosonglwidget.h"

#include <kmainwindow.h>

#include <qvaluevector.h>
#include <qvaluelist.h>
#include <qpixmap.h>

class BosonModel;
class BoPixmapRenderer;
class BoCamera;
class KCmdLineArgs;
class QPushButton;
class QLabel;

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
class BoModelPixmapsGLWidget : public BosonGLWidget
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

protected:
	virtual void initializeGL();
	virtual void paintGL();

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

private:
	BoModelPixmapsPrivate* d;
	BoDebugDCOPIface* mIface;
	BoModelPixmapsGLWidget* mGLWidget;
	BoPixmapRenderer* mPixmapRenderer;

	QString mModelFileName;
	QPushButton* mModelFile;
	QPtrList<BoModelPixmapCollection> mModelPixmaps;
	QValueVector<QLabel*> mModelPixmapLabels;
	QPtrList<BoTextureCopyright> mTextureCopyright;
};



#endif

