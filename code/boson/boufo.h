/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// note the copyright above: this is LGPL!
#ifndef BOUFO_H
#define BOUFO_H

#include <qobject.h>

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;

namespace ufo {
	class UXToolkit;
	class UXDisplay;
	class UXContext;
	class URootPane;
	class UObject;
	class ULayoutManager;
	class UWidget;
	class UButton;
	class UCheckBox;
	class UTextEdit;
	class USlider;
};

class BoUfoManager
{
public:
	BoUfoManager(int w, int h);
	~BoUfoManager();

	// AB: note that atm we always use video device size == context size
	void resize(int w, int h);

	ufo::UXToolkit* toolkit() const
	{
		return mToolkit;
	}

	ufo::UXDisplay* display() const
	{
		return mDisplay;
	}

	ufo::UXContext* context() const
	{
		return mContext;
	}
	ufo::URootPane* rootPane() const
	{
		return mRootPane;
	}
	ufo::UWidget* contentWidget() const
	{
		return mContentWidget;
	}

	void render();

	void postResizeEvent(int w, int h);

	// TODO: find out whether the event is taken by UFO and return true if
	// that is the case (other GUI elements can ignore it)
	void postMousePressEvent(QMouseEvent* e);
	void postMouseReleaseEvent(QMouseEvent* e);
	void postMouseMoveEvent(QMouseEvent* e);
	void postWheelEvent(QWheelEvent* e);
	void postKeyPressEvent(QKeyEvent* e);
	void postKeyReleaseEvent(QKeyEvent* e);

private:
	void initUfoWidgets();

private:
	ufo::UXToolkit* mToolkit;
	ufo::UXDisplay* mDisplay;
	ufo::UXContext* mContext;

	ufo::URootPane* mRootPane;
	ufo::UWidget* mContentWidget;
};

class BoUfoWidget : public QObject
{
	Q_OBJECT
public:
	BoUfoWidget(QObject*, const char* name = 0);
	~BoUfoWidget();

	ufo::UWidget* contentWidget() const
	{
		return mWidget;
	}

	/**
	 * Default: TRUE (i.e. background is visible)
	 **/
	void setOpaque(bool o);

	/**
	 * Equivalent to contentWidget()->setLayout(layout).
	 *
	 * WARNING this has weird results when called for derived widgets!
	 **/
	void setLayout(ufo::ULayoutManager* layout);

	/**
	 * Add the @ref contentWidget of @p w to this @ref contentWidget.
	 *
	 * WARNING: you are responsible for correct deletion of the BoUfoWidget
	 * objects. If @p w is a normal child of this widget (i.e. you used
	 * "this" in the c'tor of @p w), then you don't need to do anything.
	 * Otherwise you must make sure that @p w is deleted before this object.
	 **/
	void addWidget(BoUfoWidget* w);

	void addSpacing(int);


protected:
	virtual void setChildrenOpaque(bool) {}

private:
	ufo::UWidget* mWidget;
};

class BoUfoHBox : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoHBox(QObject* parent, const char* name = 0);
};

class BoUfoVBox : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoVBox(QObject* parent, const char* name = 0);
};

class BoUfoPushButton : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoPushButton(QObject* parent, const char* name = 0);
	BoUfoPushButton(const QString& text, QObject* parent, const char* name = 0);
	~BoUfoPushButton();

signals:
	void signalClicked();

private:
	void init(const QString& text);

private:
	ufo::UButton* mButton;
};

class BoUfoCheckBox : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoCheckBox(QObject* parent, const char* name = 0);
	BoUfoCheckBox(const QString& text, QObject* parent, const char* name = 0);
	BoUfoCheckBox(const QString& text, bool checked, QObject* parent, const char* name = 0);
	~BoUfoCheckBox();

	void setChecked(bool);

signals:
	void signalToggled(bool);

private:
	void init(const QString& text);

private:
	ufo::UCheckBox* mCheckBox;
};

class BoUfoNumInput: public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoNumInput(QObject* parent, const char* name = 0);
	~BoUfoNumInput();


	void setLabel(const QString&);
	void setStepSize(float);
	void setRange(float min, float max);


public slots:
	void setValue(float);
	void slotSetMaxValue(float);

signals:
	void signalValueChanged(float);

private:
	void init();

private:
	ufo::UTextEdit* mTextEdit;
	ufo::USlider* mSlider;
};


#endif
