/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOPUI_H
#define BOPUI_H

#include <qobject.h>

class puButton;
class puGroup;
class puObject;

class BoPUIWidget;
class BoPUILabel;

class BoPUILayoutPrivate;
class BoPUILayout : public QObject
{
	Q_OBJECT
public:
	BoPUILayout(bool vertical, QObject* parent, const char* name = 0);
	~BoPUILayout();

	void setBasePos(int x, int y, bool redoLayout = true)
	{
		mBaseX = x;
		mBaseY = y;
		if (redoLayout) {
			doLayout();
		}
	}
	int width() const;
	int height() const;

	void addObject(puObject*);
	void addWidget(BoPUIWidget*);
	void addLayout(BoPUILayout*);
	void addSpacing(int);

	void removeWidget(BoPUIWidget*);
	void removeLayout(BoPUILayout*);

	void doLayout();

	void hide();
	void show();
	void setWidgetsEnabled(bool e);

	bool hasTopParentLayout() const
	{
		if (!mTopParentLayout) {
			return false;
		}
		return true;
	}
	void setTopParentLayout(BoPUILayout* l);

	/**
	 * Called by @ref addLayout only.
	 **/
	void setParentLayout(BoPUILayout* l)
	{
		mParentLayout = l;
	}
	bool hasParentLayout() const
	{
		if (!mParentLayout) {
			return false;
		}
		return true;
	}

	/**
	 * Disable layout updates if @p disable is TRUE, otherwise enables it
	 * again.
	 *
	 * Note that this modifies the top layout, you should _not_ use this
	 * unless your layout structure is complete.
	 **/
	void disableLayout(bool disable);

protected:
	void doLayout(int availableWidth, int availableHeight);

protected:
	BoPUILayout* mTopParentLayout;
	BoPUILayout* mParentLayout;

private:
	void init();

private:
	BoPUILayoutPrivate* d;
	bool mVertical;
	int mBaseX;
	int mBaseY;
};

class BoPUIVLayout : public BoPUILayout
{
	Q_OBJECT
public:
	BoPUIVLayout(QObject* parent, const char* name = 0) : BoPUILayout(true, parent, name)
	{
	}
};

class BoPUIHLayout : public BoPUILayout
{
	Q_OBJECT
public:
	BoPUIHLayout(QObject* parent, const char* name = 0) : BoPUILayout(false, parent, name)
	{
	}
};

class BoPUIWidget : public QObject
{
	Q_OBJECT
public:
	/**
	 * This constructor creates a user layout that can be used to add
	 * widgets to it. See @ref layout
	 * @param vLayout Provide a @ref BoPUIVLayout if TRUE, otherwise a @ref
	 * BoPUIHLayout
	 **/
	BoPUIWidget(bool vLayout, QObject* parent, const char* name = 0);
	BoPUIWidget(QObject* parent, const char* name = 0);
	~BoPUIWidget();

	void setPos(int x, int y);
	void setSize(int w, int h);

	void setLabel(const QString& text, int a = AlignLeft);
	void setLegend(const QString& text);

	/**
	 * See @ref puObject::greyOut and @ref puObject::activate
	 **/
	void setEnabled(bool e);
	bool isEnabled() const
	{
		return mIsEnabled;
	}

	int x() const { return mX; }
	int y() const { return mY; }
	int width() const;
	int height() const;

	/**
	 * @return The user layout that can be used to add widgets to it. Note
	 * that there is at least one additional internal layout as parent of
	 * this layout (containing e.g. the label). NULL if you did not request
	 * a layout in the constructor.
	 **/
	BoPUILayout* layout() const
	{
		return mUserLayout;
	}

	void hide();
	void show();

	/**
	 * @internal
	 **/
	bool hasTopParentLayout() const
	{
		if (!mTopParentLayout) {
			return false;
		}
		return true;
	}

	/**
	 * @internal
	 * Do not use directly!
	 **/
	void setTopParentLayout(BoPUILayout* l);

	/**
	 * @internal
	 * Called by @ref BoPUILayout::addWidget only.
	 **/
	void setParentLayout(BoPUILayout* l)
	{
		mParentLayout = l;
	}
	bool hasParentLayout() const
	{
		if (!mParentLayout) {
			return false;
		}
		return true;
	}

	/**
	 * @internal
	 * Do not use directly!
	 **/
	bool doParentLayout();
	/**
	 * @internal
	 * Do not use directly!
	 **/
	void doLayout(int, int);
	friend class BoPUILayout;

signals:
	void signalActivated();

private:
	static void myCallback(puObject*);

private:
	void init();

protected:
	void installCallback(puObject*);

	/**
	 * Called by the callback function.
	 **/
	virtual void emitSignal();

	void setLabelText(const QString& text);

protected:
	puObject* mWidget;
	BoPUILayout* mTopParentLayout;
	BoPUILayout* mParentLayout;
	BoPUILayout* mLayout;
	BoPUILayout* mUserLayout;
	char* mLabel;

private:
	int mX;
	int mY;
	bool mHidden;
	bool mIsEnabled;

	char* mLegend;
	BoPUILabel* mLabelWidget;
};

class BoPUILabel : public BoPUIWidget
{
	Q_OBJECT
public:
	BoPUILabel(QObject* parent, const char* name = 0);
	BoPUILabel(const QString& text, QObject* parent, const char* name = 0);

	void setText(const QString& text);

private:
	void init();
};

class BoPUIPushButton : public BoPUIWidget
{
	Q_OBJECT
public:
	BoPUIPushButton(const QString& text, QObject* parent, const char* name = 0);
	BoPUIPushButton(QObject* parent, const char* name = 0);

	/**
	 * Set the text of the button. Convenience method to the @ref setLegend
	 * method. "legend" is the plib notation.
	 **/
	void setText(const QString& text)
	{
		setLegend(text);
	}

signals:
	void signalClicked();

protected:
	virtual void emitSignal();

private:
	void init();
};

class BoPUICheckBox : public BoPUIWidget
{
	Q_OBJECT
public:
	BoPUICheckBox(const QString& text, QObject* parent, const char* name = 0);
	BoPUICheckBox(const QString& text, bool checked, QObject* parent, const char* name = 0);
	BoPUICheckBox(bool checked, QObject* parent, const char* name = 0);
	~BoPUICheckBox();

	bool isChecked() const;

public slots:
	void setChecked(bool);

signals:
	void signalToggled(bool checked);

protected:
	virtual void emitSignal();

private:
	void init();
};

class BoPUISlider : public BoPUIWidget
{
	Q_OBJECT
public:
	BoPUISlider(QObject* parent, const char* name = 0);

	float value() const;

	void setStepSize(float s);
	float minValue() const;
	float maxValue() const;

public slots:
	void setValue(float v);
	void setRange(float min, float max);

	void slotSetMinValue(float min)
	{
		setRange(min, maxValue());
	}
	void slotSetMaxValue(float max)
	{
		setRange(minValue(), max);
	}

signals:
	void signalValueChanged(float);

protected:
	virtual void emitSignal();

private:
	void init();

private:
};

class BoPUIInput : public BoPUIWidget
{
	Q_OBJECT
public:
	BoPUIInput(QObject* parent, const char* name = 0);

	QString text() const;

	void setText(const QString& text);

signals:
	void signalTextChanged(QString);

protected:
	virtual void emitSignal();

private:
	void init();
};

class BoPUINumInput : public BoPUIWidget
{
	Q_OBJECT
public:
	BoPUINumInput(QObject* parent, const char* name = 0);

	void setStepSize(float s);
	float minValue() const;
	float maxValue() const;
	float value() const;

public slots:
	void setValue(float v);
	void setRange(float min, float max);

	void slotSetMinValue(float min)
	{
		setRange(min, maxValue());
	}
	void slotSetMaxValue(float max)
	{
		setRange(minValue(), max);
	}

signals:
	void signalValueChanged(float);

protected:
	virtual void emitSignal();

protected slots:
	void slotSliderChanged(float);
	void slotInputChanged(QString);

private:
	void init();

private:
	BoPUISlider* mSlider;
	BoPUIInput* mInput;
};

class BoPUITabWidgetPrivate;
class BoPUITabWidget : public BoPUIWidget
{
	Q_OBJECT
public:
	BoPUITabWidget(QObject* parent, const char* name = 0);
	~BoPUITabWidget();

	void addTab(BoPUIWidget* widget, const QString& label);
	void removeTab(BoPUIWidget* widget);

	void setCurrentTab(int tab);
	BoPUIWidget* currentTab() const;

protected slots:
	void slotButtonClicked();

private:
	void init();

private:
	BoPUITabWidgetPrivate* d;
};

void puInitQt();

#ifdef puInit
#error Oops - puInit already defined
#else
#define puInit puInitQt
#endif

#endif

